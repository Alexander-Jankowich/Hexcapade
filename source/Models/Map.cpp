// this file generates and renders a full run layout made of rooms, halls,
// walls, and room-based spawn points

#include "Map.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>

using namespace cugl;
using namespace cugl::graphics;

Map::Map() :
_gridWidth(0),
_gridHeight(0),
_tileSize(0.0f),
_origin(Vec2::ZERO),
_worldBounds(Rect::ZERO),
_playerSpawn(Vec2::ZERO),
_config() {
    _rng.seed(std::random_device{}());
}

bool Map::generate(const Size& viewSize, const MapGenerationConfig& config, unsigned int seed) {
    _config = config;

    if (_config.roomPalette.empty()) {
        _config.roomPalette.push_back(Color4(118, 146, 108, 255));
    }

    if (seed == 0) {
        seed = std::random_device{}();
    }
    _rng.seed(seed);

    _tileSize = std::max(8.0f, _config.tileSize);
    float worldScaleX = std::max(1.0f, _config.worldScaleX);
    float worldScaleY = std::max(1.0f, _config.worldScaleY);
    _gridWidth = std::max(std::max(16, _config.worldMinGridWidth),
                          (int)std::ceil((viewSize.width / _tileSize) * worldScaleX));
    _gridHeight = std::max(std::max(16, _config.worldMinGridHeight),
                           (int)std::ceil((viewSize.height / _tileSize) * worldScaleY));
    _origin = Vec2::ZERO;
    _worldBounds = Rect(_origin.x, _origin.y, _gridWidth * _tileSize, _gridHeight * _tileSize);

    _tiles.assign((size_t)(_gridWidth * _gridHeight), TileType::EMPTY);
    _tileRoomIndex.assign((size_t)(_gridWidth * _gridHeight), -1);
    _rooms.clear();
    _itemSpawnPoints.clear();
    _enemySpawnPoints.clear();
    _playerSpawn = Vec2::ZERO;

    int clampedMinRooms = std::max(1, _config.roomCountMin);
    int clampedMaxRooms = std::max(clampedMinRooms, _config.roomCountMax);
    std::uniform_int_distribution<int> roomCountDist(clampedMinRooms, clampedMaxRooms);
    int targetRoomCount = roomCountDist(_rng);

    int widthMin = std::max(4, _config.roomWidthMin);
    int widthMax = std::max(widthMin, _config.roomWidthMax);
    int heightMin = std::max(4, _config.roomHeightMin);
    int heightMax = std::max(heightMin, _config.roomHeightMax);
    std::uniform_int_distribution<int> roomWidthDist(widthMin, widthMax);
    std::uniform_int_distribution<int> roomHeightDist(heightMin, heightMax);
    bool useTemplates = !_config.roomTemplates.empty();
    std::uniform_int_distribution<size_t> templateDist(0, useTemplates ? (_config.roomTemplates.size() - 1) : 0);

    std::vector<GridRoom> generatedRooms;
    generatedRooms.reserve((size_t)targetRoomCount);
    int maxHallwayLength = std::max(1, _config.maxHallwayLengthTiles);

    auto hallwayDistance = [](const GridRoom& a, const GridRoom& b) {
        int ax = a.x + a.w / 2;
        int ay = a.y + a.h / 2;
        int bx = b.x + b.w / 2;
        int by = b.y + b.h / 2;
        return std::abs(ax - bx) + std::abs(ay - by);
    };

    int edgePadding = std::max(1, _config.roomEdgePadding);
    int placementAttempts = std::max(1, _config.roomPlacementAttempts);
    for (int attempt = 0; attempt < placementAttempts && (int)generatedRooms.size() < targetRoomCount; ++attempt) {
        int rw = roomWidthDist(_rng);
        int rh = roomHeightDist(_rng);
        int templateIndex = -1;
        if (useTemplates) {
            templateIndex = (int)templateDist(_rng);
            const RoomTemplateConfig& templ = _config.roomTemplates[(size_t)templateIndex];
            rw = std::max(4, templ.width);
            rh = std::max(4, templ.height);
        }

        int maxX = _gridWidth - rw - edgePadding - 1;
        int maxY = _gridHeight - rh - edgePadding - 1;
        if (maxX < edgePadding || maxY < edgePadding) {
            continue;
        }

        std::uniform_int_distribution<int> xDist(edgePadding, maxX);
        std::uniform_int_distribution<int> yDist(edgePadding, maxY);
        GridRoom candidate{xDist(_rng), yDist(_rng), rw, rh, templateIndex};
        if (overlapsExistingRoom(candidate, generatedRooms)) {
            continue;
        }

        if (!generatedRooms.empty()) {
            int nearestDistance = std::numeric_limits<int>::max();
            for (const GridRoom& existing : generatedRooms) {
                nearestDistance = std::min(nearestDistance, hallwayDistance(candidate, existing));
            }
            if (nearestDistance > maxHallwayLength) {
                continue;
            }
        }
        generatedRooms.push_back(candidate);
    }

    if (generatedRooms.empty()) {
        int fallbackW = std::clamp((widthMin + widthMax) / 2, 6, std::max(6, _gridWidth - 2));
        int fallbackH = std::clamp((heightMin + heightMax) / 2, 6, std::max(6, _gridHeight - 2));
        int fallbackX = std::clamp(_gridWidth / 2 - fallbackW / 2, 1, std::max(1, _gridWidth - fallbackW - 1));
        int fallbackY = std::clamp(_gridHeight / 2 - fallbackH / 2, 1, std::max(1, _gridHeight - fallbackH - 1));
        generatedRooms.push_back(GridRoom{fallbackX, fallbackY, fallbackW, fallbackH, -1});
    }

    for (size_t ii = 0; ii < generatedRooms.size(); ++ii) {
        carveRoom(generatedRooms[ii], (int)ii);
    }

    std::vector<std::pair<int, int>> roomLinks;
    auto pushRoomLink = [&](int a, int b) {
        if (a == b) {
            return;
        }
        if (a > b) {
            std::swap(a, b);
        }
        for (const auto& edge : roomLinks) {
            if (edge.first == a && edge.second == b) {
                return;
            }
        }
        roomLinks.emplace_back(a, b);
    };

    for (size_t ii = 1; ii < generatedRooms.size(); ++ii) {
        Vec2 centerA((float)(generatedRooms[ii].x + generatedRooms[ii].w / 2),
                     (float)(generatedRooms[ii].y + generatedRooms[ii].h / 2));

        float bestDist2 = std::numeric_limits<float>::max();
        size_t bestIndex = 0;
        for (size_t jj = 0; jj < ii; ++jj) {
            Vec2 centerB((float)(generatedRooms[jj].x + generatedRooms[jj].w / 2),
                         (float)(generatedRooms[jj].y + generatedRooms[jj].h / 2));
            float dist2 = (centerA - centerB).lengthSquared();
            if (dist2 < bestDist2) {
                bestDist2 = dist2;
                bestIndex = jj;
            }
        }

        Vec2 centerB((float)(generatedRooms[bestIndex].x + generatedRooms[bestIndex].w / 2),
                     (float)(generatedRooms[bestIndex].y + generatedRooms[bestIndex].h / 2));
        carveHallway(centerA, centerB);
        pushRoomLink((int)ii, (int)bestIndex);
    }

    size_t extraStep = (size_t)std::max(2, _config.extraHallConnectionStep);
    size_t extraOffset = extraStep - 1;
    for (size_t ii = 0; ii + extraOffset < generatedRooms.size(); ii += extraStep) {
        if (hallwayDistance(generatedRooms[ii], generatedRooms[ii + extraOffset]) > maxHallwayLength) {
            continue;
        }
        Vec2 centerA((float)(generatedRooms[ii].x + generatedRooms[ii].w / 2),
                     (float)(generatedRooms[ii].y + generatedRooms[ii].h / 2));
        Vec2 centerB((float)(generatedRooms[ii + extraOffset].x + generatedRooms[ii + extraOffset].w / 2),
                     (float)(generatedRooms[ii + extraOffset].y + generatedRooms[ii + extraOffset].h / 2));
        carveHallway(centerA, centerB);
        pushRoomLink((int)ii, (int)(ii + extraOffset));
    }

    for (const GridRoom& room : generatedRooms) {
        placeObstaclesInRoom(room);
    }

    buildWalls();

    _rooms.reserve(generatedRooms.size());
    for (size_t ii = 0; ii < generatedRooms.size(); ++ii) {
        const GridRoom& room = generatedRooms[ii];
        Rect worldRect(_origin.x + room.x * _tileSize,
                       _origin.y + room.y * _tileSize,
                       room.w * _tileSize,
                       room.h * _tileSize);

        Color4 color = _config.roomPalette[ii % _config.roomPalette.size()];
        std::string activeModifier;
        if (room.templateIndex >= 0 && room.templateIndex < (int)_config.roomTemplates.size()) {
            const RoomTemplateConfig& templ = _config.roomTemplates[(size_t)room.templateIndex];
            if (templ.hasColor) {
                color = templ.color;
            }
            if (!templ.possibleModifiers.empty()) {
                std::uniform_int_distribution<size_t> modifierDist(0, templ.possibleModifiers.size() - 1);
                activeModifier = templ.possibleModifiers[modifierDist(_rng)];
            }
        }
        _rooms.emplace_back((int)ii, worldRect, color, activeModifier);
    }

    for (const auto& edge : roomLinks) {
        if (edge.first < 0 || edge.second < 0 ||
            edge.first >= (int)_rooms.size() || edge.second >= (int)_rooms.size()) {
            continue;
        }
        _rooms[(size_t)edge.first].addConnection(edge.second);
        _rooms[(size_t)edge.second].addConnection(edge.first);
    }

    seedRoomSpawns(generatedRooms);

    if (!generatedRooms.empty()) {
        const GridRoom& start = generatedRooms.front();
        _playerSpawn = tileToWorldCenter(start.x + start.w / 2, start.y + start.h / 2);
        _playerSpawn = findNearestWalkableWorld(_playerSpawn);
    }

    return !_rooms.empty();
}

void Map::draw(const std::shared_ptr<SpriteBatch>& batch) const {
    if (!batch || _tiles.empty()) {
        return;
    }

    batch->setTexture(nullptr);

    Rect backdrop(_worldBounds.origin.x, _worldBounds.origin.y,
                  _worldBounds.size.width, _worldBounds.size.height);
    batch->setColor(_config.backdropColor);
    batch->fill(backdrop);

    for (int yy = 0; yy < _gridHeight; ++yy) {
        for (int xx = 0; xx < _gridWidth; ++xx) {
            TileType tile = getTile(xx, yy);
            if (tile == TileType::EMPTY) {
                continue;
            }

            float px = _origin.x + xx * _tileSize;
            float py = _origin.y + yy * _tileSize;
            Rect cell(px, py, _tileSize, _tileSize);

            Color4 color = _config.backdropColor;
            if (tile == TileType::ROOM_FLOOR) {
                color = getFloorColorForTile(xx, yy);
            } else if (tile == TileType::HALL_FLOOR) {
                color = _config.hallFloorColor;
            } else if (tile == TileType::WALL) {
                color = _config.wallTopColor;
            } else if (tile == TileType::OBSTACLE) {
                color = _config.obstacleFaceColor;
            }

            batch->setColor(color);
            batch->fill(cell);
        }
    }
}

bool Map::canOccupy(const Vec2& worldPos, float radius) const {
    if (_tiles.empty()) {
        return false;
    }

    static const std::array<Vec2, 9> OFFSETS = {
        Vec2(0.0f, 0.0f),
        Vec2(1.0f, 0.0f), Vec2(-1.0f, 0.0f),
        Vec2(0.0f, 1.0f), Vec2(0.0f, -1.0f),
        Vec2(0.7f, 0.7f), Vec2(-0.7f, 0.7f),
        Vec2(0.7f, -0.7f), Vec2(-0.7f, -0.7f)
    };

    for (const Vec2& offset : OFFSETS) {
        Vec2 sample = worldPos + offset * radius;
        int tx = 0;
        int ty = 0;
        if (!worldToTile(sample, tx, ty)) {
            return false;
        }
        if (!isWalkableTile(tx, ty)) {
            return false;
        }
    }
    return true;
}

Vec2 Map::resolveSlideMove(const Vec2& startPos, const Vec2& velocity, float radius) const {
    Vec2 resolved = startPos;

    if (std::fabs(velocity.x) > 0.0001f) {
        Vec2 testX = resolved + Vec2(velocity.x, 0.0f);
        if (canOccupy(testX, radius)) {
            resolved.x = testX.x;
        }
    }

    if (std::fabs(velocity.y) > 0.0001f) {
        Vec2 testY = resolved + Vec2(0.0f, velocity.y);
        if (canOccupy(testY, radius)) {
            resolved.y = testY.y;
        }
    }

    return resolved;
}

Vec2 Map::getNextWaypoint(const Vec2& fromWorld, const Vec2& toWorld) const {
    if (_tiles.empty()) {
        return toWorld;
    }

    int sx = 0;
    int sy = 0;
    int gx = 0;
    int gy = 0;
    if (!worldToTile(fromWorld, sx, sy) || !worldToTile(toWorld, gx, gy)) {
        return toWorld;
    }

    int startIndex = findNearestWalkableTileIndex(sx, sy);
    int goalIndex = findNearestWalkableTileIndex(gx, gy);
    if (startIndex < 0 || goalIndex < 0) {
        return toWorld;
    }
    if (startIndex == goalIndex) {
        return toWorld;
    }

    std::vector<int> parent((size_t)(_gridWidth * _gridHeight), -1);
    std::vector<uint8_t> visited((size_t)(_gridWidth * _gridHeight), 0);
    std::queue<int> frontier;
    frontier.push(startIndex);
    visited[(size_t)startIndex] = 1;

    int reached = -1;
    int fallback = startIndex;

    int goalX = goalIndex % _gridWidth;
    int goalY = goalIndex / _gridWidth;

    auto manhattanToGoal = [&](int idx) {
        int xx = idx % _gridWidth;
        int yy = idx / _gridWidth;
        return std::abs(xx - goalX) + std::abs(yy - goalY);
    };

    static const std::array<Vec2, 4> DIRS = {
        Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1)
    };

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();

        if (manhattanToGoal(current) < manhattanToGoal(fallback)) {
            fallback = current;
        }

        if (current == goalIndex) {
            reached = current;
            break;
        }

        int cx = current % _gridWidth;
        int cy = current / _gridWidth;
        for (const Vec2& dir : DIRS) {
            int nx = cx + (int)dir.x;
            int ny = cy + (int)dir.y;
            if (!inBounds(nx, ny) || !isWalkableTile(nx, ny)) {
                continue;
            }
            int nextIndex = toIndex(nx, ny);
            if (visited[(size_t)nextIndex]) {
                continue;
            }
            visited[(size_t)nextIndex] = 1;
            parent[(size_t)nextIndex] = current;
            frontier.push(nextIndex);
        }
    }

    if (reached < 0) {
        reached = fallback;
    }
    if (reached == startIndex) {
        return tileToWorldCenter(startIndex % _gridWidth, startIndex / _gridWidth);
    }

    int step = reached;
    while (parent[(size_t)step] != -1 && parent[(size_t)step] != startIndex) {
        step = parent[(size_t)step];
    }

    return tileToWorldCenter(step % _gridWidth, step / _gridWidth);
}

Vec2 Map::findNearestWalkableWorld(const Vec2& fromWorld) const {
    int tx = 0;
    int ty = 0;
    if (!worldToTile(fromWorld, tx, ty)) {
        tx = (int)std::clamp((fromWorld.x - _origin.x) / _tileSize, 0.0f, (float)(_gridWidth - 1));
        ty = (int)std::clamp((fromWorld.y - _origin.y) / _tileSize, 0.0f, (float)(_gridHeight - 1));
    }

    int nearest = findNearestWalkableTileIndex(tx, ty);
    if (nearest < 0) {
        return fromWorld;
    }
    return tileToWorldCenter(nearest % _gridWidth, nearest / _gridWidth);
}

int Map::findRoomIndexForWorld(const Vec2& worldPos) const {
    for (size_t ii = 0; ii < _rooms.size(); ++ii) {
        if (_rooms[ii].contains(worldPos)) {
            return (int)ii;
        }
    }
    return -1;
}

const std::vector<int>& Map::getConnectedRooms(int roomIndex) const {
    static const std::vector<int> EMPTY;
    if (roomIndex < 0 || roomIndex >= (int)_rooms.size()) {
        return EMPTY;
    }
    return _rooms[(size_t)roomIndex].getConnectedRooms();
}

bool Map::inBounds(int x, int y) const {
    return x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight;
}

int Map::toIndex(int x, int y) const {
    return y * _gridWidth + x;
}

Map::TileType Map::getTile(int x, int y) const {
    if (!inBounds(x, y)) {
        return TileType::EMPTY;
    }
    return _tiles[(size_t)toIndex(x, y)];
}

void Map::setTile(int x, int y, TileType tile, int roomIndex) {
    if (!inBounds(x, y)) {
        return;
    }

    int idx = toIndex(x, y);
    _tiles[(size_t)idx] = tile;
    if (roomIndex >= 0) {
        _tileRoomIndex[(size_t)idx] = roomIndex;
    } else if (tile == TileType::EMPTY || tile == TileType::HALL_FLOOR || tile == TileType::WALL) {
        _tileRoomIndex[(size_t)idx] = -1;
    }
}

bool Map::isWalkableTile(int x, int y) const {
    TileType tile = getTile(x, y);
    return tile == TileType::ROOM_FLOOR || tile == TileType::HALL_FLOOR;
}

bool Map::worldToTile(const Vec2& worldPos, int& outX, int& outY) const {
    if (_tileSize <= 0.0f) {
        return false;
    }

    float gx = (worldPos.x - _origin.x) / _tileSize;
    float gy = (worldPos.y - _origin.y) / _tileSize;
    outX = (int)std::floor(gx);
    outY = (int)std::floor(gy);
    return inBounds(outX, outY);
}

Vec2 Map::tileToWorldCenter(int x, int y) const {
    return Vec2(_origin.x + (x + 0.5f) * _tileSize,
                _origin.y + (y + 0.5f) * _tileSize);
}

bool Map::overlapsExistingRoom(const GridRoom& candidate, const std::vector<GridRoom>& rooms) const {
    int roomBuffer = std::max(0, _config.roomBufferTiles);
    int cLeft = candidate.x - roomBuffer;
    int cRight = candidate.x + candidate.w + roomBuffer - 1;
    int cBottom = candidate.y - roomBuffer;
    int cTop = candidate.y + candidate.h + roomBuffer - 1;

    for (const GridRoom& room : rooms) {
        int rLeft = room.x;
        int rRight = room.x + room.w - 1;
        int rBottom = room.y;
        int rTop = room.y + room.h - 1;

        bool overlapX = cLeft <= rRight && cRight >= rLeft;
        bool overlapY = cBottom <= rTop && cTop >= rBottom;
        if (overlapX && overlapY) {
            return true;
        }
    }
    return false;
}

void Map::carveRoom(const GridRoom& room, int roomIndex) {
    for (int yy = room.y; yy < room.y + room.h; ++yy) {
        for (int xx = room.x; xx < room.x + room.w; ++xx) {
            setTile(xx, yy, TileType::ROOM_FLOOR, roomIndex);
        }
    }
}

void Map::carveHallway(const Vec2& from, const Vec2& to) {
    int x1 = (int)from.x;
    int y1 = (int)from.y;
    int x2 = (int)to.x;
    int y2 = (int)to.y;
    int hallThickness = std::max(1, _config.hallThickness);

    bool horizontalFirst = std::uniform_int_distribution<int>(0, 1)(_rng) == 0;
    if (horizontalFirst) {
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            for (int offset = 0; offset < hallThickness; ++offset) {
                setTile(xx, y1 + offset, TileType::HALL_FLOOR);
            }
        }
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            for (int offset = 0; offset < hallThickness; ++offset) {
                setTile(x2 + offset, yy, TileType::HALL_FLOOR);
            }
        }
    } else {
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            for (int offset = 0; offset < hallThickness; ++offset) {
                setTile(x1 + offset, yy, TileType::HALL_FLOOR);
            }
        }
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            for (int offset = 0; offset < hallThickness; ++offset) {
                setTile(xx, y2 + offset, TileType::HALL_FLOOR);
            }
        }
    }
}

void Map::placeObstaclesInRoom(const GridRoom& room) {
    int obstacleCountMin = std::max(0, _config.obstacleCountMin);
    int obstacleCountMax = std::max(obstacleCountMin, _config.obstacleCountMax);
    int obstacleSizeMin = std::max(1, _config.obstacleSizeMin);
    int obstacleSizeMax = std::max(obstacleSizeMin, _config.obstacleSizeMax);
    std::uniform_int_distribution<int> obstacleCountDist(obstacleCountMin, obstacleCountMax);
    std::uniform_int_distribution<int> obstacleSizeDist(obstacleSizeMin, obstacleSizeMax);
    int targetCount = obstacleCountDist(_rng);

    int placed = 0;
    int tries = 0;
    int maxPlacementTries = std::max(1, _config.obstaclePlacementAttempts);
    int centerX = room.x + room.w / 2;
    int centerY = room.y + room.h / 2;
    int obstacleMargin = std::max(1, _config.obstacleMarginTiles);
    int centerClearance = std::max(0, _config.obstacleCenterClearanceTiles);

    while (placed < targetCount && tries < maxPlacementTries) {
        ++tries;
        int ow = obstacleSizeDist(_rng);
        int oh = obstacleSizeDist(_rng);

        int minX = room.x + obstacleMargin;
        int minY = room.y + obstacleMargin;
        int maxX = room.x + room.w - ow - obstacleMargin - 1;
        int maxY = room.y + room.h - oh - obstacleMargin - 1;
        if (maxX < minX || maxY < minY) {
            continue;
        }

        std::uniform_int_distribution<int> xDist(minX, maxX);
        std::uniform_int_distribution<int> yDist(minY, maxY);
        int ox = xDist(_rng);
        int oy = yDist(_rng);

        int obstacleCenterX = ox + ow / 2;
        int obstacleCenterY = oy + oh / 2;
        if (std::abs(obstacleCenterX - centerX) <= centerClearance &&
            std::abs(obstacleCenterY - centerY) <= centerClearance) {
            continue;
        }

        bool canPlace = true;
        for (int yy = oy; yy < oy + oh && canPlace; ++yy) {
            for (int xx = ox; xx < ox + ow; ++xx) {
                if (getTile(xx, yy) != TileType::ROOM_FLOOR) {
                    canPlace = false;
                    break;
                }
            }
        }
        if (!canPlace) {
            continue;
        }

        for (int yy = oy; yy < oy + oh; ++yy) {
            for (int xx = ox; xx < ox + ow; ++xx) {
                setTile(xx, yy, TileType::OBSTACLE);
            }
        }
        ++placed;
    }
}

void Map::buildWalls() {
    std::vector<TileType> source = _tiles;

    auto sourceTile = [&](int x, int y) {
        if (!inBounds(x, y)) {
            return TileType::EMPTY;
        }
        return source[(size_t)toIndex(x, y)];
    };

    for (int yy = 0; yy < _gridHeight; ++yy) {
        for (int xx = 0; xx < _gridWidth; ++xx) {
            if (sourceTile(xx, yy) != TileType::EMPTY) {
                continue;
            }

            bool shouldBeWall = false;
            for (int dy = -1; dy <= 1 && !shouldBeWall; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    TileType neighbor = sourceTile(xx + dx, yy + dy);
                    if (neighbor == TileType::ROOM_FLOOR ||
                        neighbor == TileType::HALL_FLOOR ||
                        neighbor == TileType::OBSTACLE) {
                        shouldBeWall = true;
                        break;
                    }
                }
            }

            if (shouldBeWall) {
                setTile(xx, yy, TileType::WALL);
            }
        }
    }
}

Vec2 Map::pickRandomWalkableInRoom(const GridRoom& room, const Vec2* avoid) {
    int minX = room.x + 1;
    int maxX = room.x + room.w - 2;
    int minY = room.y + 1;
    int maxY = room.y + room.h - 2;

    if (maxX < minX || maxY < minY) {
        Vec2 center = tileToWorldCenter(room.x + room.w / 2, room.y + room.h / 2);
        return findNearestWalkableWorld(center);
    }

    std::uniform_int_distribution<int> xDist(minX, maxX);
    std::uniform_int_distribution<int> yDist(minY, maxY);
    float minSpacing = _tileSize * 1.5f;
    float minSpacing2 = minSpacing * minSpacing;
    int sampleAttempts = 24;

    for (int attempt = 0; attempt < sampleAttempts; ++attempt) {
        int tx = xDist(_rng);
        int ty = yDist(_rng);
        if (!isWalkableTile(tx, ty)) {
            continue;
        }

        Vec2 world = tileToWorldCenter(tx, ty);
        if (avoid && (world - *avoid).lengthSquared() < minSpacing2) {
            continue;
        }
        return world;
    }

    Vec2 fallback = tileToWorldCenter(room.x + room.w / 2 + _config.primaryItemOffsetX,
                                      room.y + room.h / 2 + _config.primaryItemOffsetY);
    fallback = findNearestWalkableWorld(fallback);

    if (avoid && (fallback - *avoid).lengthSquared() < minSpacing2) {
        Vec2 nudged = tileToWorldCenter(room.x + room.w / 2 + _config.secondaryItemOffsetX,
                                        room.y + room.h / 2 + _config.secondaryItemOffsetY);
        fallback = findNearestWalkableWorld(nudged);
    }

    return fallback;
}

void Map::seedRoomSpawns(const std::vector<GridRoom>& rooms) {
    _itemSpawnPoints.clear();
    _enemySpawnPoints.clear();

    for (size_t ii = 0; ii < rooms.size(); ++ii) {
        const GridRoom& room = rooms[ii];

        Vec2 itemSpot = pickRandomWalkableInRoom(room, nullptr);
        _itemSpawnPoints.push_back(itemSpot);
        if (ii < _rooms.size()) {
            _rooms[ii].addItemSpawn(itemSpot);
        }

        bool hasSecondItemSpot = false;
        Vec2 secondSpot = Vec2::ZERO;
        if (room.w >= _config.largeRoomWidthThreshold && room.h >= _config.largeRoomHeightThreshold) {
            secondSpot = pickRandomWalkableInRoom(room, &itemSpot);
            _itemSpawnPoints.push_back(secondSpot);
            if (ii < _rooms.size()) {
                _rooms[ii].addItemSpawn(secondSpot);
            }
            hasSecondItemSpot = true;
        }

        if ((int)ii < std::max(0, _config.skipEnemySpawnsInFirstRooms)) {
            continue;
        }

        const Vec2* avoid = hasSecondItemSpot ? &secondSpot : &itemSpot;
        Vec2 enemySpot = pickRandomWalkableInRoom(room, avoid);
        _enemySpawnPoints.push_back(enemySpot);
        if (ii < _rooms.size()) {
            _rooms[ii].addEnemySpawn(enemySpot);
        }
    }

    if (_itemSpawnPoints.empty() && !rooms.empty()) {
        _itemSpawnPoints.push_back(pickRandomWalkableInRoom(rooms.front(), nullptr));
    }
    if (_enemySpawnPoints.empty() && rooms.size() > 1) {
        _enemySpawnPoints.push_back(pickRandomWalkableInRoom(rooms.back(), nullptr));
    }
}

Color4 Map::getFloorColorForTile(int x, int y) const {
    int idx = toIndex(x, y);
    int roomIndex = _tileRoomIndex[(size_t)idx];
    if (roomIndex < 0 || roomIndex >= (int)_rooms.size()) {
        return _config.roomPalette.empty() ? Color4(118, 146, 108, 255) : _config.roomPalette.front();
    }
    return _rooms[(size_t)roomIndex].getDebugColor();
}

int Map::findNearestWalkableTileIndex(int startX, int startY) const {
    if (_tiles.empty()) {
        return -1;
    }

    int clampedX = std::clamp(startX, 0, _gridWidth - 1);
    int clampedY = std::clamp(startY, 0, _gridHeight - 1);

    int startIndex = toIndex(clampedX, clampedY);
    if (isWalkableTile(clampedX, clampedY)) {
        return startIndex;
    }

    std::vector<uint8_t> visited((size_t)(_gridWidth * _gridHeight), 0);
    std::queue<int> frontier;
    frontier.push(startIndex);
    visited[(size_t)startIndex] = 1;

    static const std::array<Vec2, 4> DIRS = {
        Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1)
    };

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();

        int cx = current % _gridWidth;
        int cy = current / _gridWidth;
        if (isWalkableTile(cx, cy)) {
            return current;
        }

        for (const Vec2& dir : DIRS) {
            int nx = cx + (int)dir.x;
            int ny = cy + (int)dir.y;
            if (!inBounds(nx, ny)) {
                continue;
            }
            int nextIndex = toIndex(nx, ny);
            if (visited[(size_t)nextIndex]) {
                continue;
            }
            visited[(size_t)nextIndex] = 1;
            frontier.push(nextIndex);
        }
    }

    return -1;
}

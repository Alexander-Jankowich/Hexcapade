// this file generates and renders a full run layout made of rooms, halls,
// walls, and room-based spawn points

#include "Map.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>
#include <utility>
#include "TiledRoomLoader.h"
#include "RectFactory.h"

#define PHYSICS_SCALE 100


using namespace cugl;
using namespace cugl::graphics;

namespace {
    // tiled stores flip flags inside the high bits of the gid, so the draw
    // path needs to peel those back off before it looks up the base tile art
    constexpr unsigned int kTiledHorizontalFlipFlag = 0x80000000u;
    constexpr unsigned int kTiledVerticalFlipFlag = 0x40000000u;
    constexpr unsigned int kTiledDiagonalFlipFlag = 0x20000000u;
    constexpr unsigned int kTiledFlipMask = 0xE0000000u;

    // this strips the tiled flip bits off so we can look up the real tile id.
    unsigned int stripTiledFlipFlags(unsigned int rawGid) {
        return rawGid & ~kTiledFlipMask;
    }

    // these tiny helpers let the draw path ask for each authored tiled flip directly.
    bool hasTiledHorizontalFlip(unsigned int rawGid) {
        return (rawGid & kTiledHorizontalFlipFlag) != 0;
    }

    bool hasTiledVerticalFlip(unsigned int rawGid) {
        return (rawGid & kTiledVerticalFlipFlag) != 0;
    }

    bool hasTiledDiagonalFlip(unsigned int rawGid) {
        return (rawGid & kTiledDiagonalFlipFlag) != 0;
    }

    // this is the tiny 2x2 matrix we use for authored tiled flip transforms.
    struct TileTransform2D {
        float m11 = 1.0f;
        float m12 = 0.0f;
        float m21 = 0.0f;
        float m22 = 1.0f;
    };

    // this multiplies two small tile transforms so draw can apply them together.
    TileTransform2D multiplyTransform(const TileTransform2D& a, const TileTransform2D& b) {
        TileTransform2D result;
        result.m11 = a.m11 * b.m11 + a.m12 * b.m21;
        result.m12 = a.m11 * b.m12 + a.m12 * b.m22;
        result.m21 = a.m21 * b.m11 + a.m22 * b.m21;
        result.m22 = a.m21 * b.m12 + a.m22 * b.m22;
        return result;
    }

    // tiled flip flags represent affine transforms too, so we convert them
    // into one small 2x2 matrix for the draw call...?
    TileTransform2D makeTileTransform(bool flipHorizontal, bool flipVertical,
                                      bool flipDiagonal) {
        TileTransform2D raw;

        if (flipDiagonal) {
            raw = multiplyTransform(TileTransform2D{0.0f, 1.0f, 1.0f, 0.0f}, raw);
        }
        if (flipHorizontal) {
            raw = multiplyTransform(TileTransform2D{-1.0f, 0.0f, 0.0f, 1.0f}, raw);
        }
        if (flipVertical) {
            raw = multiplyTransform(TileTransform2D{1.0f, 0.0f, 0.0f, -1.0f}, raw);
        }

        return raw;
    }
}

Map::Map() :
_gridWidth(0),
_gridHeight(0),
_tileSize(0.0f),
_origin(Vec2::ZERO),
_worldBounds(Rect::ZERO),
_playerSpawn(Vec2::ZERO),
_config(),
_hallFloorGid(0),
_tiledRoomPrefab(nullptr) {
}

// FLAG: Serena added
void Map::init() {
    _tiledRoomPrefab = TiledRoomLoader::loadFromAsset("example_3.tmj");
    if (!_tiledRoomPrefab) {
        CULog("tiled room load failed: tiles/example_2.tmj :(");
    }
}
//Alex flood- literally # islands leetcode esque 
void floodFill(std::vector<std::vector<Map::TileType>>& tileGrid) {
    std::queue<cugl::Vec2> queue;

    bool found = false;
    for (int i = 0; i < tileGrid.size() && !found; ++i) {
        for (int j = 0; j < tileGrid[i].size() && !found; ++j) {
            if (tileGrid[i][j] == Map::TileType::EMPTY) {
                queue.push(cugl::Vec2(i, j));
                tileGrid[i][j] = Map::TileType::HALL_FLOOR;
                found = true;
            }
        }
    }

    if (!found) return;

    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};

    while (!queue.empty()) {
        cugl::Vec2 node = queue.front();
        queue.pop();

        for (int dir = 0; dir < 4; ++dir) {
            int nx = node.x + dx[dir];
            int ny = node.y + dy[dir];

            if (nx >= 0 && nx < tileGrid.size() &&
                ny >= 0 && ny < tileGrid[nx].size() &&
                tileGrid[nx][ny] == Map::TileType::EMPTY)
            {
                tileGrid[nx][ny] = Map::TileType::HALL_FLOOR;
                queue.push(cugl::Vec2(nx, ny));
            }
        }
    }
}

bool Map::generate(const Size& viewSize, const MapGenerationConfig& config, unsigned int seed) {
    CULog("seed: %d", seed);
    // this now builds the map from one repeated tiled room (ex) instead of carving
    // each room shape procedurally. we still keep the current map runtime,
    // hallway logic, collision helpers, and room controller on top of it just in case
    _config = config;

    if (_config.roomPalette.empty()) {
        _config.roomPalette.push_back(Color4(118, 146, 108, 255));
    }

    CUAssertLog(seed!=0, "map generation seed is zero");
    _rng.initWithSeed(seed);

    // this is the one room prefab we repeat for the first tiled-backed map;
    // later we can swap this for a pool of tmj room files without changing
    // the rest of the map/runtime architecture!

    // we use the tiled tile size as the real map tile size so the rendered art,
    // collision grid, and world positions stay aligned (ask deisgners)
    _tileSize = std::max(8.0f, (float)_tiledRoomPrefab->getTileWidth());
    CULog("TIle Size %f", _tileSize);
    //int clampedMinRooms = std::max(1, _config.roomCountMin);
//    int clampedMaxRooms = std::max(clampedMinRooms, _config.roomCountMax);
//    std::uniform_int_distribution<int> roomCountDist(clampedMinRooms, clampedMaxRooms);
    // FLAG: RANDOM INT
    int targetRoomCount = std::max(_config.roomCountMin, (int) (_rng.getDouble() * _config.roomCountMax));

    bool useTemplates = !_config.roomTemplates.empty();
    int prefabWidth = std::max(1, _tiledRoomPrefab->getWidth());
    int prefabHeight = std::max(1, _tiledRoomPrefab->getHeight());

    // for now we stay on one hallway layout mode so the parser/render changes
    // are easier to validate against what tiled itself shows
    float worldScaleX = std::max(1.0f, _config.worldScaleX);
    float worldScaleY = std::max(1.0f, _config.worldScaleY);
    _gridWidth = std::max(std::max(16, _config.worldMinGridWidth),
                          (int)std::ceil((viewSize.width / _tileSize) * worldScaleX));
    _gridHeight = std::max(std::max(16, _config.worldMinGridHeight),
                           (int)std::ceil((viewSize.height / _tileSize) * worldScaleY));

    _origin = Vec2::ZERO;
    _worldBounds = Rect(_origin.x, _origin.y, _gridWidth * _tileSize, _gridHeight * _tileSize);
    
    for (int col = 0; col < _gridWidth; col++){
        std::vector<TileType> v(_gridHeight);
        _tileGrid.push_back(v);
    }

    _tiles.assign((size_t)(_gridWidth * _gridHeight), TileType::EMPTY);
    _tileRoomIndex.assign((size_t)(_gridWidth * _gridHeight), -1);
    clearRenderLayers();
    _rooms.clear();
    _itemSpawnPoints.clear();
    _enemySpawnPoints.clear();
    _playerSpawn = Vec2::ZERO;
    _hallFloorGid = _tiledRoomPrefab->getPrimaryFloorGid();

    std::vector<GridRoom> generatedRooms;
    //generatedRooms.reserve((size_t)targetRoomCount);
    int maxHallwayLength = std::max(1, _config.maxHallwayLengthTiles);

    auto hallwayDistance = [](const GridRoom& a, const GridRoom& b) {
        int ax = a.x + a.w / 2;
        int ay = a.y + a.h / 2;
        int bx = b.x + b.w / 2;
        int by = b.y + b.h / 2;
        return std::abs(ax - bx) + std::abs(ay - by);
    };


    // the new tiled path still uses the old high-level room placement logic,
    // but every room now has the same shape because it comes from the tmj prefab.
    int edgePadding = std::max(1, _config.roomEdgePadding);
    int placementAttempts = std::max(1, _config.roomPlacementAttempts);
    // TODO this needs to be a while loop probably??
    for (int attempt = 0; attempt < placementAttempts && (int)generatedRooms.size() < targetRoomCount; ++attempt) {
        int rw = prefabWidth;
        int rh = prefabHeight;
        int templateIndex = useTemplates ? (int)(generatedRooms.size() % _config.roomTemplates.size()) : -1;

        int maxX = _gridWidth - rw - edgePadding - 1;
        int maxY = _gridHeight - rh - edgePadding - 1;
        if (maxX < edgePadding || maxY < edgePadding) {
            continue;
        }

//        std::uniform_int_distribution<int> xDist(edgePadding, maxX);
//        std::uniform_int_distribution<int> yDist(edgePadding, maxY);
        // FLAG: RANDOM
        int x = std::max((int)(_rng.getDouble() * maxX), edgePadding);
        int y = std::max((int)(_rng.getDouble() * maxY), edgePadding);

        GridRoom candidate{x, y, rw, rh, templateIndex};
        if (overlapsExistingRoom(candidate, generatedRooms)) {
            continue;
        }

        if (!generatedRooms.empty()) {
            int nearestDistance = INT_MAX;
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
        // if room placement fails, we still drop one room in the center so the
        // run stays playable instead of booting into an empty map (for debugging originally)
        int centerRoomW = std::min(prefabWidth, std::max(1, _gridWidth - 2));
        int centerRoomH = std::min(prefabHeight, std::max(1, _gridHeight - 2));
        int centerRoomX = std::clamp(_gridWidth / 2 - centerRoomW / 2, 1, std::max(1, _gridWidth - centerRoomW - 1));
        int centerRoomY = std::clamp(_gridHeight / 2 - centerRoomH / 2, 1, std::max(1, _gridHeight - centerRoomH - 1));
        generatedRooms.push_back(GridRoom{centerRoomX, centerRoomY, centerRoomW, centerRoomH,
                                          useTemplates ? 0 : -1});
    }

    // this stamps the tiled room layers directly into the map grid (?) and the
    // map-wide render layers before we carve hallways between room instances.
    for (size_t ii = 0; ii < generatedRooms.size(); ++ii) {
        stampTiledRoom(generatedRooms[ii], (int)ii);
    }
    
    floodFill(_tileGrid);
        
    
//    std::vector<std::pair<int, int>> roomLinks;
//    auto pushRoomLink = [&](int a, int b) {
//        if (a == b) {
//            return;
//        }
//        if (a > b) {
//            std::swap(a, b);
//        }
//        for (const auto& edge : roomLinks) {
//            if (edge.first == a && edge.second == b) {
//                return;
//            }
//        }
//        roomLinks.emplace_back(a, b);
//    };
//
//    for (size_t ii = 1; ii < generatedRooms.size(); ++ii) {
//        float bestDist2 = std::numeric_limits<float>::max();
//        size_t bestIndex = 0;
//        for (size_t jj = 0; jj < ii; ++jj) {
//            Vec2 centerA((float)(generatedRooms[ii].x + generatedRooms[ii].w / 2),
//                         (float)(generatedRooms[ii].y + generatedRooms[ii].h / 2));
//            Vec2 centerB((float)(generatedRooms[jj].x + generatedRooms[jj].w / 2),
//                         (float)(generatedRooms[jj].y + generatedRooms[jj].h / 2));
//            float dist2 = (centerA - centerB).lengthSquared();
//            if (dist2 < bestDist2) {
//                bestDist2 = dist2;
//                bestIndex = jj;
//            }
//        }
//
//        // old center-to-center hall anchors are what made hallways bulldoze
//        // through the middle of the room art. the new anchors try to connect
//        // from room edges so we only open a doorway-sized cut into the prefab.
//        Vec2 hallStartAnchor = pickHallAnchorTile(generatedRooms[ii], generatedRooms[bestIndex]);
//        Vec2 hallEndAnchor = pickHallAnchorTile(generatedRooms[bestIndex], generatedRooms[ii]);
//        openDoorwayForHallAnchor(generatedRooms[ii], generatedRooms[bestIndex], hallStartAnchor);
//        openDoorwayForHallAnchor(generatedRooms[bestIndex], generatedRooms[ii], hallEndAnchor);
//
//        // the doorway stays on the room boundary, but the hallway itself starts
//        // one tile outside the room so it does not paint over the room edge.
//        Vec2 hallStart = getExteriorHallTileForAnchor(generatedRooms[ii], hallStartAnchor);
//        Vec2 hallEnd = getExteriorHallTileForAnchor(generatedRooms[bestIndex], hallEndAnchor);
//        carveHallway(hallStart, hallEnd, (int)ii, (int)bestIndex);
//        pushRoomLink((int)ii, (int)bestIndex);
//    }


    // this small cleanup strips out isolated hall tabs that can be left behind
    // when the hallway brush touches a room edge but does not actually continue.
    //pruneHallArtifacts();

    _rooms.reserve(generatedRooms.size());
    for (size_t ii = 0; ii < generatedRooms.size(); ++ii) {
        const GridRoom& room = generatedRooms[ii];
        Rect worldRect(_origin.x + room.x * _tileSize,
                       _origin.y + room.y * _tileSize,
                       room.w * _tileSize,
                       room.h * _tileSize);
        // FLAG: what the fuck is this doing - serena
        Color4 color = _config.roomPalette[ii % _config.roomPalette.size()];
        std::string activeModifier;
        if (room.templateIndex >= 0 && room.templateIndex < (int)_config.roomTemplates.size()) {
            const RoomTemplateConfig& templ = _config.roomTemplates[(size_t)room.templateIndex];
            if (templ.hasColor) {
                color = templ.color;
            }
            if (!templ.possibleModifiers.empty()) {
                //std::uniform_int_distribution<size_t> modifierDist(0, templ.possibleModifiers.size() - 1);
                activeModifier = templ.possibleModifiers[(int) (_rng.getDouble() * (templ.possibleModifiers.size() - 1))];
            }
        }
        _rooms.emplace_back((int)ii, worldRect, color, activeModifier);
    }

//    for (const auto& edge : roomLinks) {
//        if (edge.first < 0 || edge.second < 0 ||
//            edge.first >= (int)_rooms.size() || edge.second >= (int)_rooms.size()) {
//            continue;
//        }
//        _rooms[(size_t)edge.first].addConnection(edge.second);
//        _rooms[(size_t)edge.second].addConnection(edge.first);
//    }

    // the new spawn pass uses the tiled markers and transforms them into
    // world-space positions for each repeated room instance.
    seedTiledRoomSpawns(generatedRooms);

    if (!generatedRooms.empty()) {
        const GridRoom& startRoom = generatedRooms.front();
        if (_tiledRoomPrefab->hasPlayerSpawn()) {
            _playerSpawn = roomLocalToWorld(startRoom, _tiledRoomPrefab->getPlayerSpawn().localPosition);
        } else {
            _playerSpawn = tileToWorldCenter((startRoom.x + startRoom.w / 2), (startRoom.y + startRoom.h / 2));
        }
        _playerSpawn = findNearestWalkableWorld(_playerSpawn);
    }

    return !_rooms.empty();
}

void Map::clearRenderLayers() {
    // these layers keep the stamped tiled art separate from the walkability grid.
    size_t gridSize = (size_t)(_gridWidth * _gridHeight);
    _floorRenderGids.assign(gridSize, 0);
    _wallRenderGids.assign(gridSize, 0);
    _decoRenderGids.assign(gridSize, 0);
    _visFixRenderGids.assign(gridSize, 0);
}

void Map::setRenderGid(std::vector<unsigned int>& layer, int x, int y, unsigned int gid) {
    // this writes one gid into the map-wide render layer for a specific tile.
    if (!inBounds(x, y)) {
        return;
    }
    layer[(size_t)toIndex(x, y)] = gid;
}

void Map::clearRenderTile(int x, int y) {
    // hall carving uses this to remove stamped room art where a hallway needs
    // to pass through a wall or room edge.
    if (!inBounds(x, y)) {
        return;
    }

    size_t index = (size_t)toIndex(x, y);
    _floorRenderGids[index] = 0;
    _wallRenderGids[index] = 0;
    _decoRenderGids[index] = 0;
    _visFixRenderGids[index] = 0;
}

void Map::stampTiledRoom(const GridRoom& room, int roomIndex) {
    // this copies one repeated tiled room into the current map grid.
    if (!_tiledRoomPrefab) {
        return;
    }

    for (int yy = 0; yy < room.h; ++yy) {
        for (int xx = 0; xx < room.w; ++xx) {
            int mapX = room.x + xx;
            int mapY = room.y + yy;
            int sourceX = xx;
            int sourceY = yy;
            // TODO: ensure this is not buggy bc it seems suspicious...
            // render layers keep the raw tiled gid so draw can still honor
            // authored flip flags. collision only cares about the base gid.
            unsigned int floorRawGid = _tiledRoomPrefab->getFloorLayer().getRawTileAt(sourceX, sourceY);
            unsigned int wallRawGid = _tiledRoomPrefab->getWallLayer().getRawTileAt(sourceX, sourceY);
            unsigned int decoRawGid = _tiledRoomPrefab->getDecoLayer().getRawTileAt(sourceX, sourceY);
            unsigned int visFixRawGid = _tiledRoomPrefab->getVisFixLayer().getRawTileAt(sourceX, sourceY);
            unsigned int floorGid = stripTiledFlipFlags(floorRawGid);
            unsigned int wallGid = stripTiledFlipFlags(wallRawGid);

            setRenderGid(_floorRenderGids, mapX, mapY, floorRawGid);
            setRenderGid(_wallRenderGids, mapX, mapY, wallRawGid);
            setRenderGid(_decoRenderGids, mapX, mapY, decoRawGid);
            setRenderGid(_visFixRenderGids, mapX, mapY, visFixRawGid);

            bool walkableFloor = _tiledRoomPrefab->hasWalkableFloorAt(sourceX, sourceY);
            bool hasAnyRoomArt = floorGid != 0 || wallGid != 0 ||
                                 stripTiledFlipFlags(decoRawGid) != 0 ||
                                 stripTiledFlipFlags(visFixRawGid) != 0;

            // only the connected interior floor region should be walkable.
            // decorative border floor tiles stay blocked so halls do not snap
            // to fake openings near the room frame art.
            if (walkableFloor) {
                setTile(mapX, mapY, TileType::ROOM_FLOOR, roomIndex);
                _tileGrid.at(mapX).at(mapY) = TileType::ROOM_FLOOR;
            } else if (hasAnyRoomArt) {
                setTile(mapX, mapY, TileType::WALL, roomIndex);
                _tileGrid.at(mapX).at(mapY) = TileType::WALL;

            }
        }
    }
}

cugl::Vec2 Map::roomLocalToWorld(const GridRoom& room, const Vec2& localPos) const {
    // tiled marker positions are local to the room prefab, so we offset them by
    // the repeated room instance origin to get their world-space position.
    return Vec2(_origin.x + room.x * _tileSize + localPos.x,
                _origin.y + room.y * _tileSize + localPos.y);
}

void Map::seedTiledRoomSpawns(const std::vector<GridRoom>& rooms) {
    // this reuses the existing room/spawn runtime, but the source positions now
    // come from tiled markers instead of random points inside the room.
    _itemSpawnPoints.clear();
    _enemySpawnPoints.clear();
    CULog("item spawn markers in prefab: %zu", _tiledRoomPrefab->getItemSpawns().size());
    if (!_tiledRoomPrefab) {
        return;
    }

    for (size_t ii = 0; ii < rooms.size(); ++ii) {
        const GridRoom& room = rooms[ii];

        for (const TiledRoomData::ObjectMarker& marker : _tiledRoomPrefab->getItemSpawns()) {
            Vec2 world = roomLocalToWorld(room, marker.localPosition);  // remove findNearestWalkableWorld
            _itemSpawnPoints.push_back(world);
            if (ii < _rooms.size()) {
                _rooms[ii].addItemSpawn(world);
            }
        }

        if ((int)ii >= std::max(0, _config.skipEnemySpawnsInFirstRooms)) {
            for (const TiledRoomData::ObjectMarker& marker : _tiledRoomPrefab->getEnemySpawns()) {
                Vec2 world = findNearestWalkableWorld(roomLocalToWorld(room, marker.localPosition));
                _enemySpawnPoints.push_back(world);
                if (ii < _rooms.size()) {
                    _rooms[ii].addEnemySpawn(world);
                }
            }
        }
    }

    if (_itemSpawnPoints.empty() && !rooms.empty()) {
        _itemSpawnPoints.push_back(findNearestWalkableWorld(
            roomLocalToWorld(rooms.front(), Vec2(_tileSize * 2.0f, _tileSize * 2.0f))
        ));
    }
    if (_enemySpawnPoints.empty() && !rooms.empty()) {
        _enemySpawnPoints.push_back(findNearestWalkableWorld(
            roomLocalToWorld(rooms.back(), Vec2(_tileSize * 2.0f, _tileSize * 2.0f))
        ));
    }
}

Vec2 Map::pickHallAnchorTile(const GridRoom& room, const GridRoom& otherRoom) const {
    /*
    // old inferred-opening scan kept here for reference. it was trying to use
    // the tmj edge art itself to find a doorway, but our sample room does not
    // author real openings on its border yet, so the result could still land
    // in awkward places near shelves or corners.
    if (!_tiledRoomPrefab) {
        return Vec2((float)(room.x + room.w / 2), (float)(room.y + room.h / 2));
    }
    ...
    */

    // this one library room prefab is much friendlier on the left/right sides
    // than on the decorated top wall, so for now we prefer side entrances.
    //
    // that keeps hallways from punching through the shelves and top wall art
    // until we have authored doorway markers or more room prefabs.
    int roomCenterX = room.x + room.w / 2;
    int otherCenterX = otherRoom.x + otherRoom.w / 2;
    int dx = otherCenterX - roomCenterX;

    auto isPlayablePrefabTile = [&](int localX, int localY) {
        if (!_tiledRoomPrefab || localX < 0 || localY < 0 || localX >= room.w || localY >= room.h) {
            return false;
        }
        return _tiledRoomPrefab->hasWalkableFloorAt(localX, localY);
    };

    // this finds the best row for a doorway on one side by scanning inward
    // from the room edge until we hit the first real walkable room floor tile.
    auto pickBestDoorRowFromSide = [&](bool useRightSide) {
        int boundaryLocalX = useRightSide ? (room.w - 1) : 0;
        int inwardStepX = useRightSide ? -1 : 1;
        int bestLocalY = room.h / 2;
        int bestScore = INT_MAX;

        for (int localY = 1; localY < std::max(1, room.h - 1); ++localY) {
            int firstFloorDepth = -1;
            for (int step = 0; step < room.w; ++step) {
                int localX = boundaryLocalX + step * inwardStepX;
                if (localX < 0 || localX >= room.w) {
                    break;
                }
                if (isPlayablePrefabTile(localX, localY)) {
                    firstFloorDepth = step;
                    break;
                }
            }

            if (firstFloorDepth < 0) {
                continue;
            }

            int centerBias = std::abs(localY - room.h / 2);
            int score = firstFloorDepth * 10 + centerBias;
            if (score < bestScore) {
                bestScore = score;
                bestLocalY = localY;
            }
        }

        return Vec2((float)(room.x + boundaryLocalX), (float)(room.y + bestLocalY));
    };

    // if the other room is roughly centered above/below us, we still pick one
    // of the side doors instead of cutting through the top wall.
    bool useRightSide = dx >= 0;
    if (dx == 0) {
        useRightSide = roomCenterX < (_gridWidth / 2);
    }

    return pickBestDoorRowFromSide(useRightSide);
}

Vec2 Map::getExteriorHallTileForAnchor(const GridRoom& room, const Vec2& anchorTile) const {
    // this keeps the hall itself outside the room footprint. the boundary tile
    // is still handled by the doorway carve above.
    int anchorX = (int)anchorTile.x;
    int anchorY = (int)anchorTile.y;

    if (anchorX <= room.x) {
        return Vec2((float)(anchorX - 1), (float)anchorY);
    }
    if (anchorX >= room.x + room.w - 1) {
        return Vec2((float)(anchorX + 1), (float)anchorY);
    }
    if (anchorY <= room.y) {
        return Vec2((float)anchorX, (float)(anchorY - 1));
    }
    if (anchorY >= room.y + room.h - 1) {
        return Vec2((float)anchorX, (float)(anchorY + 1));
    }

    // the anchor should always be on the boundary, but if it is not we return
    // it unchanged so the caller still gets a valid tile.
    return anchorTile;
}

void Map::carveHallTile(int x, int y, bool horizontalSegment) {
    // hall_thickness should mean the literal corridor width in tiles.
    int hallThickness = std::max(1, _config.hallThickness);
    int lowerSpread = (hallThickness - 1) / 2;
    int upperSpread = hallThickness / 2;

    for (int offset = -lowerSpread; offset <= upperSpread; ++offset) {
        int tx = x;
        int ty = y;
        if (horizontalSegment) {
            ty += offset;
        } else {
            tx += offset;
        }

        if (!inBounds(tx, ty)) {
            continue;
        }

        // room floor is already walkable, so we do not repaint it as hallway.
        // that keeps the rug/spawn area and room floor art intact inside rooms.
        TileType current = getTile(tx, ty);
        if (current == TileType::ROOM_FLOOR || current == TileType::HALL_FLOOR) {
            continue;
        }

        setTile(tx, ty, TileType::HALL_FLOOR);
        clearRenderTile(tx, ty);
    }
}

void Map::carveDoorTile(int x, int y) {
    // this is the one-tile doorway primitive. openDoorwayForHallAnchor can
    // call it more than once when the configured hallway is wider than one tile.
    if (!inBounds(x, y)) {
        return;
    }

    // if we have already reached the room floor, we leave the room art alone.
    if (getTile(x, y) == TileType::ROOM_FLOOR) {
        return;
    }

    setTile(x, y, TileType::HALL_FLOOR);
    clearRenderTile(x, y);
}

int Map::scoreHallTileDamage(int x, int y, int fromRoomIndex, int toRoomIndex) const {
    // empty space and existing halls are cheap to reuse. the big thing we want
    // to avoid is plowing through some third room that was already stamped.
    if (!inBounds(x, y)) {
        return 10000;
    }

    size_t index = (size_t)toIndex(x, y);
    int tileRoom = _tileRoomIndex[index];
    TileType tile = _tiles[index];

    if (tileRoom != -1 && tileRoom != fromRoomIndex && tileRoom != toRoomIndex) {
        return 400;
    }

    if (tile == TileType::EMPTY || tile == TileType::HALL_FLOOR) {
        return 0;
    }

    if (tile == TileType::ROOM_FLOOR) {
        // doorway cuts into the two endpoint rooms are expected, but we still
        // give them a small cost so the route chooser prefers empty space first.
        return 8;
    }

    if (tile == TileType::WALL) {
        // carving through room wall art is allowed near the connection point,
        // but it should lose against a route that stays out in the void.
        return 20;
    }

    return 60;
}

int Map::scoreHallwayRoute(const Vec2& from, const Vec2& to, bool horizontalFirst,
                           int fromRoomIndex, int toRoomIndex) const {
    // this scores the exact tiles each l-shaped hall would touch. lower means
    // "keep more of the stamped room art intact".
    int x1 = (int)from.x;
    int y1 = (int)from.y;
    int x2 = (int)to.x;
    int y2 = (int)to.y;

    int hallThickness = std::max(1, _config.hallThickness);
    int lowerSpread = (hallThickness - 1) / 2;
    int upperSpread = hallThickness / 2;
    int score = 0;
    std::unordered_set<int> visited;

    auto visitTile = [&](int x, int y, bool horizontalSegment) {
        for (int offset = -lowerSpread; offset <= upperSpread; ++offset) {
            int tx = x;
            int ty = y;
            if (horizontalSegment) {
                ty += offset;
            } else {
                tx += offset;
            }

            if (!inBounds(tx, ty)) {
                score += 10000;
                continue;
            }

            int index = toIndex(tx, ty);
            if (!visited.insert(index).second) {
                continue;
            }
            score += scoreHallTileDamage(tx, ty, fromRoomIndex, toRoomIndex);
        }
    };

    if (horizontalFirst) {
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            visitTile(xx, y1, true);
        }
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            visitTile(x2, yy, false);
        }
    } else {
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            visitTile(x1, yy, false);
        }
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            visitTile(xx, y2, true);
        }
    }

    return score;
}

void Map::openDoorwayForHallAnchor(const GridRoom& room, const GridRoom& otherRoom,
                                   const Vec2& anchorTile) {
    // the hall can reach the room edge and still leave the room blocked because
    // the sample tmj keeps full wall art around the perimeter. this helper
    // carves a tiny guaranteed doorway inward so every room stays enterable.

    int stepX = 0;
    int stepY = 0;

    // the anchor already tells us which boundary we chose, so use that to
    // decide how to step inward instead of re-guessing from room centers.
    if ((int)anchorTile.x <= room.x) {
        stepX = 1;
    } else if ((int)anchorTile.x >= room.x + room.w - 1) {
        stepX = -1;
    } else if ((int)anchorTile.y <= room.y) {
        stepY = 1;
    } else if ((int)anchorTile.y >= room.y + room.h - 1) {
        stepY = -1;
    }

    /*
    // old "walk inward until the tmj says this tile is open" logic kept here
    // for reference. with a room that has no authored edge doorway, this still
    // depended too much on where the anchor landed.
    int maxDoorDepth = std::max(2, std::max(room.w, room.h));
    for (int depth = 0; depth < maxDoorDepth; ++depth) {
        ...
    }
    */

    int hallThickness = std::max(1, _config.hallThickness);
    int lowerSpread = (hallThickness - 1) / 2;
    int upperSpread = hallThickness / 2;
    int perpX = (stepY != 0) ? 1 : 0;
    int perpY = (stepX != 0) ? 1 : 0;

    int tileX = (int)anchorTile.x;
    int tileY = (int)anchorTile.y;
    // instead of carving a fixed number of tiles, we stop as soon as we reach
    // the first real walkable room tile. that keeps entrances from chewing
    // through the middle rug/spawn area when the floor is already close by.
    int maxDoorDepth = std::max(room.w, room.h);
    for (int depth = 0; depth < maxDoorDepth; ++depth) {
        if (!inBounds(tileX, tileY)) {
            break;
        }

        // doorways should match the hall width so players do not get pinched
        // at the room edge even when the corridor is wider than one tile.
        for (int offset = -lowerSpread; offset <= upperSpread; ++offset) {
            carveDoorTile(tileX + perpX * offset, tileY + perpY * offset);
        }

        int localX = tileX - room.x;
        int localY = tileY - room.y;
        if (_tiledRoomPrefab) {
            if (_tiledRoomPrefab->hasWalkableFloorAt(localX, localY)) {
                break;
            }
        }

        tileX += stepX;
        tileY += stepY;
    }
}

void Map::pruneHallArtifacts() {
    // the older cleanup only removed single isolated tiles, which still left
    // behind tiny hall stubs that touched one room but did not actually go
    // anywhere useful. this pass looks at each exterior hall component as a
    // whole and only keeps it if it really connects at least two rooms.
    std::vector<cugl::Vec2> toClear;
    std::vector<uint8_t> visited((size_t)_tiles.size(), 0);
    static const std::array<cugl::Vec2, 4> DIRS = {
        cugl::Vec2(1, 0), cugl::Vec2(-1, 0), cugl::Vec2(0, 1), cugl::Vec2(0, -1)
    };

    for (int y = 0; y < _gridHeight; ++y) {
        for (int x = 0; x < _gridWidth; ++x) {
            if (getTile(x, y) != TileType::HALL_FLOOR) {
                continue;
            }

            int startIndex = toIndex(x, y);
            if (visited[(size_t)startIndex] != 0) {
                continue;
            }

            // room-owned floor stays alone. this cleanup is only for hall cells
            // that live outside a stamped room footprint.
            if (_tileRoomIndex[(size_t)startIndex] != -1) {
                visited[(size_t)startIndex] = 1;
                continue;
            }

            std::vector<cugl::Vec2> component;
            std::queue<cugl::Vec2> frontier;
            std::unordered_set<int> connectedRooms;

            frontier.push(cugl::Vec2((float)x, (float)y));
            visited[(size_t)startIndex] = 1;

            while (!frontier.empty()) {
                cugl::Vec2 current = frontier.front();
                frontier.pop();
                component.push_back(current);

                for (const cugl::Vec2& dir : DIRS) {
                    int nx = (int)current.x + (int)dir.x;
                    int ny = (int)current.y + (int)dir.y;
                    if (!inBounds(nx, ny)) {
                        continue;
                    }

                    int nextIndex = toIndex(nx, ny);
                    TileType neighborTile = getTile(nx, ny);

                    if (neighborTile == TileType::HALL_FLOOR && _tileRoomIndex[(size_t)nextIndex] == -1) {
                        if (visited[(size_t)nextIndex] == 0) {
                            visited[(size_t)nextIndex] = 1;
                            frontier.push(cugl::Vec2((float)nx, (float)ny));
                        }
                        continue;
                    }

                    int roomIndex = _tileRoomIndex[(size_t)nextIndex];
                    if (roomIndex != -1 && neighborTile == TileType::ROOM_FLOOR) {
                        connectedRooms.insert(roomIndex);
                    }
                }
            }

            // a useful hallway component should actually connect at least two
            // different rooms. if it only touches one room, it is just a stub.
            if (connectedRooms.size() < 2) {
                toClear.insert(toClear.end(), component.begin(), component.end());
            }
        }
    }

    for (const cugl::Vec2& tile : toClear) {
        setTile((int)tile.x, (int)tile.y, TileType::EMPTY, -1);
        clearRenderTile((int)tile.x, (int)tile.y);
    }
}

void Map::populateObtacles(std::shared_ptr<NetWorld>& world) {
//    if (!batch || _tiles.empty()) {
//        return;
//    }

    // map rendering changes batch state a lot, so we save/restore the basics
    // here so later world draws like players and enemies do not inherit the
    // backdrop tint by accident.
//    Color4 originalColor = batch->getColor();
//    batch->setTexture(nullptr);
//
//    Rect backdrop(_worldBounds.origin.x, _worldBounds.origin.y,
//                  _worldBounds.size.width, _worldBounds.size.height);
//    batch->setColor(_config.backdropColor);
//    batch->fill(backdrop);

    if (_tiledRoomPrefab && _tiledRoomPrefab->getTileset().texture) {
        // this is the new tiled-backed draw path. repeated room tiles come from
        // the real tileset art, while hallways still use a borrowed floor tile.
        for (int yy = 0; yy < _gridHeight; ++yy) { // uh why height then width wtf
            for (int xx = 0; xx < _gridWidth; ++xx) {
                TileType tile = _tileGrid.at(xx).at(yy);
                if (tile == TileType::EMPTY) {
                    continue;
                }

                float px = (_origin.x + xx * _tileSize) / PHYSICS_SCALE;
                float py = (_origin.y + yy * _tileSize) / PHYSICS_SCALE;
                Rect cell(px, py, _tileSize, _tileSize);
                size_t index = (size_t)toIndex(xx, yy);

                if (tile == TileType::WALL) {
                    // halls should still read as walkable tiled space, not a
                    // debug block, so we borrow a floor tile from the prefab.
                    // FLAG: serena did, unsure if we need all this error checking...
//                    std::shared_ptr<Texture> hallTexture = _tiledRoomPrefab->getTextureForGid(_hallFloorGid);
//                    if (hallTexture) {
                    std::shared_ptr<RectObstacle> r = RectObstacle::alloc(cugl::Vec2(px, py), _tileSize, _tileSize);
                    world -> addObstacle(r);
                    //CULog("creating an obstacle");
                        //batch->draw(hallTexture, Color4::WHITE, cell);
//                    } else {
//                        batch->setTexture(nullptr);
//                        batch->setColor(_config.hallFloorColor);
//                        batch->fill(cell);
//                    }
                    continue;
                }
                

//                auto drawLayerGid = [&](unsigned int rawGid) {
////                       std::shared_ptr<Texture> tileTexture = _tiledRoomPrefab->getTextureForGid(rawGid);
////                    if (!tileTexture) {
////                        return;
////                    }
//
//                    bool flipHorizontal = hasTiledHorizontalFlip(rawGid);
//                    bool flipVertical = hasTiledVerticalFlip(rawGid);
//                    bool flipDiagonal = hasTiledDiagonalFlip(rawGid);
//
//                    if (!flipHorizontal && !flipVertical && !flipDiagonal) {
//                        //batch->draw(tileTexture, Color4::WHITE, cell);
//                        auto params = rectFact->serializeParams(cugl::Vec2(px, py), _tileSize, _tileSize, rawGid);
//                        auto pair = network->getPhysController()->addSharedObstacle(rectFactId, params);
//                        return;
//                    }
//
//                    // this keeps authored tiled flips working without rotating
//                    // the whole room prefab in code.
////                    Vec2 tileOrigin(_tileSize * 0.5f, _tileSize * 0.5f);
////                    TileTransform2D tileTransform = makeTileTransform(flipHorizontal, flipVertical,
////                                                                     flipDiagonal);
////                    Affine2 transform(tileTransform.m11, tileTransform.m12,
////                                      tileTransform.m21, tileTransform.m22,
////                                      0.0f, 0.0f);
////                    batch->draw(tileTexture, Color4::WHITE, cell, tileOrigin, transform);
//                    auto params = rectFact->serializeParams(cugl::Vec2(px, py), _tileSize  , _tileSize , rawGid);
//                    auto pair = network->getPhysController()->addSharedObstacle(rectFactId, params);
//                };
//
//                // this order matches the layer order in example_2.tmj.
//                //drawLayerGid(_floorRenderGids[index]);
//                //drawLayerGid(_visFixRenderGids[index]);
//                drawLayerGid(_wallRenderGids[index]);
//                //drawLayerGid(_decoRenderGids[index]);
//
//                // no debug fill here on purpose. if a tiled layer is missing,
//                // we would rather notice the bad room data than silently hide it.
            }
        }
        //batch->setColor(originalColor);
        return;
    }
}
std::shared_ptr<cugl::scene2::SceneNode> Map::draw2(){
    auto world = scene2::SceneNode::alloc();
    //    Rect backdrop(_worldBounds.origin.x, _worldBounds.origin.y,
    //                  _worldBounds.size.width, _worldBounds.size.height);
    //    batch->setColor(_config.backdropColor);
    //    batch->fill(backdrop);

    if (_tiledRoomPrefab && _tiledRoomPrefab->getTileset().texture) {
        // this is the new tiled-backed draw path. repeated room tiles come from
        // the real tileset art, while hallways still use a borrowed floor tile.
        for (int yy = 0; yy < _gridHeight; ++yy) {
            for (int xx = 0; xx < _gridWidth; ++xx) {
                TileType tile = _tileGrid.at(xx).at(yy);
                if (tile == TileType::EMPTY) {
                    continue;
                }
                
                float px = (_origin.x + xx * _tileSize);
                float py = (_origin.y + yy * _tileSize);
                Rect cell(0, 0, _tileSize, _tileSize);
                size_t index = (size_t)toIndex(xx, yy);
                std::shared_ptr<Texture> tex = _tiledRoomPrefab->getTextureForGid(_hallFloorGid); //default for now
                tex = _tiledRoomPrefab -> getTextureForGid(_floorRenderGids[index]);
                if(!tex){
                    continue;
                }
                auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                sprite->initWithTexturePoly(tex, cell);
                sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                sprite->setPosition(Vec2(px, py));
                world -> addChild(sprite);
                }
            }
        }
       
    return world;
    }
    
    


std::shared_ptr<cugl::scene2::SceneNode> Map::draw() {
    auto world = scene2::SceneNode::alloc();
    //    Rect backdrop(_worldBounds.origin.x, _worldBounds.origin.y,
    //                  _worldBounds.size.width, _worldBounds.size.height);
    //    batch->setColor(_config.backdropColor);
    //    batch->fill(backdrop);

    if (_tiledRoomPrefab && _tiledRoomPrefab->getTileset().texture) {
        // this is the new tiled-backed draw path. repeated room tiles come from
        // the real tileset art, while hallways still use a borrowed floor tile.
        std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> floors;
        std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> decor;
        for (int yy = 0; yy < _gridHeight; ++yy) {
            for (int xx = 0; xx < _gridWidth; ++xx) {
                TileType tile = _tileGrid.at(xx).at(yy);
                if (tile == TileType::EMPTY) {
                    continue;
                }
                
                float px = (_origin.x + xx * _tileSize);
                float py = (_origin.y + yy * _tileSize);
                Rect cell(0, 0, _tileSize, _tileSize);
                size_t index = (size_t)toIndex(xx, yy);
                std::shared_ptr<Texture> tex = _tiledRoomPrefab->getTextureForGid(_hallFloorGid); //default for now
                if (tile == TileType::HALL_FLOOR) {
                    tex = _tiledRoomPrefab->getTextureForGid(_hallFloorGid);
                    auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                    sprite->initWithTexturePoly(tex, cell);
                    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                    sprite->setPosition(Vec2(px, py));
                    floors.push_back(sprite);
                    continue;
                }
                else if (tile==TileType::ROOM_FLOOR) {
                    tex = _tiledRoomPrefab->getTextureForGid(_floorRenderGids[index]);
                    auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                    sprite->initWithTexturePoly(tex, cell);
                    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                    sprite->setPosition(Vec2(px, py));
                    floors.push_back(sprite);
                }else{
                    tex = _tiledRoomPrefab -> getTextureForGid(_decoRenderGids[index]);
                    if(!tex){
                        tex = _tiledRoomPrefab -> getTextureForGid(_visFixRenderGids[index]);
                        if(!tex){
                            //tex = _tiledRoomPrefab -> getTextureForGid(_floorRenderGids[index]);
                            if (!tex){
                                tex = _tiledRoomPrefab -> getTextureForGid(_wallRenderGids[index]);
                                if(!tex){
                                    continue;
                                }
                            }
                        }
                    }
                    auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                    sprite->initWithTexturePoly(tex, cell);
                    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                    sprite->setPosition(Vec2(px, py));
                    decor.push_back(sprite);
                }
                
                
            }
        }
        for (const auto& node : decor) {
            if (node) { world->addChild(node); }
        }
        for (const auto& node : floors) {
            if (node) { world->addChild(node); }
        }
        
    }
    
    return world;
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
    int closestReached = startIndex;

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

        if (manhattanToGoal(current) < manhattanToGoal(closestReached)) {
            closestReached = current;
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
        reached = closestReached;
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
    return Vec2((_origin.x + (x + 0.5f) * _tileSize),
                (_origin.y + (y + 0.5f) * _tileSize));
}

bool Map::overlapsExistingRoom(const GridRoom& candidate, const std::vector<GridRoom>& rooms) const {
    if (rooms.size()==0) return false;
    // the tiled hallway path needs at least one tile of breathing room so the
    // outside hall tile can exist without landing inside another room.
    int roomBuffer = std::max(1, _config.roomBufferTiles);
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

/*
// old random-room helpers kept here as reference from the pre-tiled map path.
void Map::carveRoom(const GridRoom& room, int roomIndex) {
    for (int yy = room.y; yy < room.y + room.h; ++yy) {
        for (int xx = room.x; xx < room.x + room.w; ++xx) {
            setTile(xx, yy, TileType::ROOM_FLOOR, roomIndex);
        }
    }
}
*/

void Map::carveHallway(const Vec2& from, const Vec2& to, int fromRoomIndex, int toRoomIndex) {
    int x1 = (int)from.x;
    int y1 = (int)from.y;
    int x2 = (int)to.x;
    int y2 = (int)to.y;

    int horizontalScore = scoreHallwayRoute(from, to, true, fromRoomIndex, toRoomIndex);
    int verticalScore = scoreHallwayRoute(from, to, false, fromRoomIndex, toRoomIndex);
    bool horizontalFirst = horizontalScore <= verticalScore;
    if (horizontalFirst) {
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            carveHallTile(xx, y1, true);
        }
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            carveHallTile(x2, yy, false);
        }
    } else {
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int yy = startY; yy <= endY; ++yy) {
            carveHallTile(x1, yy, false);
        }
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int xx = startX; xx <= endX; ++xx) {
            carveHallTile(xx, y2, true);
        }
    }
}

/*
// old obstacle/spawn helpers kept here as reference from the pre-tiled map path.
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

    Vec2 centerPick = tileToWorldCenter(room.x + room.w / 2 + _config.primaryItemOffsetX,
                                        room.y + room.h / 2 + _config.primaryItemOffsetY);
    centerPick = findNearestWalkableWorld(centerPick);

    if (avoid && (centerPick - *avoid).lengthSquared() < minSpacing2) {
        Vec2 nudged = tileToWorldCenter(room.x + room.w / 2 + _config.secondaryItemOffsetX,
                                        room.y + room.h / 2 + _config.secondaryItemOffsetY);
        centerPick = findNearestWalkableWorld(nudged);
    }

    return centerPick;
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
*/

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


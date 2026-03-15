// this model generates and stores the shared room/hall map for one run

#ifndef __HEXCAPADE_MAP_H__
#define __HEXCAPADE_MAP_H__

#include <cugl/cugl.h>
#include <random>
#include <vector>

#include "GameplayConfig.h"
#include "Room.h"

class Map {
public:
    enum class TileType : uint8_t {
        EMPTY,
        ROOM_FLOOR,
        HALL_FLOOR,
        WALL,
        OBSTACLE
    };

private:
    struct GridRoom {
        int x;
        int y;
        int w;
        int h;
        int templateIndex;
    };

    int _gridWidth;
    int _gridHeight;
    float _tileSize;
    cugl::Vec2 _origin;
    cugl::Rect _worldBounds;

    std::vector<TileType> _tiles;
    std::vector<int> _tileRoomIndex;

    std::vector<Room> _rooms;
    std::vector<cugl::Vec2> _itemSpawnPoints;
    std::vector<cugl::Vec2> _enemySpawnPoints;
    cugl::Vec2 _playerSpawn;
    MapGenerationConfig _config;

    mutable std::mt19937 _rng;

public:
    Map();

    bool generate(const cugl::Size& viewSize, const MapGenerationConfig& config, unsigned int seed = 0);
    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) const;

    bool canOccupy(const cugl::Vec2& worldPos, float radius) const;
    cugl::Vec2 resolveSlideMove(const cugl::Vec2& startPos, const cugl::Vec2& velocity, float radius) const;
    cugl::Vec2 getNextWaypoint(const cugl::Vec2& fromWorld, const cugl::Vec2& toWorld) const;
    cugl::Vec2 findNearestWalkableWorld(const cugl::Vec2& fromWorld) const;

    int findRoomIndexForWorld(const cugl::Vec2& worldPos) const;
    const std::vector<int>& getConnectedRooms(int roomIndex) const;

    const cugl::Rect& getWorldBounds() const { return _worldBounds; }
    const std::vector<Room>& getRooms() const { return _rooms; }
    const std::vector<cugl::Vec2>& getItemSpawnPoints() const { return _itemSpawnPoints; }
    const std::vector<cugl::Vec2>& getEnemySpawnPoints() const { return _enemySpawnPoints; }
    cugl::Vec2 getPlayerSpawn() const { return _playerSpawn; }

private:
    bool inBounds(int x, int y) const;
    int toIndex(int x, int y) const;
    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType tile, int roomIndex = -1);
    bool isWalkableTile(int x, int y) const;
    bool worldToTile(const cugl::Vec2& worldPos, int& outX, int& outY) const;
    cugl::Vec2 tileToWorldCenter(int x, int y) const;

    bool overlapsExistingRoom(const GridRoom& candidate, const std::vector<GridRoom>& rooms) const;
    void carveRoom(const GridRoom& room, int roomIndex);
    void carveHallway(const cugl::Vec2& from, const cugl::Vec2& to);
    void placeObstaclesInRoom(const GridRoom& room);
    void buildWalls();
    void seedRoomSpawns(const std::vector<GridRoom>& rooms);
    cugl::Vec2 pickRandomWalkableInRoom(const GridRoom& room, const cugl::Vec2* avoid = nullptr);

    cugl::Color4 getFloorColorForTile(int x, int y) const;
    int findNearestWalkableTileIndex(int startX, int startY) const;
};

#endif // __HEXCAPADE_MAP_H__

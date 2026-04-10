
// this model generates and stores the shared room/hall map for one run

#ifndef __HEXCAPADE_MAP_H__
#define __HEXCAPADE_MAP_H__

#include <cugl/cugl.h>
#include <memory>
#include <vector>

#include "../GameplayConfig.h"
#include "Room.h"
#include "TiledRoomLoader.h"
#include "../../Controllers/NetworkController.h"
#include "RectFactory.h"

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
    std::vector<std::vector<TileType>> _tileGrid;
    std::vector<TileType> _tiles;
    std::vector<int> _tileRoomIndex;
    std::vector<unsigned int> _floorRenderGids;
    std::vector<unsigned int> _wallRenderGids;
    std::vector<unsigned int> _decoRenderGids;
    std::vector<unsigned int> _visFixRenderGids;
    unsigned int _hallFloorGid;

    std::vector<Room> _rooms;

    std::vector<cugl::Vec2> _itemSpawnPoints;
    std::vector<cugl::Vec2> _enemySpawnPoints;
    cugl::Vec2 _playerSpawn;
    MapGenerationConfig _config;

    cugl::Random _rng;

public:
    Map();
    std::shared_ptr<TiledRoomData> _tiledRoomPrefab;

    void init();


    bool generate(const MapGenerationConfig& config, unsigned int seed);
    void populateObtacles(std::shared_ptr<NetWorld>& world);
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
    std::shared_ptr<cugl::scene2::SceneNode> draw();
    std::shared_ptr<cugl::scene2::SceneNode> draw2();
private:
    bool inBounds(int x, int y) const;
    int toIndex(int x, int y) const;
    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType tile, int roomIndex = -1);
    bool isWalkableTile(int x, int y) const;
    bool worldToTile(const cugl::Vec2& worldPos, int& outX, int& outY) const;
    cugl::Vec2 tileToWorldCenter(int x, int y) const;

    bool overlapsExistingRoom(const GridRoom& candidate, const std::vector<GridRoom>& rooms) const;

    // this clears the per-tile gid layers used to render repeated tiled rooms
    void clearRenderLayers();
    // this writes one gid into one map-wide tiled render layer.
    void setRenderGid(std::vector<unsigned int>& layer, int x, int y, unsigned int gid);
    // this clears all stamped room art at a map tile; hall carving uses this
    // when it needs to cut through a room wall (not used recent)
    void clearRenderTile(int x, int y);
    // this copies one tiled room prefab into the map grid and stamps its art
    void stampTiledRoom(const GridRoom& room, int roomIndex);
    // this converts a local room-space marker from tiled into world space for one room instance
    cugl::Vec2 roomLocalToWorld(const GridRoom& room, const cugl::Vec2& localPos) const;
    // this seeds player/item/enemy spawns from the tiled markers in each repeated room
    void seedTiledRoomSpawns(const std::vector<GridRoom>& rooms);
    // this picks the tile on a room edge where a hallway should connect.
    cugl::Vec2 pickHallAnchorTile(const GridRoom& room, const GridRoom& otherRoom) const;
    // this converts a room-edge doorway tile into the first hallway tile just
    // outside the room so the hall does not start on top of room art.
    cugl::Vec2 getExteriorHallTileForAnchor(const GridRoom& room,
        const cugl::Vec2& anchorTile) const;
    // this carves a one-tile doorway so hall entrances do not shave off extra decor.
    void carveDoorTile(int x, int y);
    // this carves one hallway step using the current hall width rules.
    void carveHallTile(int x, int y, bool horizontalSegment);
    // this scores how destructive one hallway tile would be before we commit
    int scoreHallTileDamage(int x, int y, int fromRoomIndex, int toRoomIndex) const;
    // this compares the two possible l-shaped hallway routes so we can keep
    // the room art intact as much as possible
    int scoreHallwayRoute(const cugl::Vec2& from, const cugl::Vec2& to,
        bool horizontalFirst, int fromRoomIndex, int toRoomIndex) const;
    // this opens a short doorway from the hall anchor into the room interior;
    // it keeps every repeated room enterable even if the prefab edge is walled
    void openDoorwayForHallAnchor(const GridRoom& room, const GridRoom& otherRoom,
        const cugl::Vec2& anchorTile);

    // this removes stray one-tile hall tabs that can appear next to a room
    // without actually connecting to the hallway network (doublecheck)
    void pruneHallArtifacts();

    void carveHallway(const cugl::Vec2& from, const cugl::Vec2& to,
        int fromRoomIndex = -1, int toRoomIndex = -1);

    /*
    // old random-room helpers kept here as reference from the pre-tiled map path.
    void carveRoom(const GridRoom& room, int roomIndex);
    void placeObstaclesInRoom(const GridRoom& room);
    void buildWalls();
    void seedRoomSpawns(const std::vector<GridRoom>& rooms);
    cugl::Vec2 pickRandomWalkableInRoom(const GridRoom& room, const cugl::Vec2* avoid = nullptr);
    cugl::Color4 getFloorColorForTile(int x, int y) const;
    */
    int findNearestWalkableTileIndex(int startX, int startY) const;
};

#endif // __HEXCAPADE_MAP_H__

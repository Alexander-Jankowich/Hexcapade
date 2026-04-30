// this file generates and renders a full run layout made of rooms, halls,
// walls, and room-based spawn points

#include "Map.h"

#include <array>
#include <cmath>
#include <unordered_set>




Map::Map() :
_origin(Vec2::ZERO),
_playerSpawn(Vec2::ZERO),
_config() {
    _config._mapSize = 70; // just in case
}

void Map::reset() {
    _enemySpawnPoints.clear();
    _z1enemySpawnPoints.clear();
    _z2enemySpawnPoints.clear();
    _z3enemySpawnPoints.clear();
    _itemSpawnPoints.clear();
    lw_node->removeAllChildren();
}

std::shared_ptr<cugl::scene2::SceneNode> Map::generate(int level, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms, const Size& sceneSize, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world) {
    _assets = assets;
    loadLevelConfig(level, _assets->get<JsonValue>("level-config"));
    _extruder = SimpleExtruder();
    // ensure everything cleared before generation
    _mapGrid.clear();
    for (int r = 0; r < _config._mapSize; r++) {
        vector<int> v;
        v.assign(_config._mapSize, 0);
        _mapGrid.push_back(v);
    }
   
    
    _enemySpawnPoints.clear();
    _z1enemySpawnPoints.clear();
    _z2enemySpawnPoints.clear();
    _z3enemySpawnPoints.clear();
    _itemSpawnPoints.clear();
    
    _sceneSize = sceneSize;
    auto node = scene2::SceneNode::alloc();
    lw_node = scene2::SceneNode::alloc();
    std::vector<MapDoor> doors;
    _rng.initWithSeed(seed);
    
    
    // initialize level config
    

    
    //get minimum room width/height
    int minRoomW = INT_MAX;
    int minRoomH = INT_MAX;
    for (auto r : *rooms){
        minRoomW = _assets->get<LevelLoader>(r)->getWidth() < minRoomW ? assets->get<LevelLoader>(r)->getWidth() : minRoomW;
        minRoomH = _assets->get<LevelLoader>(r)->getHeight() < minRoomH ? assets->get<LevelLoader>(r)->getHeight() : minRoomH;
    }
    
    // calc lists of room w each door type:
    std::vector<std::string> hasNorthDoors;
    std::copy_if(rooms->begin(), rooms->end(), std::back_inserter(hasNorthDoors), [this](std::string r){return _assets->get<LevelLoader>(r)->hasNorthDoor();});
    
    std::vector<std::string> hasSouthDoors;
    std::copy_if(rooms->begin(), rooms->end(), std::back_inserter(hasSouthDoors), [this](std::string r){return _assets->get<LevelLoader>(r)->hasSouthDoor();});
    
    std::vector<std::string> hasEastDoors;
    std::copy_if(rooms->begin(), rooms->end(), std::back_inserter(hasEastDoors), [this](std::string r){return _assets->get<LevelLoader>(r)->hasEastDoor();});
    
    std::vector<std::string> hasWestDoors;
    std::copy_if(rooms->begin(), rooms->end(), std::back_inserter(hasWestDoors), [this](std::string r){return _assets->get<LevelLoader>(r)->hasWestDoor();});
    
    // select first room and add to center of the map
    int rm_idx = (int) _rng.getOpenSint64(0, rooms->size());
    std::string& room = rooms->at(rm_idx);
    auto tiledData = _assets->get<LevelLoader>(room);
    
    // room top left corner ON THE MAP GRID at (width/2 - roomW /2, height/2 - roomH/2) since up is 0
    auto tlcorner = Vec2(_config._mapSize/2 - tiledData->getWidth()/2, _config._mapSize/2 - tiledData->getHeight()/2);
    CULog("tlcorner first room is x: %f y: %f", tlcorner.x, tlcorner.y);
    
    addRoomSceneNodes(tiledData, tlcorner, node, lw_node);
    addRoomObstacles(tiledData, tlcorner, world);
    
    // get playerSpawn: (mapwidth/2 - roomw /2 + playerSpawn.x, mapheight/2 - roomH/2 + (roomH - playerspawn.y)
    // Corrected player spawn calculation
    auto pl_mg = cugl::Vec2(_config._mapSize/2 - tiledData->getWidth() / 2 + tiledData->getPlayerSpawn().x,
                            _config._mapSize / 2 - tiledData->getHeight() / 2 + tiledData->getPlayerSpawn().y);
    
    // Just map grid units to world pixels.
    // We use (MAP_HEIGHT - pl_mg.y) to flip from Tiled/Grid (top-left) to CUGL (bottom-left).
    _playerSpawn = Vec2(pl_mg.x * TILE_SIZE, (_config._mapSize - pl_mg.y) * TILE_SIZE);
    // add all doors in room to doors queue
    for (auto d : tiledData->getDoors()) {
        // pos is the TOP LEFT corner of door
        auto md = MapDoor(Vec2(tlcorner.x + d->_tlCorner.x, tlcorner.y + d->_tlCorner.y), d->_dimens / tiledData->getTileH(), d->_direction);
        doors.push_back(md);
    }
    _rooms.push_back(std::make_shared<Room>(tlcorner, Room::RoomModifier::NONE, TILE_SIZE * tiledData->getWidth(), TILE_SIZE * tiledData->getHeight()));
    
    CULog("added first room");
    //iterate over doors queue
    while (!doors.empty()) {
        CULog("doors size %d", (int) doors.size());
        // select a random door - ty bloomberg for the O(1) method
        int door_idx = (int) _rng.getOpenSint64(0, doors.size());
        auto door = doors.at(door_idx);
        doors[door_idx] = doors.at(doors.size()-1);
        doors.pop_back();
        
        // declare up here, default vals
        std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> res = std::make_tuple(nullptr, -1, Vec2(0, 0));
        // if add room, stores connected door so we know to NOT add to doors set. default does not matter
        Door::Direction to_skip = Door::Direction::NORTH;

        int xPos = door.pos.x;
        int yPos = door.pos.y;
        
        switch (door.dir) {
            case Door::Direction::NORTH:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasSouthDoors, Door::Direction::SOUTH);
                if (std::get<0>(res) == nullptr){
                    // wall off this room gids (10/23/36) x5 across doorway
                    CULog("walling off - N");
                    for (int i = xPos; i < xPos + 5; i++) {
                        // wall 1
                        addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(i, _config._mapSize - yPos + 1), node);
                        if (yPos-1 >= 0) _mapGrid[yPos-1][i] = 1;
                        // wall 2
                        addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(i, _config._mapSize - yPos), node);
                        if (yPos >= 0)_mapGrid[yPos][i] = 1;
                        // wall 3
                        addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(i, _config._mapSize - yPos - 1), node);
                        if (yPos + 1 >= 0)_mapGrid[yPos+1][i] = 1;
                        
                    }
                    CULog("done walling off - N");
                    std::vector<Vec2> north_points = {
                        Vec2((xPos - 2) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos) * TILE_SIZE / PHYSICS_SCALE),
                        Vec2((xPos + 8) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos) * TILE_SIZE / PHYSICS_SCALE)
                    };
                    _extruder.set(north_points,false);
                    _extruder.calculate(0.05f);
                    auto np = _extruder.getPolygon();
                    addWallObstacle(np, world);
                    continue;
                }
                CULog("adding halls - N");
                for (int y= yPos; y >= yPos - std::get<1>(res); y--) {
                    // add west wall
                    addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(xPos - 1, _config._mapSize - y), node);
                    _mapGrid[y][xPos -1] = 1;
                    // add 5 floor tiles
                    for (int x= xPos; x < xPos +5; x++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add east wall
                    addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(xPos +5, _config._mapSize - y), node);
                    _mapGrid[y][xPos +5] = 1;
                    
                }
                int height = std::get<1>(res)+2; // bc <=
                std::vector<Vec2> west_points = {
                    Vec2((xPos - 0.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 4) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos - 0.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - height + 3) * TILE_SIZE / PHYSICS_SCALE)
                };
                std::vector<Vec2> east_points = {
                    Vec2((xPos + 4.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 4) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos + 4.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - height + 3) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(west_points,false);
                _extruder.calculate(0.05f);
                auto wp = _extruder.getPolygon();
                addWallObstacle(wp, world);
                _extruder.set(east_points,false);
                _extruder.calculate(0.05f);
                auto ep = _extruder.getPolygon();
                addWallObstacle(ep, world);                

                to_skip = Door::Direction::SOUTH;
                CULog("done adding halls - N");
                break;
            }
            case Door::Direction::SOUTH:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasNorthDoors, Door::Direction::NORTH);
                if (std::get<0>(res) == nullptr){
                    // wall off this room x5 across doorway
                    CULog("walling off - S");
                    for (int i = xPos; i < xPos + 5; i++) {
                        // wall 1
                        addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(i, _config._mapSize - yPos + 2), lw_node);
                        _mapGrid[yPos-2][i] = 1;
                        // wall 2
                        addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(i, _config._mapSize - yPos + 1), lw_node);
                        _mapGrid[yPos-1][i] = 1;
                        // wall 3
                        addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(i, _config._mapSize - yPos), lw_node);
                        _mapGrid[yPos][i] = 1;
                    }
                    CULog("done walling off - S");
                    
                    std::vector<Vec2> north_points = {
                        Vec2((xPos - 2) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 1) * TILE_SIZE / PHYSICS_SCALE),
                        Vec2((xPos + 8) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 1) * TILE_SIZE / PHYSICS_SCALE)
                    };
                    _extruder.set(north_points,false);
                    _extruder.calculate(0.05f);
                    auto np = _extruder.getPolygon();
                    addWallObstacle(np, world);
                    continue;
                }
                CULog("adding halls - S");
                
                for (int y=yPos; y < yPos + std::get<1>(res); y++) {
                    // add west wall
                    addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(xPos - 1, _config._mapSize - y), node);
                    _mapGrid[y][xPos - 1] = 1;
                    // add 5 floor tiles
                    for (int x=xPos; x < xPos +5; x++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add east wall
                    addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(xPos +5, _config._mapSize - y), node);
                    _mapGrid[y][xPos+5] = 1;
                    
                }
                to_skip = Door::Direction::NORTH;
                CULog("done adding halls - S");
                // hallway colliders
                int height = std::get<1>(res)+1; // bc <=
                std::vector<Vec2> west_points = {
                    Vec2((xPos - 0.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 1) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos - 0.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - height) * TILE_SIZE / PHYSICS_SCALE)
                };
                std::vector<Vec2> east_points = {
                    Vec2((xPos + 4.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 1) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos + 4.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - height) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(west_points,false);
                _extruder.calculate(0.05f);
                auto wp = _extruder.getPolygon();
                addWallObstacle(wp, world);
                _extruder.set(east_points,false);
                _extruder.calculate(0.05f);
                auto ep = _extruder.getPolygon();
                addWallObstacle(ep, world);
//                addWallObstacle(ww, world);
//                addWallObstacle(ew, world);
                break;
            }
            case Door::Direction::EAST:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasWestDoors, Door::Direction::WEST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    CULog("walling off - E");
                    
                    for (int i = yPos - 2; i < yPos + 4; i++) {
                        addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(xPos+1, _config._mapSize - i), node);
                        if(xPos < _config._mapSize - 1) _mapGrid[i][xPos + 1] = 1;
                    }
                    // add collider
                    std::vector<Vec2> north_points = {
                        Vec2((xPos + 0.2) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 2 ) * TILE_SIZE / PHYSICS_SCALE),
                        Vec2((xPos + 0.2) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 6) * TILE_SIZE / PHYSICS_SCALE)
                    };
                    _extruder.set(north_points,false);
                    _extruder.calculate(0.05f);
                    auto np = _extruder.getPolygon();
                    addWallObstacle(np, world);
                    //addWallObstacle(ew, world);
                    continue;
                }
                CULog("adding halls - E");
                
                for (int x=xPos; x <= xPos + std::get<1>(res); x++) {
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, _config._mapSize - yPos + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, _config._mapSize - yPos + 2), node);
                    addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - yPos + 1), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, _config._mapSize - yPos + 1), node);

                    _mapGrid[yPos-3][x] = 1;
                    _mapGrid[yPos-2][x] = 1;
                    _mapGrid[yPos-1][x] = 1;
                    
                    // add 5 floor tiles
                    for (int y=yPos; y < yPos +5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, _config._mapSize - yPos - 3), lw_node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, _config._mapSize - yPos - 4), lw_node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, _config._mapSize - yPos - 5), lw_node);
                    _mapGrid[yPos+5][x] = 1;
                    _mapGrid[yPos+4][x] = 1;
                    _mapGrid[yPos+3][x] = 1;
                }
                to_skip = Door::Direction::WEST;
                CULog("done adding halls - E");
                std::vector<Vec2> north_points = {
                    Vec2((xPos + 0.5) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 1.2) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos + std::get<1>(res) + 1) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 1.2) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(north_points,false);
                _extruder.calculate(0.05f);
                auto np = _extruder.getPolygon();
                addWallObstacle(np, world);
                std::vector<Vec2> south_points = {
                    Vec2((xPos ) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 5.5) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos + 3) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 5.5) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(south_points,false);
                _extruder.calculate(0.05f);
                auto sp = _extruder.getPolygon();
                addWallObstacle(sp, world);
                break;
            }
            case Door::Direction::WEST:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasEastDoors, Door::Direction::EAST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    CULog("walling off - W");
                    
                    for (int i = yPos - 2; i < yPos + 4; i++) {
                        addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(xPos-1, _config._mapSize - i), node);
                        if(xPos > 0) _mapGrid[i][xPos -1] = 1;
                    }
                    std::vector<Vec2> north_points = {
                        Vec2((xPos) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 2 ) * TILE_SIZE / PHYSICS_SCALE),
                        Vec2((xPos) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 8) * TILE_SIZE / PHYSICS_SCALE)
                    };
                    _extruder.set(north_points,false);
                    _extruder.calculate(0.05f);
                    auto np = _extruder.getPolygon();
                    addWallObstacle(np, world);
                    //addWallObstacle(ww, world);
                    CULog("done walling off - W");
                    continue;
                }
                CULog("adding halls - W");
                for (int x=xPos; x >= xPos - std::get<1>(res); x--) {
                    CULog("adding hall at %d", x);
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, _config._mapSize - yPos + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, _config._mapSize - yPos + 2), node);
                    addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - yPos + 1), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, _config._mapSize - yPos + 1), node);
                    _mapGrid[yPos-3][x] = 1;
                    _mapGrid[yPos-2][x] = 1;
                    _mapGrid[yPos-1][x] = 1;
                    // add 7 floor tiles (vis fix stuff)
                    for (int y=yPos; y < yPos + 5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, _config._mapSize - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, _config._mapSize - yPos - 3), lw_node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, _config._mapSize - yPos - 4), lw_node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, _config._mapSize - yPos - 5), lw_node);
                    _mapGrid[yPos+3][x] = 1;
                    _mapGrid[yPos+4][x] = 1;
                    _mapGrid[yPos+5][x] = 1;
                }
                to_skip = Door::Direction::EAST;
                CULog("done adding halls - W");
                std::vector<Vec2> north_points = {
                    Vec2((xPos + 1) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 1.2) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos - std::get<1>(res) - 3) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos + 1.2) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(north_points,false);
                _extruder.calculate(0.05f);
                auto np = _extruder.getPolygon();
                addWallObstacle(np, world);
                std::vector<Vec2> south_points = {
                    Vec2((xPos + 3) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 5.5) * TILE_SIZE / PHYSICS_SCALE),
                    Vec2((xPos - std::get<1>(res)) * TILE_SIZE / PHYSICS_SCALE, (_config._mapSize - yPos - 5.5) * TILE_SIZE / PHYSICS_SCALE)
                };
                _extruder.set(south_points,false);
                _extruder.calculate(0.05f);
                auto sp = _extruder.getPolygon();
                addWallObstacle(sp, world);
                
                break;
            }
        }
        // for all, if here then res successfully filled in with room data. add room + doors + incr counter (later add room obj)
        CULog("adding room and doors");
        addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node, lw_node);
        for (auto d : std::get<0>(res)->getDoors()) {
            // pos is the TOP LEFT corner of door
            if (d->_direction!=to_skip){
                auto md = MapDoor(Vec2(std::get<2>(res).x + d->_tlCorner.x, std::get<2>(res).y + d->_tlCorner.y), d->_dimens / std::get<0>(res)->getTileH(), d->_direction);
                doors.push_back(md);
            }
        }
        // add item spawn pts
        for (auto ip : std::get<0>(res)->getItemSpawnPoints()) {
            // convert coords
            auto ip_mg = cugl::Vec2(std::get<2>(res).x + ip.x,
                                    std::get<2>(res).y + ip.y);
            // Just map grid units to world pixels.
            // We use (MAP_HEIGHT - pl_mg.y) to flip from Tiled/Grid (top-left) to CUGL (bottom-left).
            _itemSpawnPoints.push_back(Vec2(ip_mg.x * TILE_SIZE, (_config._mapSize - ip_mg.y) * TILE_SIZE));
        }
        // add enemy spawns
        for (auto ep : std::get<0>(res)->getEnemSpawnPoints()) {
            // convert coords
            auto ep_mg = cugl::Vec2(std::get<2>(res).x + ep.x,
                                    std::get<2>(res).y + ep.y);
            // Just map grid units to world pixels.
            // We use (MAP_HEIGHT - pl_mg.y) to flip from Tiled/Grid (top-left) to CUGL (bottom-left).
            _enemySpawnPoints.push_back(Vec2(ep_mg.x * TILE_SIZE, (_config._mapSize - ep_mg.y) * TILE_SIZE));
            
            auto dist = pl_mg.distance(ep_mg);
            
            if (.1 * _config._mapSize < dist && dist < .4 * _config._mapSize) _z1enemySpawnPoints.push_back(Vec2(ep_mg.x * TILE_SIZE, (_config._mapSize - ep_mg.y) * TILE_SIZE));
            else if (dist < 7. * _config._mapSize) _z2enemySpawnPoints.push_back(Vec2(ep_mg.x * TILE_SIZE, (_config._mapSize - ep_mg.y) * TILE_SIZE));
            else _z3enemySpawnPoints.push_back(Vec2(ep_mg.x * TILE_SIZE, (_config._mapSize - ep_mg.y) * TILE_SIZE));
        }
        // add colliders
        addRoomObstacles(std::get<0>(res), std::get<2>(res), world);
        _rooms.push_back(std::make_shared<Room>(std::get<2>(res), Room::RoomModifier::NONE, TILE_SIZE * std::get<0>(res)->getWidth(), TILE_SIZE * std::get<0>(res)->getHeight()));
            
        }
    
        return node;
    }

void Map::addRoomObstacles(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::physics2::distrib::NetWorld> world) {
    for (auto coll : tiledRmData->getColliders()) {
        vector<Vec2> translatedPoints;
        for (auto pt : coll.vertices) {
            auto pt_mg = cugl::Vec2(tlCorner.x + pt.x,
                                    tlCorner.y + pt.y);
            Vec2 tpt = Vec2(
                (pt_mg.x - 0.5f) * TILE_SIZE / PHYSICS_SCALE,
                (_config._mapSize - pt_mg.y) * TILE_SIZE / PHYSICS_SCALE
            );
            translatedPoints.push_back(tpt);
        }
        
        _extruder.set(translatedPoints, true);
        _extruder.calculate(0.05f);
        auto p = _extruder.getPolygon();
        addWallObstacle(p, world);
    }
}
    
    void Map::addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world, std::shared_ptr<cugl::scene2::SceneNode> lwalls) {
        // iterate over tiled nodes and add to scene
        const auto& floorLayer = tiledRmData->getFloorLayer();
        const auto& wallLayer  = tiledRmData->getWallLayer();
        const auto& fixLayer   = tiledRmData->getFixLayer();
        const auto& decoLayer  = tiledRmData->getDecoLayer();
        const auto& maskLayer  = tiledRmData->getMaskLayer();
        const auto& lwallLayer  = tiledRmData->getBottomWallLayer();

        for (int x =0; x<tiledRmData->getWidth(); x++) {
            for (int y=0; y<tiledRmData->getHeight(); y++){
                int floorGid = floorLayer[y][x];
                int wallGid  = wallLayer[y][x];
                int fixGid   = fixLayer[y][x];
                int decoGid  = decoLayer[y][x];
                int maskGid  = maskLayer[y][x];
                int lwGid = lwallLayer[y][x];
                // subtract y bc going top down
                Vec2 pos = Vec2((tlCorner.x+x), (_config._mapSize - tlCorner.y-y));
                //Size s = Size(tiledRmData->getTileW(), tiledRmData->getTileH());
                //auto cell = Rect(pos, s);
                // add tile layers in order, skip if gid zero
                if(floorGid!=0){
                    addSingleNode(tiledRmData->getTileForGid(floorGid), pos, world);
                }
                if(wallGid!=0){
                    addSingleNode(tiledRmData->getTileForGid(wallGid), pos, world);
                }
                if(fixGid!=0){
                    addSingleNode(tiledRmData->getTileForGid(fixGid), pos, world);
                }
                if(decoGid!=0){
                    addSingleNode(tiledRmData->getTileForGid(decoGid), pos, world);
                }
                if(maskGid !=0){
                    // remember y is row, x is col on mapGrid
                    _mapGrid[(int) tlCorner.y+y][(int) tlCorner.x+x] = 1; // _mapGrid is (0,0) top left corner, so just add
                }
                if(lwGid != 0){
                    addSingleNode(tiledRmData->getTileForGid(lwGid), pos, lwalls);
                }
            }
        }
        CULog("world node has %d children", (int) world->getChildCount());
    }
    
    std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> Map::attemptToFitRoom(MapDoor& d, std::vector<std::string>& rooms, Door::Direction connDir) {
        
        if(rooms.size()==0) return std::make_tuple(nullptr, -1, Vec2(-1, -1)); // no viable room
        
        int attempts = 0;
        //TODO: these will be in level config file perhaps?
        int minHallUnits = 2;
        int maxHallUnits = 6;
        while (attempts < 3){
            CULog("attempt: %d", attempts);
            // select room for this attempt
            int rm_idx = (int) _rng.getOpenSint64(0, rooms.size());
            auto rmData = _assets->get<LevelLoader>(rooms.at(rm_idx));
            
            // try to fit room with varying hall sizes
            for (int i=minHallUnits; i<=maxHallUnits; i++) {
                int tlCornerX, tlCornerY;
                
                switch (connDir) {
                    case Door::Direction::NORTH: // rooms north door is connecting
                        tlCornerY = d.pos.y + d.size.height + i; // mapGrid origin also top left
                        tlCornerX = d.pos.x - rmData->getNorthDoor()->_tlCorner.x; // x pos of hallway, subtract distance from north door corner to room corner, which is just tlCorner.x bc tiled coords
                        break;
                        
                    case Door::Direction::EAST:
                        tlCornerY = d.pos.y - rmData->getEastDoor()->_tlCorner.y;
                        tlCornerX = d.pos.x - rmData->getWidth() - i;
                        break;
                        
                    case Door::Direction::SOUTH: // rooms south door connecting
                        tlCornerY = d.pos.y - i - rmData->getHeight();
                        tlCornerX = d.pos.x - rmData->getSouthDoor()->_tlCorner.x; // same math as north door
                        break;
                        
                    case Door::Direction::WEST:
                        tlCornerY = d.pos.y - rmData->getWestDoor()->_tlCorner.y;
                        tlCornerX = d.pos.x + d.size.width + i;
                        break;
                } // switch case
                // check if room will fit by seeing if each !0 in mask is a 0 on the grid
                bool overlap = false;
                const auto& mask = rmData->getMaskLayer();
                for(int row=0; row<rmData->getHeight(); row++){
                    for(int col=0; col<rmData->getWidth(); col++){
                        if(tlCornerX+col >= _config._mapSize || tlCornerX < 0 || tlCornerY+row>=_config._mapSize || tlCornerY < 0) {
                            CULog("out of bounds");
                            overlap=true;
                            goto exit;
                        }
                        if(mask[row][col]!=0 && _mapGrid[tlCornerY+row][tlCornerX+col]!=0){
                            overlap=true;
                            goto exit;
                        }
                    }
                }
                
            exit:
                if (!overlap) { // this room works, else run again
                    CULog("found room!");
                    auto p = std::make_tuple(rmData, i, Vec2(tlCornerX, tlCornerY));
                    return p;
                }
                
            } // hallway count loop
            attempts++;
        } // attempt loop
        // if we make it here, 3 attempts failed so nullptr
        return std::make_tuple(nullptr, -1, Vec2(-1, -1));
    }
    
    void Map::addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world) {
        auto sprite = cugl::scene2::PolygonNode::allocWithTexture(t);
        
        // Convert grid coordinates directly to world pixels
        // pos.y is already "flipped" in addRoomSceneNodes, so we don't flip it again here
        if(world == lw_node){
            sprite -> setPriority(100.0);
        }
        int bl_y = pos.y - 1; // incoming pos is top left corner, but we need bottom left
        float x = pos.x * TILE_SIZE;
        float y = bl_y * TILE_SIZE;
        //CULog("X: %f, Y %f", x,y);
        sprite->setPosition(Vec2(x, y));
        sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        world->addChild(sprite);
        
    }
    
    void Map::addWallObstacle(Poly2 &poly, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world) {
        auto obs = physics2::PolygonObstacle::alloc(poly);
        //obs->setPosition(0,0);
        obs->setDensity(1.0);
        obs -> setEnabled(true);
        obs->setBodyType(b2_staticBody);
        CULog("%f",obs -> getPosition().x);
        world->addObstacle(obs);
        
    }

void Map::loadLevelConfig(int level, std::shared_ptr<cugl::JsonValue> config) {
    std::shared_ptr<JsonValue> data;
    int incr = 0;
    if (level < config->getInt("medium-start")) {
        data = config->get("easy");
        incr = level - 1;
    } else if (level < config->getInt("hard-start")) {
        data = config->get("medium");
        incr = level - 4;
    } else {
        data = config->get("hard");
        incr = level - 7;
    }
    
    _config._mapSize = data->getInt("map-size") + data->getInt("map-size-incr") * incr;
    
    for (auto e : data->get("enemies")->asStringArray()) {
        if (e == "octoplush") _config._enemyTypes.push_back(EnemyType::OCTO);
        else if (e == "ghost") _config._enemyTypes.push_back(EnemyType::GHOST);
        else if (e == "bat") _config._enemyTypes.push_back(EnemyType::BAT);
        else if (e == "slow-bat") _config._enemyTypes.push_back(EnemyType::SLOWBAT);
    }
    
    _config._zone1WeightsEn = data->get("enemies-probs")->get(0)->asFloatArray();
    _config._zone2WeightsEn = data->get("enemies-probs")->get(1)->asFloatArray();
    _config._zone3WeightsEn = data->get("enemies-probs")->get(2)->asFloatArray();
    
    _config._zone1EnemyProb = data->get("zone-probs")->asFloatArray()[0];
    _config._zone2EnemyProb = data->get("zone-probs")->asFloatArray()[1];
    _config._zone3EnemyProb = data->get("zone-probs")->asFloatArray()[2];

    
    // TODO: add parsing for item data
}

std::vector<std::pair<Vec2, EnemyType>> Map::generateEnemySpawns(const std::vector<Vec2> playersPos) {
    std::vector<std::pair<Vec2, EnemyType>> res;
    // it is fine to use the random number generator here since we re-init for next round
    bool z1 = _rng.getFloat() <= _config._zone1EnemyProb;
    bool z2 = _rng.getFloat() <= _config._zone2EnemyProb;
    bool z3 = _rng.getFloat() <= _config._zone3EnemyProb;
    
    if (z1 && _z1enemySpawnPoints.size()>0) {
        CULog("zone 1 spawn");
        auto spawn = chooseEnemySpawnPoint(_z1enemySpawnPoints, playersPos, _config._zone1WeightsEn);
        res.push_back(spawn);
    }
    
    if (z2 && _z2enemySpawnPoints.size()>0) {
        CULog("zone 2 spawn");
        auto spawn = chooseEnemySpawnPoint(_z2enemySpawnPoints, playersPos, _config._zone2WeightsEn);
        res.push_back(spawn);
    }
    
    if (z3 && _z3enemySpawnPoints.size()>0) {
        CULog("zone 3 spawn");
        auto spawn = chooseEnemySpawnPoint(_z3enemySpawnPoints, playersPos, _config._zone3WeightsEn);
        res.push_back(spawn);
    }
    return res;
}

std::pair<Vec2, EnemyType> Map::chooseEnemySpawnPoint(const std::vector<Vec2> zonePoints, const std::vector<Vec2> playersPos, const std::vector<float> zoneRates) {
    // filter to valid points
    vector<Vec2> validPts;
    
    for (auto p : zonePoints) {
        bool valid = true;
        for (auto pos : playersPos) {
            if (p.distance(pos) < 10 * TILE_SIZE) { // since points are in tile coords
                valid = false;
                break;
            }
        }
        if (valid) validPts.push_back(p);
    }
    auto idx = _rng.getOpenSint64(0, validPts.size());
    
    float enem = _rng.getFloat();

    float acc = 0;
    int enemIdx = -1;
    while (acc < enem) {
        enemIdx++;
        acc += zoneRates[enemIdx];
    }
    return std::make_pair(validPts.at(idx), _config._enemyTypes[enemIdx]);
}

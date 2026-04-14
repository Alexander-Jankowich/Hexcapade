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
    for (auto a : _mapGrid) a.fill(0);
}

std::shared_ptr<cugl::scene2::SceneNode> Map::generate(const MapGenerationConfig& config, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms) {
    
    auto node = scene2::SceneNode::alloc();
    std::queue<MapDoor> doors;
    _rng = cugl::Random();
    _rng.initWithSeed(seed);
    _assets = assets;
    
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
    int rm_idx = (int) _rng.getClosedSint64(0, rooms->size()-1);
    std::string& room = rooms->at(rm_idx);
    auto tiledData = _assets->get<LevelLoader>(room);
    
    // room top left corner at (width/2 - roomW /2, height/2 + roomH/2)
    auto tlcorner = Vec2(MAP_WIDTH/2 - tiledData->getWidth()/2, MAP_HEIGHT/2 + tiledData->getHeight()/2);
    
    addRoomSceneNodes(tiledData, tlcorner, node);
    
    // get playerSpawn: (mapwidth/2 - roomw /2 + playerSpawn.x, mapheight/2 - roomH/2 + (roomH - playerspawn.y)
    _playerSpawn = cugl::Vec2(MAP_WIDTH/2 - tiledData->getWidth() / 2 + tiledData->getPlayerSpawn().x, MAP_HEIGHT / 2 + tiledData->getHeight() /2 - tiledData->getPlayerSpawn().y);
    
    // add all doors in room to doors queue
    for (auto d : tiledData->getDoors()) {
        // pos is the TOP LEFT corner of door
        auto md = MapDoor(Vec2(tlcorner.x + d->_tlCorner.x, tlcorner.y - d->_tlCorner.y), d->_dimens / tiledData->getTileH(), d->_direction);
        doors.push(md);
    }
    
    
    // iterate over doors queue
    while (!doors.empty()) {
        auto door = doors.front();
        doors.pop();
        
        // gids to wall off doors: 100 for east door, 99 for west, (10/23/36) for north and (102 115 128) for south
        
        switch (door.dir) {
            case Door::Direction::NORTH:
                {
                    // check if a room will fit - call attemptToFitRoom
                    std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> res = attemptToFitRoom(door, hasSouthDoors, Door::Direction::SOUTH);
                    if (std::get<0>(res) == nullptr){
                        // wall off this room gids (10/23/36) x5 across doorway
                        // TODO: ensure this is fine with map bounds or if room close to edge
                        for (int i = door.pos.x; i <= door.pos.x + 5; i++) {
                            // wall 1
                            addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 3), node);
                            
                            // wall 2
                            addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 2), node);

                            // wall 3
                            addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 1), node);
                        }
                        
                        //TODO: add collider
                        continue;
                    }
                    
                    for (int y=door.pos.y; y > door.pos.y - std::get<1>(res); y--) {
                        // add west wall
                        addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x - 1, MAP_HEIGHT - y), node);
                        // add 5 floor tiles
                        for (int x=door.pos.x; x < door.pos.x +5; x++){
                            addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                        }
                        // add east wall
                        addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x +5, MAP_HEIGHT - y), node);
                    }
                    
                    addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node);
                    break;
                }
            case Door::Direction::SOUTH:
            {
                // check if a room will fit - call attemptToFitRoom
                std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> res = attemptToFitRoom(door, hasNorthDoors, Door::Direction::NORTH);
                if (std::get<0>(res) == nullptr){
                    // wall off this room x5 across doorway
                    // TODO: ensure this is fine with map bounds or if room close to edge
                    for (int i = door.pos.x; i <= door.pos.x + 5; i++) {
                        // wall 1
                        addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 3), node);
                        
                        // wall 2
                        addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 2), node);

                        // wall 3
                        addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 1), node);
                    }
                    
                    //TODO: add collider
                    continue;
                }
                
                for (int y=door.pos.y; y < door.pos.y + std::get<1>(res); y++) {
                    // add west wall
                    addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x - 1, MAP_HEIGHT - y), node);
                    // add 5 floor tiles
                    for (int x=door.pos.x; x < door.pos.x +5; x++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                    }
                    // add east wall
                    addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x +5, MAP_HEIGHT - y), node);
                }
                //TODO: hallway colliders
                addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node);
                break;
            }
            case Door::Direction::EAST:
            {
                // check if a room will fit - call attemptToFitRoom
                std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> res = attemptToFitRoom(door, hasEastDoors, Door::Direction::EAST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    for (int i = door.pos.y; i <= door.pos.y + 5; i++) {
                        addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x-1, MAP_HEIGHT - i), node);
                    }
                    //TODO: add collider
                    continue;
                }
                
                for (int x=door.pos.x; x > door.pos.x - std::get<1>(res); x--) {
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 2), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 1), node);
                    
                    // add 5 floor tiles
                    for (int y=door.pos.y; x < door.pos.y +5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 3), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 4), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 5), node);
                }
                //TODO: hallway colliders
                addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node);
                break;
            }
            case Door::Direction::WEST:
            {
                // check if a room will fit - call attemptToFitRoom
                std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> res = attemptToFitRoom(door, hasEastDoors, Door::Direction::EAST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    for (int i = door.pos.y; i <= door.pos.y + 5; i++) {
                        addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x+1, MAP_HEIGHT - i), node);
                    }
                    //TODO: add collider
                    continue;
                }
                
                for (int x=door.pos.x; x < door.pos.x + std::get<1>(res); x++) {
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 2), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 1), node);
                    // add 5 floor tiles
                    for (int y=door.pos.y; x < door.pos.y +5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 3), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 4), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 5), node);
                }
                //TODO: hallway colliders
                addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node);
                break;
            }
        }
        
    }
    
    
    return node;
}

void Map::addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world) {
    // iterate over tiled nodes and add to scene
    for (int x =0; x<tiledRmData->getWidth(); x++) {
        for (int y=0; y<tiledRmData->getHeight(); y++){
            int floorGid = tiledRmData->getFloorLayer().at(y).at(x);
            int wallGid = tiledRmData->getWallLayer().at(y).at(x);
            int fixGid = tiledRmData->getFixLayer().at(y).at(x);
            int decoGid = tiledRmData->getDecoLayer().at(y).at(x);
            int maskGid = tiledRmData->getMaskLayer().at(y).at(x);
            
            // multiply by tile size to convert to world coords? subtract y bc going top down
            // TODO: check math
            Vec2 pos = Vec2((tlCorner.x+x), (MAP_HEIGHT - tlCorner.y-y));
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
                _mapGrid[tlCorner.y+y][tlCorner.x+x] = 1; // _mapGrid is (0,0) top left corner, so just add
            }
        }
    }
    
    // add colliders
    
}

std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> Map::attemptToFitRoom(MapDoor d, std::vector<std::string> rooms, Door::Direction connDir) {
    int attempts = 0;
    //TODO: these will be in level config file perhaps?
    int minHallUnits = 3;
    int maxHallUnits = 6;
    while (attempts < 3){
        // select room for this attempt
        int rm_idx = (int) _rng.getClosedSint64(0, rooms.size()-1);
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
                    tlCornerY = d.pos.y - rmData->getEastDoor()->_tlCorner.y;
                    tlCornerX = d.pos.x + rmData->getWidth() + i;
                    break;
            }
            // check if room will fit by seeing if each !0 in mask is a 0 on the grid
            bool overlap = false;
            for(int row=0; row<rmData->getHeight(); row++){
                for(int col=0; col<rmData->getWidth(); col++){
                    if(tlCornerX+col >= MAP_WIDTH || tlCornerY+row>=MAP_HEIGHT) {
                        overlap=true;
                        goto exit;
                    }
                    if(rmData->getMaskLayer().at(row).at(col)!=0 && _mapGrid[tlCornerY+row][tlCornerX+col]!=0){
                        overlap=true;
                        goto exit;
                    }
                }
            }
            
        exit:
            if (!overlap) { // this room works, else run again
                auto p = std::make_tuple(rmData, i, Vec2(tlCornerX, tlCornerY));
                return p;
            }
            
        } // hallway count loop
    } // attempt loop
    // if we make it here, 3 attempts failed so nullptr
    return std::make_tuple(nullptr, -1, Vec2(-1, -1));
}

void Map::addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world) {
    auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
    sprite->initWithTexturePoly(t, Rect(pos, Size(TILE_SIZE, TILE_SIZE)));
    world -> addChild(sprite);
}


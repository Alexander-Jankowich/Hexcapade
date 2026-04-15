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

std::shared_ptr<cugl::scene2::SceneNode> Map::generate(const MapGenerationConfig& config, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms, const Size& sceneSize) {
    _sceneSize = sceneSize;
    auto node = scene2::SceneNode::alloc();
    std::vector<MapDoor> doors;
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
    int rm_idx = (int) _rng.getOpenSint64(0, rooms->size());
    std::string& room = rooms->at(rm_idx);
    auto tiledData = _assets->get<LevelLoader>(room);
    
    // room top left corner ON THE MAP GRID at (width/2 - roomW /2, height/2 - roomH/2) since up is 0
    auto tlcorner = Vec2(MAP_WIDTH/2 - tiledData->getWidth()/2, MAP_HEIGHT/2 - tiledData->getHeight()/2);
    CULog("tlcorner first room is x: %f y: %f", tlcorner.x, tlcorner.y);
    
    addRoomSceneNodes(tiledData, tlcorner, node);
    
    // get playerSpawn: (mapwidth/2 - roomw /2 + playerSpawn.x, mapheight/2 - roomH/2 + (roomH - playerspawn.y)
    // Corrected player spawn calculation
    auto pl_mg = cugl::Vec2(MAP_WIDTH/2 - tiledData->getWidth() / 2 + tiledData->getPlayerSpawn().x,
                            MAP_HEIGHT / 2 - tiledData->getHeight() / 2 + tiledData->getPlayerSpawn().y);

    // Just map grid units to world pixels.
    // We use (MAP_HEIGHT - pl_mg.y) to flip from Tiled/Grid (top-left) to CUGL (bottom-left).
    _playerSpawn = Vec2(pl_mg.x * TILE_SIZE, (MAP_HEIGHT - pl_mg.y) * TILE_SIZE);
    // add all doors in room to doors queue
    for (auto d : tiledData->getDoors()) {
        // pos is the TOP LEFT corner of door
        auto md = MapDoor(Vec2(tlcorner.x + d->_tlCorner.x, tlcorner.y + d->_tlCorner.y), d->_dimens / tiledData->getTileH(), d->_direction);
        doors.push_back(md);
    }
    _numRooms++;
    
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
        
        switch (door.dir) {
            case Door::Direction::NORTH:
                {
                    // check if a room will fit - call attemptToFitRoom
                    res = attemptToFitRoom(door, hasSouthDoors, Door::Direction::SOUTH);
                    if (std::get<0>(res) == nullptr){
                        // wall off this room gids (10/23/36) x5 across doorway
                        // TODO: ensure this is fine with map bounds or if room close to edge
                        CULog("walling off - N");
                        for (int i = door.pos.x; i < door.pos.x + 5; i++) {
                            // wall 1
                            addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 3), node);
                            if (door.pos.y-3 >= 0) _mapGrid[door.pos.y-3][i] = 1;
                            // wall 2
                            addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 2), node);
                            if (door.pos.y-2 >= 0)_mapGrid[door.pos.y-2][i] = 1;
                            // wall 3
                            addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 1), node);
                            if (door.pos.y-1 >= 0)_mapGrid[door.pos.y-1][i] = 1;

                        }
                        CULog("done walling off - N");
                        //TODO: add collider
                        continue;
                    }
                    CULog("adding halls - N");
                    for (int y=door.pos.y; y > door.pos.y - std::get<1>(res); y--) {
                        // add west wall
                        addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x - 1, MAP_HEIGHT - y), node);
                        _mapGrid[y][door.pos.x-1] = 1;
                        // add 5 floor tiles
                        for (int x=door.pos.x; x < door.pos.x +5; x++){
                            addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                            _mapGrid[y][x] = 1;
                        }
                        // add east wall
                        addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x +5, MAP_HEIGHT - y), node);
                        _mapGrid[y][door.pos.x+5] = 1;

                    }
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
                    for (int i = door.pos.x; i < door.pos.x + 5; i++) {
                        // wall 1
                        addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 2), node);
                        _mapGrid[door.pos.y-2][i] = 1;
                        // wall 2
                        addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(i, MAP_HEIGHT - door.pos.y + 1), node);
                        _mapGrid[door.pos.y-1][i] = 1;
                        // wall 3
                        addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(i, MAP_HEIGHT - door.pos.y), node);
                        _mapGrid[door.pos.y][i] = 1;
                    }
                    CULog("done walling off - S");

                    //TODO: add collider
                    continue;
                }
                CULog("adding halls - S");

                for (int y=door.pos.y; y < door.pos.y + std::get<1>(res); y++) {
                    // add west wall
                    addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x - 1, MAP_HEIGHT - y), node);
                    _mapGrid[y][door.pos.x - 1] = 1;
                    // add 5 floor tiles
                    for (int x=door.pos.x; x < door.pos.x +5; x++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add east wall
                    addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x +5, MAP_HEIGHT - y), node);
                    _mapGrid[y][door.pos.x+5] = 1;

                }
                to_skip = Door::Direction::NORTH;
                CULog("done adding halls - S");
                //TODO: hallway colliders
                break;
            }
            case Door::Direction::EAST:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasWestDoors, Door::Direction::WEST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    CULog("walling off - E");

                    for (int i = door.pos.y - 2; i < door.pos.y + 4; i++) {
                        addSingleNode(tiledData->getTileForGid(EAST_GID), Vec2(door.pos.x+1, MAP_HEIGHT - i), node);
                        if(door.pos.x < MAP_WIDTH - 1) _mapGrid[i][door.pos.x + 1] = 1;
                    }
                    //TODO: add collider
                    continue;
                }
                CULog("adding halls - E");

                for (int x=door.pos.x; x <= door.pos.x + std::get<1>(res); x++) {
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 2), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 1), node);
                    _mapGrid[door.pos.y-3][x] = 1;
                    _mapGrid[door.pos.y-2][x] = 1;
                    _mapGrid[door.pos.y-1][x] = 1;

                    // add 5 floor tiles
                    for (int y=door.pos.y; y < door.pos.y +5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 3), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 4), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 5), node);
                    _mapGrid[door.pos.y+5][x] = 1;
                    _mapGrid[door.pos.y+4][x] = 1;
                    _mapGrid[door.pos.y+3][x] = 1;
                }
                to_skip = Door::Direction::WEST;
                CULog("done adding halls - E");
                //TODO: hallway colliders
                break;
            }
            case Door::Direction::WEST:
            {
                // check if a room will fit - call attemptToFitRoom
                res = attemptToFitRoom(door, hasEastDoors, Door::Direction::EAST);
                if (std::get<0>(res) == nullptr){
                    // wall off this door
                    CULog("walling off - W");

                    for (int i = door.pos.y - 2; i < door.pos.y + 4; i++) {
                        addSingleNode(tiledData->getTileForGid(WEST_GID), Vec2(door.pos.x-1, MAP_HEIGHT - i), node);
                        if(door.pos.x > 0) _mapGrid[i][door.pos.x -1] = 1;
                    }
                    CULog("done walling off - W");

                    //TODO: add collider
                    continue;
                }
                CULog("adding halls - W");
                for (int x=door.pos.x; x >= door.pos.x - std::get<1>(res); x--) {
                    CULog("adding hall at %d", x);
                    // add top walls
                    addSingleNode(tiledData->getTileForGid(NORTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 3), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 2), node);
                    addSingleNode(tiledData->getTileForGid(NORTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y + 1), node);
                    _mapGrid[door.pos.y-3][x] = 1;
                    _mapGrid[door.pos.y-2][x] = 1;
                    _mapGrid[door.pos.y-1][x] = 1;
                    // add 5 floor tiles
                    for (int y=door.pos.y; y < door.pos.y +5; y++){
                        addSingleNode(tiledData->getTileForGid(FLOOR_GID), Vec2(x, MAP_HEIGHT - y), node);
                        _mapGrid[y][x] = 1;
                    }
                    // add bottom walls
                    addSingleNode(tiledData->getTileForGid(SOUTH_TOP_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 3), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_MID_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 4), node);
                    addSingleNode(tiledData->getTileForGid(SOUTH_LOW_GID), Vec2(x, MAP_HEIGHT - door.pos.y - 5), node);
                    _mapGrid[door.pos.y+3][x] = 1;
                    _mapGrid[door.pos.y+4][x] = 1;
                    _mapGrid[door.pos.y+5][x] = 1;
                }
                to_skip = Door::Direction::EAST;
                CULog("done adding halls - W");

                //TODO: hallway colliders
                break;
            }
        }
        // for all, if here then res successfully filled in with room data. add room + doors + incr counter (later add room obj)
        CULog("adding room and doors");
        addRoomSceneNodes(std::get<0>(res), std::get<2>(res), node);
        for (auto d : std::get<0>(res)->getDoors()) {
            // pos is the TOP LEFT corner of door
            if (d->_direction!=to_skip){
                auto md = MapDoor(Vec2(std::get<2>(res).x + d->_tlCorner.x, std::get<2>(res).y + d->_tlCorner.y), d->_dimens / std::get<0>(res)->getTileH(), d->_direction);
                doors.push_back(md);
            }
        }
        _numRooms++;
        
    }
    
    
    return node;
}

void Map::addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world) {
    // iterate over tiled nodes and add to scene
    const auto& floorLayer = tiledRmData->getFloorLayer();
    const auto& wallLayer  = tiledRmData->getWallLayer();
    const auto& fixLayer   = tiledRmData->getFixLayer();
    const auto& decoLayer  = tiledRmData->getDecoLayer();
    const auto& maskLayer  = tiledRmData->getMaskLayer();
    for (int x =0; x<tiledRmData->getWidth(); x++) {
        for (int y=0; y<tiledRmData->getHeight(); y++){
            int floorGid = floorLayer[y][x];
            int wallGid  = wallLayer[y][x];
            int fixGid   = fixLayer[y][x];
            int decoGid  = decoLayer[y][x];
            int maskGid  = maskLayer[y][x];
            
            // subtract y bc going top down
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
                // remember y is row, x is col on mapGrid
                _mapGrid[tlCorner.y+y][tlCorner.x+x] = 1; // _mapGrid is (0,0) top left corner, so just add
            }
        }
    }
    CULog("world has %d children", (int) world->getChildCount());
    // add colliders
    
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
                    if(tlCornerX+col >= MAP_WIDTH || tlCornerX < 0 || tlCornerY+row>=MAP_HEIGHT || tlCornerY < 0) {
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
    float x = pos.x * TILE_SIZE;
    float y = pos.y * TILE_SIZE;
    //CULog("X: %f, Y %f", x,y);
    sprite->setPosition(Vec2(x, y));
    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    world->addChild(sprite);
}

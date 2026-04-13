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

#define PHYSICS_SCALE 100
#define FLOOR_GID 66
#define EMPTY_TILE 0


using namespace cugl;
using namespace cugl::graphics;


Map::Map() :
_origin(Vec2::ZERO),
_playerSpawn(Vec2::ZERO),
_config() {
    for (auto a : _mapGrid) a.fill(0);
}

std::shared_ptr<cugl::scene2::SceneNode> Map::generate(const MapGenerationConfig& config, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms) {
    
    auto node = scene2::SceneNode::alloc();
    std::queue<std::shared_ptr<MapDoor>> doors;
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
    
    // select first room and add to center of the map
    int rm_idx = (int) (_rng.getFloat() * (int) rooms->size());
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
        auto md = std::make_shared<MapDoor>(Vec2(tlcorner.x + d->_tlCorner.x, tlcorner.y - d->_tlCorner.y), d->_dimens, d->_direction);
        doors.push(md);
    }
    
    
    // iterate over doors queue
    while (!doors.empty()) {
        auto door = doors.front();
        doors.pop();
        
        // gids to wall off doors: 100 for east door, 99 for west, (10/23/36) for north and (102 115 128) for south
        
        
        switch (door->dir) {
            case Door::Direction::NORTH:
                // check if a room will fit
                
                // western wall 99 eastern wall 100
                // floor is 4 floor tiles 66
                // start at top left corner of door + 1 tile?, fill in 4 x 3 section of floor
                
                // add walls
                
                break;
            case Door::Direction::SOUTH:
                break;
            case Door::Direction::EAST:
                // walls (back) (east/west)
                // 10
                // 23
                // 36
                // walls (bottom)
                // 102
                // 115
                // 128
                // floor: 66
                break;
            case Door::Direction::WEST:
                break;
        }
        // randomly select new room
        rm_idx = (int) (_rng.getFloat() * (int) rooms->size());
        room = rooms->at(rm_idx);
        // room bottom left corner at (width/2 - tilew /2, height/2 - tileheight/2)
        
        //helper function to create all the sceneNodes
        
        // add all doors in room to doors queue
        
        
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
            Vec2 pos = Vec2((tlCorner.x+x) * tiledRmData->getTileW(), (tlCorner.y-y) * tiledRmData->getTileH());
            Size s = Size(tiledRmData->getTileW(), tiledRmData->getTileH());
            auto cell = Rect(pos, s);
            // add tile layers in order, skip if gid zero
            if(floorGid!=0){
                auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                std::shared_ptr<Texture> tex = tiledRmData->getTileForGid(floorGid);
                sprite->initWithTexturePoly(tex, cell);
                world -> addChild(sprite);
            }
            if(wallGid!=0){
                auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                std::shared_ptr<Texture> tex = tiledRmData->getTileForGid(wallGid);
                sprite->initWithTexturePoly(tex, cell);
                world -> addChild(sprite);
            }
            if(fixGid!=0){
                auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                std::shared_ptr<Texture> tex = tiledRmData->getTileForGid(fixGid);
                sprite->initWithTexturePoly(tex, cell);
                world -> addChild(sprite);
            }
            if(decoGid!=0){
                auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
                std::shared_ptr<Texture> tex = tiledRmData->getTileForGid(decoGid);
                sprite->initWithTexturePoly(tex, cell);
                world -> addChild(sprite);
            }
            if(maskGid !=0){
                _mapGrid[tlCorner.y-y][tlCorner.x+x] = 1; // x, y are tiled coords
            }
        }
    }
}


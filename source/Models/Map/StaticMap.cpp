//
//  StaticMap.cpp
//  hexcapade
//
//  Created by Serena Duncan on 5/8/26.
//

#include "StaticMap.h"

StaticMap::StaticMap() : _origin(Vec2::ZERO), _sceneSize(Size::ZERO), lw_node(nullptr) {
}
StaticMap::~StaticMap() {
}

std::shared_ptr<cugl::scene2::SceneNode> StaticMap::generateStaticMap(std::shared_ptr<AssetManager> assets, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world) {
    _assets = assets;
    auto node = scene2::SceneNode::alloc();
    lw_node = scene2::SceneNode::alloc();
    _data = _assets->get<StaticLevelLoader>("tutorial");
    addRoomSceneNodes(_data, Vec2(0, 0), node, lw_node);
    addRoomObstacles(_data, Vec2(0, 0), world);
    addRoomEvents(_data, Vec2(0, 0), world);
    return node;
}

void StaticMap::addRoomEvents(std::shared_ptr<StaticLevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::physics2::distrib::NetWorld> world) {
    for (auto e : tiledRmData->getEvents()) {
        // convert rectangle origin to map coords
        Rect r = Rect(e.location.origin.x * TILE_SIZE / PHYSICS_SCALE, (tiledRmData->getHeight() - e.location.origin.y) * TILE_SIZE / PHYSICS_SCALE,  e.location.size.width * TILE_SIZE / PHYSICS_SCALE, e.location.size.height * TILE_SIZE / PHYSICS_SCALE);
        // get texture
        std::string t = e.png;
        CULog("Event '%s': tiled origin=(%.2f, %.2f) size=(%.2f, %.2f) -> physics rect origin=(%.2f, %.2f) size=(%.2f, %.2f)",
                    t.c_str(),
                    e.location.origin.x, e.location.origin.y,
                    e.location.size.width, e.location.size.height,
                    r.origin.x, r.origin.y,
                    r.size.width, r.size.height
                );
        
        
        addEventObstacle(r, t, world);
    }
}

void StaticMap::addRoomObstacles(std::shared_ptr<StaticLevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::physics2::distrib::NetWorld> world) {
    for (auto coll : tiledRmData->getColliders()) {
        vector<Vec2> translatedPoints;
        for (auto pt : coll.vertices) {
            auto pt_mg = cugl::Vec2(tlCorner.x + pt.x,
                                    tlCorner.y + pt.y);
            Vec2 tpt = Vec2(
                (pt_mg.x - 0.5f) * TILE_SIZE / PHYSICS_SCALE,
                (tiledRmData->getHeight() - pt_mg.y) * TILE_SIZE / PHYSICS_SCALE
            );
            translatedPoints.push_back(tpt);
        }
        
        _extruder.set(translatedPoints, true);
        _extruder.calculate(0.05f);
        auto p = _extruder.getPolygon();
        addWallObstacle(p, world);
    }
}
    
void StaticMap::addRoomSceneNodes(std::shared_ptr<StaticLevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world, std::shared_ptr<cugl::scene2::SceneNode> lwalls) {
        // iterate over tiled nodes and add to scene
        const auto& floorLayer = tiledRmData->getFloorLayer();
        const auto& wallLayer  = tiledRmData->getWallLayer();
        const auto& fixLayer   = tiledRmData->getFixLayer();
        const auto& decoLayer  = tiledRmData->getDecoLayer();
        const auto& lwallLayer  = tiledRmData->getBottomWallLayer();

        for (int x =0; x<tiledRmData->getWidth(); x++) {
            for (int y=0; y<tiledRmData->getHeight(); y++){
                int floorGid = floorLayer[y][x];
                int wallGid  = wallLayer[y][x];
                int fixGid   = fixLayer[y][x];
                int decoGid  = decoLayer[y][x];
                int lwGid = lwallLayer[y][x];
                // subtract y bc going top down
                Vec2 pos = Vec2((tlCorner.x+x), (tiledRmData->getHeight() - tlCorner.y-y));
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
                if(lwGid != 0){
                    addSingleNode(tiledRmData->getTileForGid(lwGid), pos, lwalls);
                }
            }
        }
        CULog("world node has %d children", (int) world->getChildCount());
    }

void StaticMap::addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world) {
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

void StaticMap::addWallObstacle(Poly2 &poly, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world) {
    auto obs = physics2::PolygonObstacle::alloc(poly);
    //obs->setPosition(0,0);
    obs->setDensity(1.0);
    obs -> setEnabled(true);
    obs->setBodyType(b2_staticBody);
    CULog("%f",obs -> getPosition().x);
    world->addObstacle(obs);
}

void StaticMap::addEventObstacle(Rect &poly, std::string tex, std::shared_ptr<cugl::physics2::distrib::NetWorld> &world) {
    std::shared_ptr<TutorialObstacle> obs = TutorialObstacle::alloc(poly, tex);
    // copying wall obstacle code since i am hoping they are the same...
    obs->setDensity(1.0);
    obs -> setEnabled(true);
    obs->setBodyType(b2_staticBody);
    world->addObstacle(obs);
}

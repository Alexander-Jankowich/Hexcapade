//
//  StaticMap.h
//  hexcapade
//
//  Created by Serena Duncan on 5/8/26.
//

#ifndef __STATICMAP_H__
#define __STATICMAP_H__


#include <cugl/cugl.h>
#include "StaticLevelLoader.h"

#define TUT_QUOTA 50
#define TILE_SIZE 40
#define PHYSICS_SCALE 50 //TODO: find a way to like. not redefine it lol

using namespace cugl;
using namespace cugl::graphics;

class StaticMap {
private:
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<StaticLevelLoader> _data;
    cugl::Vec2 _origin;
    cugl::Size _sceneSize;
    SimpleExtruder _extruder;
    std::shared_ptr<cugl::scene2::SceneNode> lw_node;
    
public:
    StaticMap();
    ~StaticMap();
    
    std::shared_ptr<cugl::scene2::SceneNode> getLowerWallNode() {return lw_node;}
    std::shared_ptr<StaticLevelLoader> getData() {return _data;}
    int getQuota() {return TUT_QUOTA;}

    
    std::shared_ptr<cugl::scene2::SceneNode> generateStaticMap(std::shared_ptr<AssetManager> assets, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world);
    
private:
    /** Helper function to add all sceneNodes for a room (given tiled data) in the right place.
     * Side effect:  fills in the mapGrid with 1s for each space this room covers
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in mapGrid coordinates
     * @param world the base sceneNode to add all created nodes to
     * @param lwalls the sceneNode to add all the lower walls to
     */
    void addRoomSceneNodes(std::shared_ptr<StaticLevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world, std::shared_ptr<cugl::scene2::SceneNode> lwalls);
    
    /** Helper function to add all obstacles for a room
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in mapGrid coordinates
     * @param world the physics networld to add all created obstacles to
     */
    void addRoomObstacles(std::shared_ptr<StaticLevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::physics2::distrib::NetWorld> world);
    
    void addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world);
    
    void addWallObstacle(Poly2& poly, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world);
};

#endif // !__STATICMAP_H__

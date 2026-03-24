//
//  RectFactory.h
//  tech_final
//
//  Created by Serena Duncan on 3/22/26.
//

#ifndef __RECT_FACTORY_H__
#define __RECT_FACTORY_H__

#include <cugl/cugl.h>
#include "TiledRoomLoader.h"

using namespace cugl;
using namespace cugl::physics2::distrib;

/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class RectObstacle : public physics2::BoxObstacle {
protected:
    /** bottom left corner position */
    cugl::Vec2 _pos;
    cugl::Size _dimens;

public:
    RectObstacle(const cugl::Vec2 pos, float w, float h);

    static std::shared_ptr<RectObstacle> alloc(const cugl::Vec2 pos, float w, float h) {
        auto f = std::make_shared<RectObstacle>(pos, w, h);
        //f->createFixtures();
        return f;
    }


    Vec2 getPos() {return _pos;}
    Size getDimensions() {return _dimens;}
};

/**
 * The factory class for crate objects.
 *
 * This class is used to support automatically syncing newly added obstacle mid-simulation.
 * Obstacles added throught the ObstacleFactory class from one client will be added to all
 * clients in the simulations.
 */
class RectFactory : public ObstacleFactory {
protected:
    /** Pointer to the AssetManager for texture access, etc. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<TiledRoomData> _tiledData;
    /** Serializer for supporting parameters */
    LWSerializer _serializer;
    /** Deserializer for supporting parameters */
    LWDeserializer _deserializer;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, float w, float h, unsigned int t_gid);


public:
    RectFactory() {};
    /**
     * Allocates a new instance of the factory using the given AssetManager.
     */
    static std::shared_ptr<RectFactory> alloc(std::shared_ptr<AssetManager>& assets, std::shared_ptr<TiledRoomData>& tiles) {
        auto f = std::make_shared<RectFactory>();
        f->init(assets, tiles);
        return f;
    };

    /**
     * Initializes empty factories using the given AssetManager.
     */
    void init(std::shared_ptr<AssetManager>& assets, std::shared_ptr<TiledRoomData>& tiles) {
        _assets = assets;
        _tiledData = tiles;

    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    std::shared_ptr<std::vector<std::byte>> serializeParams(Vec2 pos, float w, float h, unsigned int t_gid);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(const std::vector<std::byte>& params) override;

};

#endif // !__RECT_FACTORY_H__

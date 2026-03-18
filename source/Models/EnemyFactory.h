//
//  EnemyFactory.h
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#ifndef __ENEMY_FACTORY_H__
#define __ENEMY_FACTORY_H__

#include <cugl/cugl.h>
#include "Enemy.h"

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;
/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class EnemyObstacle : public physics2::CapsuleObstacle {
protected:
    int _health;
public:
    EnemyObstacle(Vec2 pos, cugl::Size s, int h);

    static shared_ptr<EnemyObstacle> alloc(Vec2 pos, cugl::Size& s, int h) {
        auto f = make_shared<EnemyObstacle>(pos, s, h);
        return f;
    }

};

/**
 * The factory class for crate objects.
 *
 * This class is used to support automatically syncing newly added obstacle mid-simulation.
 * Obstacles added throught the ObstacleFactory class from one client will be added to all
 * clients in the simulations.
 */
class EnemyFactory : public ObstacleFactory {
protected:
    /** Pointer to the AssetManager for texture access, etc. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** Serializer for supporting parameters */
    LWSerializer _serializer;
    /** Deserializer for supporting parameters */
    LWDeserializer _deserializer;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, Vec2 velocity, int h);


public:
    /**
     * Allocates a new instance of the factory using the given AssetManager.
     */
    static shared_ptr<EnemyFactory> alloc(std::shared_ptr<AssetManager>& assets) {
        auto f = make_shared<EnemyFactory>();
        f->init(assets);
        return f;
    };

    /**
     * Initializes empty factories using the given AssetManager.
     */
    void init(shared_ptr<AssetManager>& assets) {
        _assets = assets;
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, Vec2 velocity, int h);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;
};

#endif // !__ENEMY_FACTORY_H__

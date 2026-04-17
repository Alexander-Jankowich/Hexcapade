//
//  EnemyFactory.h
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#ifndef __ENEMY_FACTORY_H__
#define __ENEMY_FACTORY_H__

#include <cugl/cugl.h>
#include "EnemyObstacle.h"
#include "EnemyNode.h"

#define ATTACK_DELAY 1.3f

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;

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

    std::shared_ptr<cugl::JsonValue> _enemyConsts;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(EnemyType type, Vec2 pos);

public:
    EnemyFactory() {}

    /** Set of all current enemies */
    std::unordered_set<std::shared_ptr<EnemyObstacle>> current;

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
        _enemyConsts = _assets->get<JsonValue>("constants")->get("enemies");
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(EnemyType type, Vec2 pos);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;

    /**
    * The linking function for enemy obstacles -> enemy nodes
    */
    void linkFunc(const shared_ptr<EnemyObstacle> obs, const shared_ptr<EnemyNode> node);

    /**
    * Returns a set of enemies that have been flagged for garbage collection and removes them from
    * the current enemy set. Should be called every update frame
    * 
    * @return a set of all enemies to be removed from the world
    */
    std::unordered_set<std::shared_ptr<EnemyObstacle>> getRemoved();
};

#endif // !__ENEMY_FACTORY_H__

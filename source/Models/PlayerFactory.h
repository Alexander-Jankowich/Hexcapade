//
//  PlayerFactory.h
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#ifndef __PLAYER_FACTORY_H__
#define __PLAYER_FACTORY_H__

#include <cugl/cugl.h>
#include "Player.h"

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class PlayerObstacle : public physics2::CapsuleObstacle {
protected:
    string _pid;
public:
    PlayerObstacle(Vec2 pos, std::string pid, cugl::Size s);

    static shared_ptr<PlayerObstacle> alloc(Vec2 pos, string pid, cugl::Size& s) {
        auto f = make_shared<PlayerObstacle>(pos, pid, s);
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
class PlayerFactory : public ObstacleFactory {
protected:
    /** Pointer to the AssetManager for texture access, etc. */
    shared_ptr<cugl::AssetManager> _assets;
    /** Serializer for supporting parameters */
    cugl::netcode::NetcodeSerializer _serializer;
    /** Deserializer for supporting parameters */
    cugl::netcode::NetcodeDeserializer _deserializer;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, Vec2 velocity, string pid);


public:
    /**
     * Allocates a new instance of the factory using the given AssetManager.
     */
    static shared_ptr<PlayerFactory> alloc(shared_ptr<AssetManager>& assets) {
        auto f = make_shared<PlayerFactory>();
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
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, Vec2 velocity, string pid);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;
};

#endif // !__PLAYER_FACTORY_H__

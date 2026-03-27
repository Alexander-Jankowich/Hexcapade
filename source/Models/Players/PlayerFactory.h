//
//  PlayerFactory.h
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#ifndef __PLAYER_FACTORY_H__
#define __PLAYER_FACTORY_H__

#include <cugl/cugl.h>
#include "PlayerNode.h"

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;


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
    


public:
    PlayerFactory();
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, string pid);
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
        PlayerNode::setConsts(_assets->get<JsonValue>("constants")->get("player"));
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, string pid);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;

    std::unordered_set<std::shared_ptr<PlayerObstacle>> current;


    std::shared_ptr<PlayerObstacle> getPlayerByPID(std::string pid) {
        for (auto p : current) {
            if (p->getPID() == pid) return p;
        }
        return nullptr; // should NOT get here
    }

    //void reset() {
    //    for (auto p : current) {
    //        p->reset();
    //    }
    //}
};

#endif // !__PLAYER_FACTORY_H__

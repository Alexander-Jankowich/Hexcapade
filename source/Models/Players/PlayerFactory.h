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

    static int rows;
    static int cols;


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
        auto consts = _assets->get<JsonValue>("constants")->get("player");
        PlayerNode::setConsts(consts);
        rows = consts->getInt("rows");
        cols = consts->getInt("columns");
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, string pid);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;

    /**
    * The linking function for player obstacles -> player nodes
    */
    void linkFunc(const shared_ptr<PlayerObstacle> obs, const shared_ptr<PlayerNode> node);

    unordered_set<shared_ptr<PlayerObstacle>> current;


    shared_ptr<PlayerObstacle> getPlayerByPID(string pid) {
        for (auto p : current) {
            CULog("i am trying to get a player");
            if (p->getPID() == pid) return p;
        }
        return nullptr; // should NOT get here
    }
    
    void addCurrent(shared_ptr<PlayerObstacle> p){
        current.emplace(p);
    }
    
    

    //void reset() {
    //    for (auto p : current) {
    //        p->reset();
    //    }
    //}
};

#endif // !__PLAYER_FACTORY_H__

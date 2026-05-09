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
#include "PlayerObstacle.h"
#include "../Animations.h"
#include "../Spells/SpellEntityNode.h"

#define PHYSICS_SCALE 50

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
    shared_ptr<AssetManager> _assets;
    shared_ptr<JsonValue> _spellConsts;

    /** Serializer for supporting parameters */
    netcode::NetcodeSerializer _serializer;
    /** Deserializer for supporting parameters */
    netcode::NetcodeDeserializer _deserializer;

    static int rows;
    static int cols;


public:
    PlayerFactory();
    unordered_set<shared_ptr<PlayerObstacle>> current;
    
    void reset() {current.clear();}
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, string pid, string textureKey);
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;
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
        PlayerNode::setConsts(_assets, consts);
        rows = consts->getInt("rows");
        cols = consts->getInt("columns");

        _spellConsts = _assets->get<JsonValue>("constants")->get("spells");
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, string pid, string textureKey = "player_fox");

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */

    /**
    * The linking function for player obstacles -> player nodes
    */
    void linkFunc(const shared_ptr<PlayerObstacle> obs, const shared_ptr<PlayerNode> node, shared_ptr<Animator> animations);

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
    
    std::vector<Vec2> getPlayerPositions() {
        std::vector<Vec2> res;
        for (auto p : current) res.push_back(p->getPosition() / PHYSICS_SCALE);
        return res;
    }
    
    

    //void reset() {
    //    for (auto p : current) {
    //        p->reset();
    //    }
    //}
};

#endif // !__PLAYER_FACTORY_H__

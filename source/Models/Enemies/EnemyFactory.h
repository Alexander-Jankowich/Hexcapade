//
//  EnemyFactory.h
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#ifndef __ENEMY_FACTORY_H__
#define __ENEMY_FACTORY_H__

#include <cugl/cugl.h>
#include "EnemyNode.h"

#define ATTACK_DELAY 1.3f

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;

enum class EnemyType {
    GHOST = 0
};

/**
* An extension of CapsuleObstacle to represent enemy collision objects.
*/
class EnemyObstacle : public physics2::CapsuleObstacle {
public:
    enum class EnemyState : int  {
        IDLE=0,
        CHASING=1,
        ATTACK_RESET=2
    };

protected:
    // Current of hp the enemy has
    int _health;

    // Max speed of the enemy
    float _speed;

    // Distance at which the enemy can see players
    float _range;

    // The target position the enemy is trying to go to
    Vec2 _target;

    // The current action/state the enemy is in
    EnemyState _state;

    // Time since the last attack
    int _attackCounter;

    // Whether the enemy is frozen
    bool _frozen;
    
    //Whether the enemy has mud status
    bool _mud;

    // The time left until enemy is unfrozen
    int _frozenTimer;
    
    //The time left until enemies mud wears off
    int _mudTimer;

public:
    EnemyObstacle(EnemyType type, const Vec2& pos, const shared_ptr<JsonValue> consts, Size s);

    // The type of enemy this enemy is, can be public bc it is a constant
    const EnemyType TYPE;

    static shared_ptr<EnemyObstacle> alloc(EnemyType type, const Vec2& pos, const shared_ptr<JsonValue> consts, Size s) {
        auto f = make_shared<EnemyObstacle>(type, pos, consts, s);
        f -> createFixtures();
        return f;
    }

    /**
    * Sets the acceleration of the enemy to reach the intended velocity
    * This method makes it so that the enemy will still collide with walls and players
    */
    void setVelocity(Vec2 vel);

    /**
     * Returns the speed of this enemy.
     *
     * @return the speed of this enemy
     */
    const float getSpeed() const { return _speed; }
    void setSpeed(float s) {_speed = s; }
    EnemyState getState() {return _state;}
    void setState(EnemyState s) {_state=s;}
    void updateAttackCounter();

    /**
     * Returns the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @return the current enemy health.
     */
    int getHealth() const { return _health; }
    
    bool isFroze () { return _frozen; }

    /**
     * Sets the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @param value The current enemy health.
     */
    void setHealth(int value);

    float getRange() { return _range; }

    void setTarget(const Vec2 targ) { _target = targ; _state = EnemyState::CHASING;}

    Vec2 getTarget() { return _target; }
    
    void setFrozenTimer(int i) { _frozenTimer = i; }
    
    int getFrozenTimer() { return _frozenTimer; }
    
    void setFrozen(bool b) { _frozen = b; }
    
    void setMud(bool m) { _mud = m; }
    
    bool isMud () { return _mud; }
    
    int getMudTimer() { return _mudTimer; }
    
    void setMudTimer(float t) { _mudTimer = t; }
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
    * Returns a set of enemies that have been flagged for garbage collection and removes them from
    * the current enemy set. Should be called every update frame
    * 
    * @return a set of all enemies to be removed from the world
    */
    std::unordered_set<std::shared_ptr<EnemyObstacle>> getRemoved();
};

#endif // !__ENEMY_FACTORY_H__

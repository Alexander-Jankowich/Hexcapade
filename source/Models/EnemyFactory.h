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

#define ATTACK_DELAY 1.5f

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;

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
    int _health;
    float _speed;
    float _range;
    Vec2 _target;
    EnemyState _state;
    int _attackCounter;

public:
    EnemyObstacle(const cugl::Vec2& pos, float speed, int health, float range, cugl::Size s);

    static shared_ptr<EnemyObstacle> alloc(const cugl::Vec2& pos, float speed, int health, float range, cugl::Size s) {
        auto f = make_shared<EnemyObstacle>(pos, speed, health, range, s);
        f -> createFixtures();
        return f;
    }

    /**
     * Returns the speed of this enemy.
     *
     * @return the speed of this enemy
     */
    const float getSpeed() const { return _speed; }
    
    EnemyState getState() {return _state;}
    void setState(EnemyState s) {_state=s;}
    void updateAttackCounter();

    /**
     * Returns the current ship health.
     *
     * When the health of the ship is 0, it is "dead"
     *
     * @return the current ship health.
     */
    int getHealth() const { return _health; }

    /**
     * Sets the current ship health.
     *
     * When the health of the ship is 0, it is "dead"
     *
     * @param value The current ship health.
     */
    void setHealth(int value);

    float getRange() { return _range; }

    void setTarget(const Vec2 targ) { _target = targ; _state = EnemyState::CHASING;}

    Vec2 getTarget() { return _target; }
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

    Vec2 getClosestPlayer(Vec2 enemPos, std::vector<Vec2> targs);

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
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    shared_ptr<vector<byte>> serializeParams(Vec2 pos, Vec2 velocity, int h);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> createObstacle(const vector<byte>& params) override;

    /**
    * Updates all enemies to go to nearest target
    * 
    * TODO: change to a proper AI
    * 
    * @return a set of all enemies to be removed from the world
    */
    std::unordered_set<std::shared_ptr<EnemyObstacle>> update(std::vector<cugl::Vec2> targets);
};

#endif // !__ENEMY_FACTORY_H__

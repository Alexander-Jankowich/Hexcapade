//  
//  EnemyObstacle.h
//  hexc
//
//  Created by Kaz Tam on 4/12/26.
//

#ifndef __ENEMY_OBSTACLE_H__
#define __ENEMY_OBSTACLE_H__

#include <cugl/cugl.h>

using namespace cugl;
using namespace std;

enum class EnemyType {
    GHOST = 0,
    BAT = 1,
    OCTO = 2
};

/**
* An extension of CapsuleObstacle to represent enemy collision objects.
*/
class EnemyObstacle : public physics2::CapsuleObstacle {
public:
    enum class EnemyState : int {
        IDLE = 0,
        CHASING = 1,
        ATTACK_RESET = 2
    };

protected:
    // Current of hp the enemy has
    int _health;

    // Max speed of the enemy
    float _speed;

    // The max length of the enemy from its center
    float _radius;

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

    Vec2 _waterPushDir = Vec2::ZERO;
    int _waterPushTimer = 0;
    float _attackRange = 0;
    int _attackTimer = 0;



public:
    EnemyObstacle(const EnemyType type, const Vec2& pos, const shared_ptr<JsonValue>& consts, Size s);

    // The type of enemy this enemy is, can be public bc it is a constant
    const EnemyType TYPE;

    static shared_ptr<EnemyObstacle> alloc(EnemyType type, const Vec2& pos, const shared_ptr<JsonValue> consts, Size s) {
        auto f = make_shared<EnemyObstacle>(type, pos, consts, s);
        f->createFixtures();
        return f;
    }

    /**
    * Sets the acceleration of the enemy to reach the intended velocity
    * This method makes it so that the enemy will still collide with walls and players
    */
    void setVelocity(Vec2 vel);

    float getRadius() { return _radius; }

    /**
     * Returns the speed of this enemy.
     *
     * @return the speed of this enemy
     */
    const float getSpeed() const { return _speed; }
    void setSpeed(float s) { _speed = s; }
    EnemyState getState() { return _state; }
    void setState(EnemyState s) { _state = s; }
    void updateAttackCounter();
    void setAttackCounter(int frames) { _attackCounter = frames; }
    bool canAttack() { return _attackCounter <= 0; }

    /**
     * Returns the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @return the current enemy health.
     */
    int getHealth() const { return _health; }

    bool isFroze() { return _frozen; }

    /**
     * Sets the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @param value The current enemy health.
     */
    void setHealth(int value);

    float getRange() { return _range; }

    void setTarget(const Vec2 targ) { _target = targ; _state = EnemyState::CHASING; }

    Vec2 getTarget() { return _target; }

    void setFrozenTimer(int i) { _frozenTimer = i; }

    int getFrozenTimer() { return _frozenTimer; }

    void setFrozen(bool b) { _frozen = b; }

    void setMud(bool m) { _mud = m; }

    bool isMud() { return _mud; }

    int getMudTimer() { return _mudTimer; }

    void setMudTimer(float t) { _mudTimer = t; }

    void setWaterPush(Vec2 dir, int frames) { _waterPushDir = dir; _waterPushTimer = frames; }
    Vec2 getWaterPushDir() const { return _waterPushDir; }
    int getWaterPushTimer() const { return _waterPushTimer; }
    void tickWaterPush() { if (_waterPushTimer > 0) _waterPushTimer--; }
    
    float getAttackRange() const { return _attackRange; }
    int getAttackTimer() const { return _attackTimer;}
    void setAttackTimer(int a) { _attackTimer = a; }
    
};

#endif

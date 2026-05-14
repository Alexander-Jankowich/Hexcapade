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
    OCTO = 2,
    SLOWBAT = 3
};

/**
* An extension of CapsuleObstacle to represent enemy collision objects.
*/
class EnemyObstacle : public physics2::CapsuleObstacle {
public:
    enum class EnemyState : int {
        REGULAR = 0,
        ATTACKING = 2,
        HURT = 3,
        DEATH = 4
    };

    const uint32 ID;
    
protected:
    // Current of hp the enemy has
    float _health;

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

    // Attack function
    ActionListener _attackFunc;
    float _attackFuncTime;

    // Whether the enemy is frozen
    bool _frozen;
    // The time left until enemy is unfrozen
    int _frozenTimer;

    //Whether the enemy has mud status
    bool _mud;
    //The time left until enemies mud wears off
    int _mudTimer;

    int _hurtTimer;

    Vec2 _waterPushDir = Vec2::ZERO;
    int _waterPushTimer = 0;
    float _attackRange = 0;
    int _attackTimer = 0;

    bool _propertiesDirty;

public:
    EnemyObstacle(const EnemyType type, uint32 id, const Vec2& pos, const shared_ptr<JsonValue>& consts, Size s);

    // The type of enemy this enemy is, can be public bc it is a constant
    const EnemyType TYPE;

    static shared_ptr<EnemyObstacle> alloc(EnemyType type, uint32 id, const Vec2& pos, const shared_ptr<JsonValue> consts, Size s) {
        auto f = make_shared<EnemyObstacle>(type, id, pos, consts, s);
        f->createFixtures();
        return f;
    }

#pragma mark Properties

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
    void setState(EnemyState s);

    void startAttack() { setState(EnemyState::ATTACKING); }
    bool isAttacking() { return _state == EnemyState::ATTACKING; }

    void setAttackFunc(float t, ActionListener listener) { _attackFuncTime = t; _attackFunc = listener; }
    ActionListener getAttack() { return _attackFunc; }
    float getAttackTiming() { return _attackFuncTime; }

    void updateAttackCounter();
    void setAttackTimer(int frames) { _attackCounter = frames; }
    bool canAttack() { return _attackCounter <= 0; }
    int getAttackTimer() const { return _attackTimer; }

    bool isDead() { return _state == EnemyState::DEATH; }

    /**
     * Returns the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @return the current enemy health.
     */
    float getHealth() const { return _health; }

    /**
     * Sets the current enemy health.
     *
     * When the health of the enemy is 0, it is "dead"
     *
     * @param value The current enemy health.
     */
    void setHealth(float value);

    float getRange() { return _range; }

    void setTarget(const Vec2 targ) { _target = targ; }
    Vec2 getTarget() { return _target; }

    void setFrozenTimer(int i) { _frozenTimer = i; }
    int getFrozenTimer() { return _frozenTimer; }

    bool isFroze() { return _frozen; }
    void setFrozen(bool b);

    void setMud(bool m) { _mud = m; }
    bool isMud() { return _mud; }

    int getMudTimer() { return _mudTimer; }
    void setMudTimer(float t) { _mudTimer = t; }

    void setWaterPush(Vec2 dir, int frames) { _waterPushDir = dir; _waterPushTimer = frames; }
    Vec2 getWaterPushDir() const { return _waterPushDir; }
    int getWaterPushTimer() const { return _waterPushTimer; }
    void tickWaterPush() { if (_waterPushTimer > 0) _waterPushTimer--; }
    
    float getAttackRange() const { return _attackRange; }

    void setHurt(bool h);
    bool isHurt() { return _state == EnemyState::HURT; }
    void updateHurtTimer();

    void setPropertiesDirty(bool d) { _propertiesDirty = d; }
    bool isProperiesDirty() { return _propertiesDirty; }
};

#endif // !__ENEMY_OBSTACLE_H__

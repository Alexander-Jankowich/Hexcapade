//  
//  EnemyObstacle.cpp
//  hexc
//
//  Created by Kaz Tam on 4/12/26.
//

#include "EnemyObstacle.h"

#define PHYSICS_SCALE 50
#define ATTACK_DELAY 90.0f

EnemyObstacle::EnemyObstacle(const EnemyType type, const Vec2& pos, const shared_ptr<JsonValue>& consts, Size s)
    : TYPE(type) {
    init(pos, s / (2 * PHYSICS_SCALE));
    setAwake(true);
    setEnabled(true);
    setBodyType(b2_dynamicBody);
    setDensity(1.0f);
    setFixedRotation(true);

    _health = consts->getInt("health");
    _speed = consts->getFloat("speed");
    _range = consts->getFloat("range");
    _attackRange = consts -> getFloat("attack_range");
    _target = pos;
    _radius = Vec2(s.height / (2 * PHYSICS_SCALE), s.width / (2 * PHYSICS_SCALE)).length() / 2;

    _state = EnemyState::IDLE;
    _frozen = false;
    _mud = false;
    _mudTimer = 0;
    _frozenTimer = 0;
    _attackCounter = 0;
}

/**
 * Sets the current enemy health.
 *
 * When the health of the enemy is 0, it is "dead"
 *
 * @param value The current enemy health.
 */
void EnemyObstacle::setHealth(int value) {
    if (value <= 0) {
        _health = 0;
        _state = EnemyState::DEATH;
    }
    else {
        _health = value;
    }
}

/**
 * Tracks this enemy's "reset" window after an attack. Once the time is up, sets enemy back to IDLE.
 */
void EnemyObstacle::updateAttackCounter() {
    if (_attackCounter > 0) {
        _attackCounter--;
        //CULog("attack_counter %d", _attackCounter);
    }
    else {
        _attackCounter = 0;
        _state = EnemyState::IDLE;
    }
}

void EnemyObstacle::setVelocity(Vec2 v) {
    if (!getBody()) return; // body not ready yet

    //CULog("Setting velocity to %.2f, %.2f", v.x, v.y);
    b2Vec2 currentVel = getBody()->GetLinearVelocity();
    b2Vec2 targetVel = b2Vec2(v.x, v.y);
    b2Vec2 velDiff = targetVel - currentVel;
    float mass = getBody()->GetMass();
    float scale = mass * 60.0f;
    b2Vec2 force = b2Vec2(velDiff.x * scale, velDiff.y * scale);
    getBody()->ApplyForceToCenter(force, true);
}

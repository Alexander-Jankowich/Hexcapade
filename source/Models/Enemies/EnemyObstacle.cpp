//  
//  EnemyObstacle.cpp
//  hexc
//
//  Created by Kaz Tam on 4/12/26.
//

#include "EnemyObstacle.h"

#define PHYSICS_SCALE 100

EnemyObstacle::EnemyObstacle(const EnemyType type, const Vec2& pos, const shared_ptr<JsonValue> consts, Size s)
    : TYPE(type) {
    init(pos, s / PHYSICS_SCALE);
    setAwake(true);
    setEnabled(true);
    setBodyType(b2_dynamicBody);
    setDensity(1.0f);
    setFixedRotation(true);

    shared_ptr<JsonValue> vals;
    switch (type) {
    case EnemyType::GHOST:
        vals = consts->get("ghost");
        break;
    default:
        CULog("Error! Enemy type not found in JSON!!!");
        break;
    }

    _health = vals->getInt("health");
    _speed = vals->getFloat("speed");
    _range = vals->getFloat("range");
    _target = pos;

    _state = EnemyState::IDLE;
    _frozen = false;
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
    _health = value >= 0 ? value : 0;
}

/**
 * Tracks this enemy's "reset" window after an attack. Once the time is up, sets enemy back to IDLE.
 */
void EnemyObstacle::updateAttackCounter() {
    if (_attackCounter < ATTACK_DELAY) _attackCounter++;
    else {
        _attackCounter = 0;
        _state = EnemyState::IDLE;
    }
    //CULog("attack counter: %d", _attackCounter);
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
    CULog("mass: %f, force: %f %f", mass, force.x, force.y);
    getBody()->ApplyForceToCenter(force, true);
}
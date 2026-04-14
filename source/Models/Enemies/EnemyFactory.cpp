//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"
#define PHYSICS_SCALE 100
#define FROZEN_FRAMES 60

using namespace std;
using namespace cugl;

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
    _health = value >= 0 ? value : 0;
}

/**
 * Tracks this enemy's "reset" window after an attack. Once the time is up, sets enemy back to IDLE.
 */
void EnemyObstacle::updateAttackCounter() {
    if (_attackCounter < ATTACK_DELAY) _attackCounter++;
    else {
        _attackCounter=0;
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
    getBody()->ApplyForceToCenter(force, true);
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(EnemyType type, Vec2 pos) {
    
    auto image = _assets->get<graphics::Texture>("enemy"); // TODO this will need to update with additional metadata (which asset to get)
    Size size = image->getSize();
    
    // Create the EnemyObstacle

    auto en = EnemyObstacle::alloc(type, pos, _enemyConsts, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(Vec2::ZERO);
    // which of those are needed...

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = make_shared<EnemyNode>(image);
    sprite -> setScale(0.5);

    return std::make_pair(en, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> EnemyFactory::serializeParams(EnemyType type, Vec2 pos) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeUint16((uint16) type);
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    return make_shared<vector<byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(const vector<byte>& params) {
    /**
     * TODO: Use _deserializer to deserialize byte vectors packed by
     * {@link serializeParams()} and call the regular createObstacle() method
     * with them.
     */
    _deserializer.reset();
    _deserializer.receive(params);
    EnemyType type = (EnemyType) _deserializer.readUint16();
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    return createObstacle(type, pos);
}


std::unordered_set<std::shared_ptr<EnemyObstacle>> EnemyFactory::getRemoved() {
    std::unordered_set<std::shared_ptr<EnemyObstacle>> remove;

    for (auto it = current.begin(); it != current.end(); ) {
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
        }
        else {
            it++;
        }
    }

    return remove;
}

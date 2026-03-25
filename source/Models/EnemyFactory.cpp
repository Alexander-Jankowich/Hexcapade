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

EnemyObstacle::EnemyObstacle(const cugl::Vec2& pos, float speed, int health, float range, cugl::Size s) {
    init(pos, s / PHYSICS_SCALE);
    setAwake(true);
    setEnabled(true);
    setBodyType(b2_dynamicBody);

    _health = health;
    _speed = speed;
    _range = range;
    _target = pos;
    _state = EnemyState::IDLE;
    _frozen = false;
    _frozenTimer = 0;
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
    CULog("attack counter: %d", _attackCounter);
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(Vec2 pos, Vec2 velocity, int h) {
    
    auto image = _assets->get<graphics::Texture>("enemy"); // TODO this will need to update with additional metadata (which asset to get)
    Size size = image->getSize();
    
    // Create the SpellObstacle
    auto en = EnemyObstacle::alloc(pos, 2.5, h,_assets->get<JsonValue>("constants")->get("enemies")->getFloat("range"), size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(velocity);
    // which of those are needed...

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = make_shared<Enemy>(pos, 2.5, h, image);
    sprite -> setScale(0.5);

    return std::make_pair(en, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> EnemyFactory::serializeParams(Vec2 pos, Vec2 velocity, int h) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeFloat(velocity.x);
    _serializer.writeFloat(velocity.y);
    _serializer.writeUint32(h);
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
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    float vx = _deserializer.readFloat();
    float vy = _deserializer.readFloat();
    Vec2 velocity = Vec2(vx, vy);
    int health =  _deserializer.readUint32();
    return createObstacle(pos, velocity, health);
}

#pragma mark Enemy Behavior

Vec2 EnemyFactory::getClosestPlayer(Vec2 enemPos, std::vector<Vec2> targs) {
    float minDist = FLT_MAX;
    Vec2 minPos = Vec2(0, 0); // default
    for (auto t : targs) {
        float d = enemPos.distance(t);
        if (d < minDist) {
            minDist = d;
            minPos = t;
        }
    }
    return minPos;
}

std::unordered_set<std::shared_ptr<EnemyObstacle>> EnemyFactory::update(std::vector<Vec2> targets) {
    std::unordered_set<std::shared_ptr<EnemyObstacle>> remove;

    for (auto it = current.begin(); it != current.end(); ) {
        // Remove from set if removed from scene
        if ((*it) -> isFroze()) {
            (*it)->setLinearVelocity(Vec2::ZERO);
            CULog("i am frozen af");
            int i = (*it) -> getFrozenTimer() + 1;
            (*it) -> setFrozenTimer(i);
            if ((*it) -> getFrozenTimer() >= FROZEN_FRAMES) {
                (*it) -> setFrozen(false);
                (*it) -> setFrozenTimer(0);
            }
            it++;
            continue;
        }
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
            continue;
        }
        
        if((*it)->getState()==EnemyObstacle::EnemyState::ATTACK_RESET) (*it)->updateAttackCounter();
        else {
            // Update individual enemies
            Vec2 targ = getClosestPlayer((*it)->getPosition(), targets);

            if (targ.distance((*it)->getPosition()) < (*it)->getRange()) {
                (*it)->setTarget(targ);
                //CULog("%.2f", targ.distance((*it)->getPosition()));
            }
            // if chasing
            if ((*it)->getState()==EnemyObstacle::EnemyState::CHASING) {
                // Set velocity to go to target
                Vec2 currentVel = (*it)->getLinearVelocity();

                Vec2 desiredVel = (*it)->getTarget() - (*it)->getPosition();
                desiredVel.normalize();
                desiredVel *= (*it)->getSpeed();

                float recovery = 0.2f;
                Vec2 newVel = currentVel * (1 - recovery) + desiredVel * recovery;

                (*it)->setLinearVelocity(newVel);
            }
        }
        it++;
    }

    return remove;
}

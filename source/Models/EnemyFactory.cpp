//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"
#define PHYSICS_SCALE 100

using namespace std;
using namespace cugl;

EnemyObstacle::EnemyObstacle(const cugl::Vec2& pos, float speed, int health, cugl::Size s) {
    init(pos, s / PHYSICS_SCALE);
    setAwake(true);
    setEnabled(true);
    setBodyType(b2_dynamicBody);
    _health = health;
    _speed = speed;
}

/**
 * Sets the current ship health.
 *
 * When the health of the ship is 0, it is "dead"
 *
 * @param value The current ship health.
 */
void EnemyObstacle::setHealth(int value) {
    if (value >= 0) {
        // Do not allow health to go negative
        _health = value;
    }
    else {
        _health = 0;
    }
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(Vec2 pos, Vec2 velocity, int h) {
    
    auto image = _assets->get<graphics::Texture>("enemy"); // TODO this will need to update with additional metadata (which asset to get)
    Size size = image->getSize();
    
    // Create the SpellObstacle
    auto en = EnemyObstacle::alloc(pos, 2.5, h, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(velocity);
    // which of those are needed...

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = make_shared<Enemy>(pos, 2.5, h, image);

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
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
            continue;
        }

        // Update individual enemies
        Vec2 targ = getClosestPlayer((*it)->getPosition(), targets);

        Vec2 vel = targ - (*it)->getPosition();
        vel.normalize();
        vel *= (*it)->getSpeed();
        (*it)->setLinearVelocity(vel);


        // delete enemies if they fall below 0 hp.
//        if ((*it)->getHealth() > 0) {
//            it++;
//        }
//        else {
//            remove.emplace(*it);
//            it = current.erase(it);
//        }

    }

    return remove;
}

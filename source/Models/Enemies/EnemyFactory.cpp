//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"

using namespace std;
using namespace cugl;

#define PHYSICS_SCALE 100

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(EnemyType type, Vec2 pos) {

    shared_ptr<JsonValue> vals;
    switch (type) {
    case EnemyType::GHOST:
        vals = _enemyConsts->get("ghost");
        break;
    case EnemyType::BAT:
        vals = _enemyConsts->get("bat");
        break;
    case EnemyType::OCTO:
        vals = _enemyConsts->get("octoplush");
        break;
    default:
        CULog("Error! Enemy type not found in JSON!!!");
        break;
    }

    auto img = _assets->get<graphics::Texture>(vals->getString("texture"));
    Size size = img->getSize() / 2;
    size.width /= vals->getInt("columns");
    size.height /= vals->getInt("rows");
    
    // Create the EnemyObstacle
    auto en = EnemyObstacle::alloc(type, pos, vals, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(Vec2::ZERO);

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = make_shared<EnemyNode>(img, vals);
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


void EnemyFactory::linkFunc(const shared_ptr<EnemyObstacle> obs, const shared_ptr<EnemyNode> node) {
    obs->updateAttackCounter();

    if (obs->getWaterPushTimer() > 0) {
        obs->tickWaterPush();
        Vec2 pushVel = obs->getWaterPushDir() * obs->getSpeed() * 2.5f;
        obs->setVelocity(pushVel);
    }
    else if (obs->isFroze()) {
        obs->setLinearVelocity(Vec2::ZERO);
        int i = obs->getFrozenTimer() - 1;
        obs->setFrozenTimer(i);
        if (obs->getFrozenTimer() == 0) {
            obs->setFrozen(false);
        }
    }

    // Update state for animation
    node->setFrozen(obs->isFroze());
    node->setStatue(obs->isMud());
    node->updateState();
    node->updateAnimationState(obs->getState(), obs->getHealth());

    // Update position
    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
    node->setPosition(pos * PHYSICS_SCALE);
    node->setAngle(angle);
    node->setVelocity(obs->getLinearVelocity() * PHYSICS_SCALE);
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

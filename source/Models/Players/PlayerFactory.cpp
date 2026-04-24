//
//  PlayerFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//
#include "PlayerFactory.h"
using namespace std;
using namespace cugl;


#define PHYSICS_SCALE 50

int PlayerFactory::rows = 1;
int PlayerFactory::cols = 1;

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> PlayerFactory::createObstacle(Vec2 pos, string pid) {
    
    auto image = _assets->get<graphics::Texture>("player"); // TODO this will need to update with additional metadata (which asset to get)
    auto size = image->getSize();
    size.width /= cols;
    size.height /= rows;
    
    // Create the PlayerObstacle
    auto pl = PlayerObstacle::alloc(pos, pid, size);
    pl->setShared(true);

    current.emplace(pl);

    // Create player
    auto sprite = std::make_shared<PlayerNode>(image);
    sprite -> setScale(0.5);

    return std::make_pair(pl, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> PlayerFactory::serializeParams(Vec2 pos, string pid) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeString(pid);
    return make_shared<vector<byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> PlayerFactory::createObstacle(const vector<byte>& params) {
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
    string id =  _deserializer.readString(); // this is why i had to switch just this class to netcode de/serializers sorry abt that - serena
    return createObstacle(pos, id);
}

void PlayerFactory::linkFunc(const shared_ptr<PlayerObstacle> obs, const shared_ptr<PlayerNode> node, shared_ptr<Animator> animations) {
    if (obs->getHealth() <= 0 || obs->isFrozen()) {
        obs->setLinearVelocity(Vec2::ZERO);
    }

    if (obs->hasReleasedSpell()) {
        obs->setReleasedSpell(false);
        node->releasedSpell();
    }

    // Update frozen status for node if changed
    if (obs->isFrozen() && !node->isFrozen()) {
        animations->pauseAnimation(node);
    }
    else if (!obs->isFrozen() && node->isFrozen()) {
        animations->resumeAnimation(node);
    }
       
    node->updateAnimationState(obs->getHealth(), obs->isCasting(), obs->isFrozen(), obs->getMud(), obs->getBuff());
    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
    node->setPosition(pos * PHYSICS_SCALE);
    node->setAngle(angle);
    node->setVelocity(obs->getLinearVelocity() * PHYSICS_SCALE);
}

PlayerFactory::PlayerFactory() {
    current.clear();
}

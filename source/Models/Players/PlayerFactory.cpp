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
#define SHIELD_ID 1

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

    // Check shield status, update the child node if needed
    if (obs->hasShield() && !node->hasShield()) {
        // Just got shielded
        node->setShield(true);
        auto shield = make_shared<SpellEntityNode>(_assets->get<graphics::Texture>("shield_entity"), _spellConsts->get("shield-entity"));
        node->addChildWithTag(shield, SHIELD_ID);
        shield->setPriority(3);

        animations->addNode(shield);
    }
    else if (!obs->hasShield() && node->hasShield()) {
        CULog("Lost shield");
        node->setShield(false);
        auto shield = dynamic_pointer_cast<SpellEntityNode>(node->getChildByTag(SHIELD_ID));
        shield->end();
        animations->addNextListener(shield, [node, shield](string key, float time, float actual) {
            node->removeChild(shield);
        });
        animations->endAnimation(shield);
    }
       
    node->updateAnimationState(obs->getHealth(), obs->isHurt(), obs->isCasting(), obs->isFrozen(), obs->getMud(), obs->getBuff());

    if (obs->getHealth() <= 0) {
        animations->resumeAnimation(node);
    }
    if (obs->isHurt()) {
        // Remove hurt state when animation finishes playing
        animations->addNextListener(node, [obs](string key, float time, float actual) {
            obs->setHurt(false);
            CULog("not hurt now");
        });
    }

    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
    node->setPosition(pos * PHYSICS_SCALE);
    node->setAngle(angle);
    node->setVelocity(obs->getLinearVelocity() * PHYSICS_SCALE);

    if (node->skipToNext()) {
        animations->endAnimation(node);
    }
}

PlayerFactory::PlayerFactory() {
    current.clear();
}

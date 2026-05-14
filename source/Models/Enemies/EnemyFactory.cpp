//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"
#include "OctoplushNode.h"

using namespace std;
using namespace cugl;

#define PHYSICS_SCALE 50
#define FROZEN_ID 3

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(EnemyType type, uint32 id, Vec2 pos) {

    shared_ptr<JsonValue> vals;
    switch (type) {
    case EnemyType::GHOST:
        vals = _enemyConsts->get("ghost");
        break;
    case EnemyType::BAT:
        vals = _enemyConsts->get("bat");
        break;
    case EnemyType::SLOWBAT:
        vals = _enemyConsts->get("slow-bat");
        break;
    case EnemyType::OCTO:
        vals = _enemyConsts->get("octoplush");
        break;
    default:
        CULog("Error! Enemy type not found in JSON!!! EnemyFactory::createObstacle, type = %d", type);
        break;
    }

    auto img = _assets->get<graphics::Texture>(vals->getString("texture"));
    Size size = img->getSize() / 2;
    size.width /= vals->getInt("columns");
    size.height /= vals->getInt("rows");
    
    // Create the EnemyObstacle
    auto en = EnemyObstacle::alloc(type, id, pos, vals, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(Vec2::ZERO);

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = (type == EnemyType::OCTO) ? make_shared<OctoplushNode>(img, vals, _assets->get<graphics::Texture>(vals->getString("texture")+"_attack"))
        : make_shared<EnemyNode>(img, vals);
    sprite -> setScale(0.5);

    return std::make_pair(en, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> EnemyFactory::serializeParams(EnemyType type, Vec2 pos) {
    _serializer.reset();
    _serializer.writeUint16((uint16) type);
    _serializer.writeUint32(_nextId);
    _nextId++;
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    return make_shared<vector<byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(const vector<byte>& params) {
    _deserializer.reset();
    _deserializer.receive(params);
    EnemyType type = (EnemyType) _deserializer.readUint16();
    uint32 id = _deserializer.readUint32();
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    return createObstacle(type, id, pos);
}


void EnemyFactory::linkFunc(const shared_ptr<EnemyObstacle> obs, const shared_ptr<EnemyNode> node, bool isHost, const shared_ptr<Animator>& animator) {
    if (isHost) {
        // Only update timers and velocity if host

        if (obs->isDead()) {
            obs->setLinearVelocity(Vec2::ZERO);
            obs->setFrozenTimer(0);
            obs->setFrozen(false);
        }
        else {
            // If not dead, update attributes
            obs->updateAttackCounter();
            obs->updateHurtTimer();

            // Update push and frozen
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
        }
    }

    // Update frozen status for node if changed
    if (obs->isFroze() && !node->getFrozen()) {
        node->setFrozen(true);
        animator->pauseAnimation(node);

        auto frozen = make_shared<SpellEntityNode>(_assets->get<graphics::Texture>("freeze_entity"), _spellConsts->get("freeze-entity"));
        node->addChildWithTag(frozen, FROZEN_ID);
        frozen->setPriority(3);
        //frozen->setScale(node->bounds().size.width / frozen->bounds().size.width);
        //CULog("%.2f", frozen->getScale());

        animator->addNode(frozen);

        animator->addNextListener(frozen, 90 / FPS, [node, frozen](string key, float time, float actual) {
            node->setTint(Color4(0, 106, 209, 255));
        });
    }
    else if (!obs->isFroze() && node->getFrozen()) {
        node->setFrozen(false);
        animator->resumeAnimation(node);

        auto frozen = dynamic_pointer_cast<SpellEntityNode>(node->getChildByTag(FROZEN_ID));
        frozen->end();
        animator->addNextListener(frozen, [node, frozen](string key, float time, float actual) {
            node->removeChild(frozen);
        });

        animator->addNextListener(frozen, 90 / FPS, [node, frozen](string key, float time, float actual) {
            node->setTint(Color4::WHITE);
        });
        animator->endAnimation(frozen);
        
        obs->setHurt(false);
    }

    if (obs->isHurt() && !node->isHurt()) {
        obs->setLinearVelocity(Vec2::ZERO);
        //CULog("do hurt animation");
        //animator->addNextListener(node, [obs, node](string key, float time, float actual) {
        //    obs->setHurt(false);
        //    node->setHurt(false);
        //    //CULog("not hurt now");
        //});
        animator->endAnimation(node);
    }

    if (obs->isDead() && !node->isDead()) {
        obs->setLinearVelocity(Vec2::ZERO);
        animator->addNextListener(node, [obs](string key, float time, float actual) {
            obs->markRemoved(true);
        });
        animator->resumeAnimation(node);
        animator->endAnimation(node);
    }

    if (obs->isAttacking() && !node->isAttacking()) {
        //CULog("do attack thing");
        obs->setLinearVelocity(Vec2::ZERO);
        animator->addNextListener(node, obs->getAttackTiming(), obs->getAttack());
        animator->addNextListener(node, [obs, node](string key, float time, float actual) {
            obs->setState(EnemyObstacle::EnemyState::REGULAR);
            node->updateAnimationState(obs->getState());
        });
        animator->endAnimation(node);
    }

    // Update position
    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
    node->setPosition(pos * PHYSICS_SCALE);
    node->setAngle(angle);
    node->setVelocity(obs->getLinearVelocity() * PHYSICS_SCALE);

    // Update state for animation
    node->setStatue(obs->isMud());
    node->updateAnimationState(obs->getState());
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

//
//  Player.cpp
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#include "PlayerNode.h"

using namespace cugl;
using namespace std;
using namespace cugl::graphics;

int PlayerNode::rows = 1;
int PlayerNode::cols = 1;

int PlayerNode::idle[4] = { 0,0,0,0 };
int PlayerNode::walk[4] = { 0,0,0,0 };
int PlayerNode::startCast[4] = { 0,0,0,0 };
int PlayerNode::casting[4] = { 0,0,0,0 };
int PlayerNode::endCast[4] = { 0,0,0,0 };
int PlayerNode::releaseCast[4] = { 0,0,0,0 };
int PlayerNode::damage[4] = { 0,0,0,0 };
int PlayerNode::dying[4] = { 0,0,0,0 };
int PlayerNode::dead[4] = { 0,0,0,0 };

int PlayerNode::idleFrames = 0;
int PlayerNode::walkFrames = 0;
int PlayerNode::startCastFrames = 0;
int PlayerNode::castingFrames = 0;
int PlayerNode::endCastFrames = 0;
int PlayerNode::releaseCastFrames = 0;
int PlayerNode::damageFrames = 0;
int PlayerNode::dyingFrames = 0;
int PlayerNode::deadFrames = 0;

float PlayerNode::idleThreshold = 0.0;

PlayerNode::PlayerNode(const std::shared_ptr<cugl::graphics::Texture>& tex) {
    initWithSheet(tex, rows, cols);

    _velocity = Vec2::ZERO;
    facingBack = false;
    facingLeft = false;

    _curAnimation = AnimationType::IDLE;
    _nextAnimation = AnimationType::IDLE;

    setFrame(idle[0]);
    setPriority(1);
}

void PlayerNode::setVelocity(cugl::Vec2 value) {
    _velocity = value;
    if (fabs(_velocity.x) > idleThreshold) {
        facingLeft = _velocity.x < 0;
    }
    if (fabs(_velocity.y) > idleThreshold) {
        facingBack = _velocity.y > 0;
    }
}

void PlayerNode::updateAnimationState(int health, bool casting, bool m) {
    CULog("player animation state is: %d", _curAnimation);
    _mud = m;

    // Just get direction
    if (_frozen) {
        setColor(Color4::BLUE);
    }
    else if (_mud){
        setColor(Color4::GRAY);
    }
    else{
        setColor(Color4::WHITE);
    }

    if (health <= 0) {
        _nextAnimation = AnimationType::DEAD;
    }
    else if (_hurt && _curAnimation != AnimationType::DAMAGED) {
        _nextAnimation = AnimationType::DAMAGED;
    }
    else if (casting) {
        _nextAnimation = AnimationType::CASTING;
    }
    else if ((_nextAnimation != AnimationType::RELEASE_CAST || _curAnimation == AnimationType::RELEASE_CAST)) {
        _nextAnimation = _velocity.length() < idleThreshold ? AnimationType::IDLE : AnimationType::WALK;
    }
}

pair<ActionFunction, float> PlayerNode::getNextAnimation() {
    auto cur = _curAnimation;
    _curAnimation = _nextAnimation;

    switch (_nextAnimation) {
    case AnimationType::IDLE:
        if (cur == AnimationType::CASTING) {
            return getAnimationPair(endCast, endCastFrames);
        }
        return getAnimationPair(idle, idleFrames);
    case AnimationType::WALK:
        if (cur == AnimationType::CASTING) {
            return getAnimationPair(endCast, endCastFrames);
        }
        return getAnimationPair(walk, walkFrames);
    case AnimationType::CASTING:
        if (cur != AnimationType::CASTING) {
            return getAnimationPair(startCast, startCastFrames);
        }
        return getAnimationPair(casting, castingFrames);
    case AnimationType::RELEASE_CAST:
        return getAnimationPair(releaseCast, releaseCastFrames);
    case AnimationType::DAMAGED:
        return getAnimationPair(damage, damageFrames);
    case AnimationType::DEAD:
        if (cur == AnimationType::CASTING) {
            // This is to kinda cheese it and have it do the dying animation too
            _nextAnimation = AnimationType::IDLE;

            return getAnimationPair(endCast, endCastFrames);
        }
        if (cur != AnimationType::DEAD) {
            return getAnimationPair(dying, dyingFrames);
        }
        return getAnimationPair(dead, deadFrames);
    default:
        CULog("Animation for this state %d has not been implemented! (PlayerNode::getNextAnimation)", _nextAnimation);
        return END_ANIMATION;
    }
}

bool PlayerNode::skipToNext() {
    return (_curAnimation == AnimationType::IDLE && _nextAnimation != AnimationType::IDLE) ||
        (_curAnimation == AnimationType::CASTING && _nextAnimation != AnimationType::CASTING) ||
        (_nextAnimation == AnimationType::DEAD && _curAnimation != AnimationType::DEAD) ||
        (_nextAnimation == AnimationType::DAMAGED && _curAnimation != AnimationType::DAMAGED);
}

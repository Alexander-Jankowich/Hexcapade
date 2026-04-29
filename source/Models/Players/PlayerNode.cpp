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

#define FPS 10.0f

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

void PlayerNode::updateAnimationState(int health, bool hurt, bool casting, bool f, bool m, bool b, bool s) {
    _frozen = f;
    _mud = m;
    _buff = b;
    _shield = s;

    // Just get direction
    if (_frozen) {
        setColor(Color4::BLUE);
    }
    else if (_mud){
        setColor(Color4::GRAY);
    }
    else if (_buff){
        CULog("coloring red");  
        setColor(Color4::RED);
    }
    else if (_shield){
        setColor(Color4::YELLOW);
    }else{
        setColor(Color4::WHITE);
    }

    if (health <= 0) {
        _nextAnimation = AnimationType::DEAD;
    }
    else if (hurt && _curAnimation != AnimationType::DAMAGED) {
        _nextAnimation = AnimationType::DAMAGED;
    }
    else if (casting) {
        _nextAnimation = AnimationType::CASTING;
    }
    else if ((_nextAnimation != AnimationType::RELEASE_CAST || _curAnimation != AnimationType::RELEASE_CAST) 
            && _nextAnimation != AnimationType::DEAD) {
        _nextAnimation = _velocity.length() < idleThreshold ? AnimationType::IDLE : AnimationType::WALK;
    }else{
        _nextAnimation = AnimationType::IDLE;
    }
}

pair<ActionFunction, float> PlayerNode::getNextAnimation() {
    int* frameSet = {};
    int numFrames = 0;
    switch (_nextAnimation) {
    case AnimationType::IDLE:
        if (_curAnimation == AnimationType::CASTING) {
            frameSet = endCast;  
            numFrames = endCastFrames;
        }
        else {
            frameSet = idle;
            numFrames = idleFrames;
        }
        break;
    case AnimationType::WALK:
        if (_curAnimation == AnimationType::CASTING) {
            frameSet = endCast;
            numFrames = endCastFrames;
        }
        else {
            frameSet = walk;
            numFrames = walkFrames;
        }
        break;
    case AnimationType::CASTING:
        if (_curAnimation != AnimationType::CASTING) {
            frameSet = startCast;
            numFrames = startCastFrames;
        }
        else {
            frameSet = casting;
            numFrames = castingFrames;
        }
        break;
    case AnimationType::RELEASE_CAST:
        frameSet = releaseCast;
        numFrames = releaseCastFrames;
        break;
    case AnimationType::DAMAGED:
        frameSet = damage;
        numFrames = damageFrames;
        break;
    case AnimationType::DEAD:
        if (_curAnimation == AnimationType::CASTING) {
            frameSet = endCast;
            numFrames = endCastFrames;

            // This is to kinda cheese it and have it do the dying animation too
            _nextAnimation = AnimationType::IDLE;
        }
        else if (_curAnimation != AnimationType::DEAD) {
            frameSet = dying;
            numFrames = dyingFrames;
        }
        else {
            frameSet = dead;
            numFrames = deadFrames;
        }
        break;
    default:
        CULog("Animation for this state %d has not been implemented!", _nextAnimation);
    }

    _curAnimation = _nextAnimation;
    //CULog("Loop player animation, %.2f", numFrames/FPS);
    return {
        [this, frameSet, numFrames](float t, ActionState state) {
            int offset = state == ActionState::FINISH ? numFrames - 1 :
                state == ActionState::BEGIN ? 0 :
                min((int)(t * numFrames), numFrames - 1);
            setFrame(frameSet[getDirection()] + offset);
        },
        numFrames / FPS
    };
}

bool PlayerNode::skipToNext() {
    return (_curAnimation == AnimationType::IDLE && _nextAnimation != AnimationType::IDLE) ||
        (_curAnimation == AnimationType::CASTING && _nextAnimation != AnimationType::CASTING) ||
        (_nextAnimation == AnimationType::DEAD && _curAnimation != AnimationType::DEAD) ||
        (_nextAnimation == AnimationType::DAMAGED && _curAnimation != AnimationType::DAMAGED);
}

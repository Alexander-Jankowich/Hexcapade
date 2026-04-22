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

#define FPS 8.0f

int PlayerNode::rows = 1;
int PlayerNode::cols = 1;

int PlayerNode::idle[4] = { 0,0,0,0 };
int PlayerNode::walk[4] = { 0,0,0,0 };
int PlayerNode::startCast[4] = { 0,0,0,0 };
int PlayerNode::casting[4] = { 0,0,0,0 };
int PlayerNode::endCast[4] = { 0,0,0,0 };
int PlayerNode::dying[4] = { 0,0,0,0 };
int PlayerNode::dead[4] = { 0,0,0,0 };

int PlayerNode::idleFrames = 0;
int PlayerNode::walkFrames = 0;
int PlayerNode::startCastFrames = 0;
int PlayerNode::castingFrames = 0;
int PlayerNode::endCastFrames = 0;
int PlayerNode::dyingFrames = 0;
int PlayerNode::deadFrames = 0;

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
    if (_velocity.x != 0) {
        facingLeft = _velocity.x < 0;
    }
    if (_velocity.y != 0) {
        facingBack = _velocity.y > 0;
    }
}

void PlayerNode::updateAnimationState(PlayerObstacle::AnimationState s) {
    switch (s) {
    case PlayerObstacle::AnimationState::FROZEN:
        _nextAnimation = AnimationType::IDLE;
        setColor(Color4::BLUE);
        break;
    case PlayerObstacle::AnimationState::DEAD:
        _nextAnimation = AnimationType::DEAD;
        break;
    case PlayerObstacle::AnimationState::CASTING:
        _nextAnimation = AnimationType::CASTING;
        break;
    default:
        // Just get direction
        if(mud){
            setColor(Color4::GRAY);
        }else{
            setColor(Color4::WHITE);
        }
        _nextAnimation = _velocity.isZero() ? AnimationType::IDLE : AnimationType::WALK;
        break;
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
            //CULog("frames: %d", startCastFrames);
        }
        else {
            frameSet = casting;
            numFrames = castingFrames;
        }
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
            int offset = min((int)(t * numFrames), numFrames - 1);
            setFrame(frameSet[getDirection()] + offset);
        },
        numFrames / FPS
    };
}

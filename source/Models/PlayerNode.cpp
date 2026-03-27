//
//  Player.cpp
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#include "PlayerNode.h"

using namespace cugl;
using namespace cugl::graphics;

#define PI 3.14159265358979323

int PlayerNode::rows = 1;
int PlayerNode::cols = 1;

int PlayerNode::leftFrontFrame = 0;
int PlayerNode::rightFrontFrame = 0;
int PlayerNode::leftBackFrame = 0;
int PlayerNode::rightBackFrame = 0;
int PlayerNode::deadFrame = 0;

PlayerNode::PlayerNode(const std::shared_ptr<cugl::graphics::Texture>& tex) {
    initWithSheet(tex, rows, cols);

    _velocity = Vec2::ZERO;
    facingBack = false;
    facingLeft = false;
}

void PlayerNode::setVelocity(cugl::Vec2 value) {
    if (_velocity.x != 0) {
        facingLeft = _velocity.x < 0;
    }
    if (_velocity.y != 0) {
        facingBack = _velocity.y > 0;
    }
}

void PlayerNode::updateAnimationState(PlayerObstacle::AnimationState s) {
    if (s == PlayerObstacle::AnimationState::DEAD) {
        setFrame(PlayerNode::deadFrame);
        return;
    }
    if (s==PlayerObstacle::AnimationState::CASTING) {
        setColor(Color4::BLUE);
    }
    else {
        setColor(Color4::WHITE);
    }
    
    if (facingLeft) {
        if (facingBack) {
            setFrame(PlayerNode::leftBackFrame);
        }
        else {
            setFrame(PlayerNode::leftFrontFrame);
        }
    }
    else {
        if (facingBack) {
            setFrame(PlayerNode::rightBackFrame);
        }
        else {
            setFrame(PlayerNode::rightFrontFrame);
        }
    }
}
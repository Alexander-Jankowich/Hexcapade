#include "EnemyNode.h"

using namespace cugl;
using namespace std;

#define FPS 8

/**
 * Creates a ship wiht the given position and data.
 *
 * The JsonValue should be a reference of all of the constants
 * that necessary to set the "hidden physical properties".
 *
 * @param pos   The ship position
 * @param data  The data defining the physics constants
 */
EnemyNode::EnemyNode(shared_ptr<graphics::Texture>& tex, shared_ptr<JsonValue>& consts) :
        LEFT_FORWARD_WALK(consts->getInt("left_forward_walk")),
        RIGHT_FORWARD_WALK(consts->getInt("right_forward_walk")),
        LEFT_BACKWARD_WALK(consts->getInt("left_backward_walk")),
        RIGHT_BACKWARD_WALK(consts->getInt("right_backward_walk")) {

    initWithSheet(tex, consts->getInt("rows"), consts->getInt("columns"));

    facingBack = false;
    facingLeft = false;
    curFrame = 0;
    curFrameOffset = 0;
    setFrame(0);
}

void EnemyNode::setVelocity(Vec2 value) {
    _velocity = value;
    if (_velocity.x != 0) {
        facingLeft = _velocity.x < 0;
    }
    if (_velocity.y != 0) {
        facingBack = _velocity.y > 0;
    }
}

void EnemyNode::updateAnimationState(EnemyObstacle::EnemyState s) {
    if (s == EnemyObstacle::EnemyState::CHASING) {
        if (facingLeft) {
            if (facingBack) {
                curFrameOffset = LEFT_BACKWARD_WALK;
            }
            else {
                curFrameOffset = LEFT_FORWARD_WALK;
            }
        }
        else {
            if (facingBack) {
                curFrameOffset = RIGHT_BACKWARD_WALK;
            }
            else {
                curFrameOffset = RIGHT_FORWARD_WALK;
            }
        }
    }
}
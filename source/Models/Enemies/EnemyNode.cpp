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
        RIGHT_BACKWARD_WALK(consts->getInt("right_backward_walk")),
        NUM_WALK(consts->getInt("walk_frames")) {

    initWithSheet(tex, consts->getInt("rows"), consts->getInt("columns"));

    facingBack = false;
    facingLeft = false;
    frozen = false;
    statue = false;

    curFrame = 0;
    nextAnimation = AnimationType::IDLE;
    setFrame(0);
}

void EnemyNode::updateState() {
    if(frozen){
        setColor(Color4::BLUE);
    }
    else if(statue){
        setColor(Color4::GRAY);
    }
    else{
        setColor(Color4::WHITE);
    }
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

void EnemyNode::updateAnimationState(EnemyObstacle::EnemyState s, int health) {
    if (health <= 0) {
        nextAnimation = AnimationType::DEAD;
        return;
    }

    switch (s) {
    case EnemyObstacle::EnemyState::IDLE:
        nextAnimation = AnimationType::IDLE;
        break;
    case EnemyObstacle::EnemyState::CHASING:
        nextAnimation = AnimationType::WALK;
        break;
    case EnemyObstacle::EnemyState::ATTACK_RESET:
        nextAnimation = AnimationType::ATTACK;
        break;
    }
}

pair<ActionFunction, float> EnemyNode::getNextAnimation() {
    switch (nextAnimation) {
    case AnimationType::DEAD:
        return { [this](float, ActionState) {}, -1 };
    default:
        // Only have walking for now...
        return {
            [this](float t, ActionState state) {
                int offset = min((int)(t * NUM_WALK), NUM_WALK-1);
                if (facingLeft) {
                    if (facingBack) {
                        setFrame(LEFT_BACKWARD_WALK + offset);
                    }
                    else {
                        setFrame(LEFT_FORWARD_WALK + offset);
                    }
                }
                else {
                    if (facingBack) {
                        setFrame(RIGHT_BACKWARD_WALK + offset);
                    }
                    else {
                        setFrame(RIGHT_FORWARD_WALK + offset);
                    }
                }
            },
            NUM_WALK / FPS
        };
    }
}

#include "EnemyNode.h"

using namespace cugl;
using namespace std;

#define FPS 10.0f

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
        WALK{ consts->getInt("right_forward_walk"),
            consts->getInt("right_backward_walk"),
            consts->getInt("left_forward_walk"),
            consts->getInt("left_backward_walk") },
        WALK_FRAMES(consts->getInt("walk_frames")),
        IDLE{ consts->getInt("right_forward_idle"),
            consts->getInt("right_backward_idle"),
            consts->getInt("left_forward_idle"),
            consts->getInt("left_backward_idle") },
        IDLE_FRAMES(consts->getInt("idle_frames")) {

    initWithSheet(tex, consts->getInt("rows"), consts->getInt("columns"));

    facingBack = false;
    facingLeft = false;
    frozen = false;
    statue = false;

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

    //switch (s) {
    //case EnemyObstacle::EnemyState::IDLE:
    //    nextAnimation = AnimationType::IDLE;
    //    break;
    //case EnemyObstacle::EnemyState::CHASING:
    //    nextAnimation = AnimationType::WALK;
    //    break;
    //case EnemyObstacle::EnemyState::ATTACK_RESET:
    //    nextAnimation = AnimationType::ATTACK;
    //    break;
    //}
    if (s == EnemyObstacle::EnemyState::ATTACK_RESET) {
        nextAnimation = AnimationType::ATTACK;
    }
    else if (_velocity.length() < 0.05) {
        nextAnimation = AnimationType::IDLE;
    }
    else {
        nextAnimation = AnimationType::WALK;
    }
}

pair<ActionFunction, float> EnemyNode::getNextAnimation() {
    const int* frameSet = {};
    int numFrames = 0;

    switch (nextAnimation) {
    case AnimationType::DEAD:
        return { [this](float, ActionState) {}, -1 };
    case AnimationType::IDLE:
        frameSet = IDLE;
        numFrames = IDLE_FRAMES;
        break;
    case AnimationType::WALK:
    default:
        frameSet = WALK;
        numFrames = WALK_FRAMES;
    }

    return {
        [this, frameSet, numFrames](float t, ActionState state) {
            int offset = min((int)(t * numFrames), numFrames - 1);
            setFrame(frameSet[getDirection()] + offset);
        },
        numFrames / FPS
    };
}

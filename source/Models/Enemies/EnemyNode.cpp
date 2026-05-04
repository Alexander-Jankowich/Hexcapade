#include "EnemyNode.h"

using namespace cugl;
using namespace std;

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
    setPriority(1);

    _tint = Color4::WHITE;
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
    //if (statue && !frozen) {
    //    setColor(Color4::GRAY);
    //}
    //else if (!frozen) {
    //    setColor(Color4::WHITE);
    //}

    if (s == EnemyObstacle::EnemyState::DEATH) {
        nextAnimation = AnimationType::DEAD;
    }
    else if (s == EnemyObstacle::EnemyState::ATTACK_RESET) {
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
    switch (nextAnimation) {
    case AnimationType::DEAD:
        return END_ANIMATION;
    case AnimationType::IDLE:
        return getAnimationPair(IDLE, IDLE_FRAMES);
    case AnimationType::WALK:
    default:
        return getAnimationPair(WALK, WALK_FRAMES);
    }
}

void EnemyNode::draw(const std::shared_ptr< graphics::SpriteBatch >& batch, const Affine2& transform, Color4 tint) {
    AnimatedSprite::draw(batch, transform, _tint);
}

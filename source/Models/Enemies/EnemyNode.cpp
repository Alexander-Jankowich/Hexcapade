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
        WALK{ consts->getInt("walk-right_forward"),
            consts->getInt("walk-right_backward"),
            consts->getInt("walk-left_forward"),
            consts->getInt("walk-left_backward") },
        WALK_FRAMES(consts->getInt("walk_frames")),
        IDLE{ consts->getInt("idle-right_forward"),
            consts->getInt("idle-right_backward"),
            consts->getInt("idle-left_forward"),
            consts->getInt("idle-left_backward") },
        IDLE_FRAMES(consts->getInt("idle_frames")),
        DAMAGE{ consts->getInt("damage-right_forward"),
            consts->getInt("damage-right_backward"),
            consts->getInt("damage-left_forward"),
            consts->getInt("damage-left_backward") },
        DAMAGE_FRAMES(consts->getInt("damage_frames")),
        DEATH{ consts->getInt("death-right_forward"),
            consts->getInt("death-right_backward"),
            consts->getInt("death-left_forward"),
            consts->getInt("death-left_backward") },
        DEATH_FRAMES(consts->getInt("death_frames")) {

    initWithSheet(tex, consts->getInt("rows"), consts->getInt("columns"));

    facingBack = false;
    facingLeft = false;
    _frozen = false;
    _statue = false;
    _hurt = false;
    _dead = false;

    _nextAnimation = AnimationType::IDLE;
    _curAnimation = AnimationType::IDLE;
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
        _nextAnimation = AnimationType::DEAD;
    }
    //else if (_hurt && _curAnimation != AnimationType::DAMAGED) {
    else if (s == EnemyObstacle::EnemyState::HURT && _curAnimation != AnimationType::DAMAGED) {
        _nextAnimation = AnimationType::DAMAGED;
    }
    else if (s == EnemyObstacle::EnemyState::ATTACK_RESET) {
        _nextAnimation = AnimationType::ATTACK;
    }
    else if (_velocity.length() < 0.2) {
        _nextAnimation = AnimationType::IDLE;
    }
    else {
        _nextAnimation = AnimationType::WALK;
    }
}

pair<ActionFunction, float> EnemyNode::getNextAnimation() {
    AnimationType temp = _curAnimation;
    _curAnimation = _nextAnimation;

    switch (_nextAnimation) {
    case AnimationType::DEAD:
        if (_dead) {
            return END_ANIMATION;
        }
        _dead = true;
        return getAnimationPair(DEATH, DEATH_FRAMES);
    case AnimationType::DAMAGED:
        CULog("play hurt animation");
        return getAnimationPair(DAMAGE, DAMAGE_FRAMES);
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
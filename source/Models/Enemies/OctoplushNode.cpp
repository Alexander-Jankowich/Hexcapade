#include "OctoplushNode.h"

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
OctoplushNode::OctoplushNode(shared_ptr<graphics::Texture>& tex, shared_ptr<JsonValue>& consts,
        const shared_ptr<graphics::Texture>& attack) : EnemyNode(tex, consts) {
    _attack = SpriteNode::allocWithSheet(attack, consts->getInt("attack_rows"), consts->getInt("attack_columns"));
    _offset = Vec2(consts->getInt("attack_offset_x"), consts->getInt("attack_offset_y"));
}

pair<ActionFunction, float> OctoplushNode::getNextAnimation() {
    AnimationType temp = _curAnimation;
    _curAnimation = _nextAnimation;

    switch (_nextAnimation) {
    case AnimationType::DEAD:
        if (_dead) {
            CULog("stop animation");
            return END_ANIMATION;
        }
        _dead = true;
        CULog("death animation");
        return getAnimationPair(DEATH, DEATH_FRAMES);
    case AnimationType::DAMAGED:
        CULog("damage animation");
        return getAnimationPair(DAMAGE, DAMAGE_FRAMES);
    case AnimationType::ATTACK:
        return getAnimationPair(_attack, ATTACK[0], ATTACK_FRAMES);
    case AnimationType::IDLE:
        return getAnimationPair(IDLE, IDLE_FRAMES);
    case AnimationType::WALK:
    default:
        return getAnimationPair(WALK, WALK_FRAMES);
    }
}

void OctoplushNode::draw(const std::shared_ptr< graphics::SpriteBatch >& batch, const Affine2& transform, Color4 tint) {
    if (_curAnimation == AnimationType::ATTACK) {
        _attack->draw(batch, transform + _offset, _tint);
    }
    else {
        AnimatedSprite::draw(batch, transform, _tint);
    }
}
//
//  Player.cpp
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#include "Player.h"

using namespace cugl;
using namespace cugl::graphics;

#define PI 3.14159265358979323

Player::Player(const cugl::Vec2& pos, float speed, std::string pid, const std::shared_ptr<cugl::graphics::Texture>& tex) :
Entity(pos, 100, tex) {
    _speed = speed;
    _playerId = pid;
}

Player::Player(const cugl::Vec2& pos, float speed, const std::shared_ptr<cugl::graphics::Texture>& tex)
: Entity(pos, 100, tex) {
    _speed = speed;
    _curSpell = SpellType::NONE;
    _curSpellNum = 0;
}

void Player::setVelocity(cugl::Vec2 value) {
    Entity::setVelocity(value);
}

void Player::updateAnimationState(PlayerObstacle::AnimationState s) {
    if (s==PlayerObstacle::AnimationState::CASTING) {
        setColor(Color4::BLUE);
    }
    else {
        setColor(Color4::WHITE);
    }
}

void Player::setSpell(SpellType spell) {
    if (spell != _curSpell && spell != SpellType::NONE) {
        _curSpellNum++;
    }
    _curSpell = spell;
}

const std::string Player::getSpellId() const {
    return _playerId+":"+std::to_string(_curSpellNum);
}

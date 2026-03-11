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

Player::Player(const cugl::Vec2& pos, float speed, std::string pid, const std::shared_ptr<cugl::graphics::Texture>& t) : Entity(pos, 100) {
    _playerAngle = -PI/2; // initialize to point right
    _speed = speed;
    _playerId = pid;
    if (t) {
        _size = t->getSize();
        _texture = t;
    }
    else {
        _size = Size::ZERO;
        _texture = nullptr;
    }
}

Player::Player(const cugl::Vec2& pos, float speed) : Entity(pos, 100) {
    _playerAngle = -PI/2; // initialize to point right
    _speed = speed;
    _curSpell = SpellType::NONE;
    _curSpellNum = 0;
}

void Player::setVelocity(cugl::Vec2 value) {
    Entity::setVelocity(value);
    if (!_velocity.isZero()) {
        _playerAngle = _velocity.getAngle();
    }
}

void Player::updateAngle(float angle) {
    _playerAngle = angle;
    //_shape->setAngle(angle);
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
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

int Player::rows = 1;
int Player::cols = 1;

int Player::leftFrontFrame = 0;
int Player::rightFrontFrame = 0;
int Player::leftBackFrame = 0;
int Player::rightBackFrame = 0;

Player::Player(const cugl::Vec2& pos, float speed, std::string pid, const std::shared_ptr<cugl::graphics::Texture>& tex) :
Entity(pos, 100, tex, rows, cols) {
    _speed = speed;
    _playerId = pid;
    facingBack = false;
    facingLeft = false;
}

void Player::setVelocity(cugl::Vec2 value) {
    Entity::setVelocity(value);
    if (_velocity.x != 0) {
        facingLeft = _velocity.x < 0;
    }
    if (_velocity.y != 0) {
        facingBack = _velocity.y > 0;
    }
}

void Player::updateAnimationState(PlayerObstacle::AnimationState s) {
    if (s==PlayerObstacle::AnimationState::CASTING) {
        setColor(Color4::BLUE);
    }
    else {
        setColor(Color4::WHITE);
    }
    
    if (facingLeft) {
        if (facingBack) {
            setFrame(Player::leftBackFrame);
        }
        else {
            setFrame(Player::leftFrontFrame);
            CULog("Frame: %d", Player::leftFrontFrame);
        }
    }
    else {
        if (facingBack) {
            setFrame(Player::rightBackFrame);
        }
        else {
            setFrame(Player::rightFrontFrame);
        }
    }
    //CULog("Frame: %d", getFrame());
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

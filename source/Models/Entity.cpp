#include "Entity.h"

using namespace cugl;
using namespace cugl::graphics;

#define KNOCKBACK_FRAMES 15
#define FROZEN_FRAMES 60

/**
 * Creates a ship wiht the given position and data.
 *
 * The JsonValue should be a reference of all of the constants
 * that necessary to set the "hidden physical properties".
 *
 * @param pos   The ship position
 * @param data  The data defining the physics constants
 */
Entity::Entity(const cugl::Vec2& pos, int health, std::shared_ptr<cugl::graphics::Texture> tex) {
    initWithSheet(tex, 1, 1);
    _position = pos;
    _health = health;

    _knockback = false;
    _knockback_counter = 0;
    _frozen = false;
    _frozen_counter = 0;

    _size = Size::ZERO;
}

/**
 * Sets the current ship health.
 *
 * When the health of the ship is 0, it is "dead"
 *
 * @param value The current ship health.
 */
void Entity::setHealth(int value) {
    if (value >= 0) {
        // Do not allow health to go negative
        _health = value;
    }
    else {
        _health = 0;
    }
}

#pragma mark Movement

void Entity::update() {
    if (_frozen) {
        _frozen_counter++;
        if (_frozen_counter >= FROZEN_FRAMES) {
            _frozen = false;
            _frozen_counter = 0;
        }
        return;
    }
    if (_knockback) {
        _knockback_counter++;
        _position += _knockback_vel.normalize() * _velocity.length();
        if (_knockback_counter >= KNOCKBACK_FRAMES) {
            _knockback = false;
            _knockback_counter = 0;
        }
        return;
    }

    _position += _velocity;
}

void Entity::setKnockback(Vec2 v) {
    _knockback = true;
    _knockback_vel = v;
}

//void Entity::setTexture(const std::shared_ptr<Texture>& value) {
//    if (value) {
//        _size = value->getSize();
//        _texture = value;
//    }
//    else {
//        _size = Size::ZERO;
//        _texture = nullptr;
//    }
//}

void Entity::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
    static Affine2 trans;
    trans = Affine2::createTranslation(_position);
    batch->draw(_texture, Color4::WHITE, _size/2, trans);
}

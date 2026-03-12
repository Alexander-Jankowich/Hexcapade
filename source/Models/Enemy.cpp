#include "Enemy.h"

using namespace cugl;

/**
 * Creates a ship wiht the given position and data.
 *
 * The JsonValue should be a reference of all of the constants
 * that necessary to set the "hidden physical properties".
 *
 * @param pos   The ship position
 * @param data  The data defining the physics constants
 */
Enemy::Enemy(const cugl::Vec2& pos, float speed, int health) : Entity(pos, health) {
    _speed = speed;
}

#pragma mark Movement

bool Enemy::update(Vec2 target) {
    _velocity = target - _pos;
    _velocity.normalize();
    _velocity *= _speed;

    Entity::update();

    return _health > 0;
}
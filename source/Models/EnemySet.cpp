//
//  Spell.cpp
//  gp
//
//  Created by Alex Jankowich on 2/22/26.
//
#include "EnemySet.h"
using namespace cugl;
using namespace cugl::graphics;

EnemySet::EnemySet() {
    current.clear();
    _radius = 0;
}

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
EnemySet::Enemy::Enemy(const cugl::Vec2& pos, int health) {
    _pos = pos;
    _speed = 2.5;

    _health = health;
    _knockback = false;
    _knockback_counter = 0;
    _frozen = false;
    _frozen_counter = 0;
}

/**
 * Sets the current ship health.
 *
 * When the health of the ship is 0, it is "dead"
 *
 * @param value The current ship health.
 */
void EnemySet::Enemy::setHealth(int value) {
    if (value >= 0) {
        // Do not allow health to go negative
        _health = value;
    }
    else {
        _health = 0;
    }
}

#pragma mark Movement

bool EnemySet::Enemy::update(Vec2 target) {
    if (_frozen) {
        _frozen_counter++;
        if (_frozen_counter >= FROZEN_FRAMES) {
            _frozen = false;
            _frozen_counter = 0;
        }
        return _health > 0;
    }
    if (_knockback) {
        _knockback_counter++;
        _pos += _knockback_vel.normalize() * _speed;
        if (_knockback_counter >= KNOCKBACK_FRAMES) {
            _knockback = false;
            _knockback_counter = 0;
        }
        return _health > 0;
    }

    Vec2 dir = target - _pos;
    dir.normalize();
    dir *= _speed;

    _pos += dir;

    return _health > 0;
}

void EnemySet::Enemy::setKnockback(Vec2 v) {
    _knockback = true;
    _knockback_vel = v;
}

void EnemySet::setTexture(const std::shared_ptr<Texture>& value) {
    if (value) {
        Size size = value->getSize();
        _radius = std::max(size.width, size.height) / 2;
        _texture = value;
    }
    else {
        _radius = 0;
        _texture = nullptr;
    }
}

void EnemySet::spawnEnemy(const cugl::Vec2 position, int health){
    current.emplace(std::make_shared<EnemySet::Enemy>(position, health));
}

void EnemySet::update(Vec2 target){
    for (auto it = current.begin(); it != current.end(); ) {
        if ((*it)->update(target)) {
            it++;
        }
        else {
            // delete enemies if they fall below 0 hp.
            it = current.erase(it);
        }
    }
}

void EnemySet::draw(const std::shared_ptr<SpriteBatch>& batch) {
    if (_texture) {
        for (auto it = current.begin(); it != current.end(); ++it) {
            Vec2 pos = (*it)->getPosition();
            Vec2 origin(_radius, _radius);

            Affine2 trans;
            trans.translate(pos); 
            batch->draw(_texture, Color4::RED, origin, trans);
        }
    }
}

void EnemySet::dispose() {
    _texture = nullptr;
}

void EnemySet::reset() {
    for (auto it = current.begin(); it != current.end(); ) {
        // delete enemies if they fall below 0 hp.
        it = current.erase(it);
    }
}
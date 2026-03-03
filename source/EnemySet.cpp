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

void EnemySet::spawnEnemy(const cugl::Vec2 position, int health){
    current.emplace(std::make_shared<Enemy>(position, 2.5, health));
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
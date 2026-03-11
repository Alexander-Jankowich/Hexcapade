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
    _totalCount = 0;
    
}

void EnemySet::spawnEnemy(const cugl::Vec2 position, int health){
    // this IS stupid but yolo - serena
    current.insert(std::make_pair(_totalCount, std::make_shared<Enemy>(position, 2.5, health)));
    _totalCount++;
}
void EnemySet::clientSpawnEnemy(int eid, const cugl::Vec2 position, int health){
    // this IS stupid but yolo - serena
    current.insert(std::make_pair(eid, std::make_shared<Enemy>(position, 2.5, health)));
}

Vec2 getClosestPlayer(Vec2 enemPos, std::vector<Vec2> targs) {
    float minDist = FLT_MAX;
    Vec2 minPos = Vec2(0, 0); // default
    for (auto t : targs) {
        float d =enemPos.distance(t);
        if (d < minDist) {
            minDist = d;
            minPos = t;
        }
    }
    return minPos;
}
void EnemySet::update(std::vector<Vec2> targets){
    for (auto it = current.begin(); it != current.end(); ) {
        Vec2 targ = getClosestPlayer(std::get<1>(*it)->getPosition(), targets);
        if (std::get<1>(*it)->update(targ)) {
            it++;
        }
        else {
            // delete enemies if they fall below 0 hp.
            it = current.erase(it);
        }
    }
}
void EnemySet::updateEnemyAsClient(int eid, const cugl::Vec2 position, int health) {
    if (!current.contains(eid)){
        clientSpawnEnemy(eid, position, health);
        return;
    }
    current.at(eid)->setPosition(position);
    current.at(eid)->setHealth(health);
}

void EnemySet::draw(const std::shared_ptr<SpriteBatch>& batch) {
    if (_texture) {
        for (auto it = current.begin(); it != current.end(); ++it) {
            Vec2 pos = std::get<1>(*it)->getPosition();
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

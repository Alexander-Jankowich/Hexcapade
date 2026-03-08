//
//  Spell.cpp
//  gp
//
//  Created by Alex Jankowich on 2/22/26.
//
#include "SpellSet.h"
using namespace cugl;
using namespace cugl::graphics;


#pragma markl Spell

SpellSet::Spell::Spell(const cugl::Vec2 pos,const cugl::Vec2 vel, bool f){
    damage = f ? 2 : 5;
    froze = f;
    velocity = vel;
    position = pos;
    
}

void SpellSet::Spell::update(){
    position += velocity;
};

SpellSet::SpellSet(){
    current.clear();
    pending.clear();
    mass = 0.0;
}

void SpellSet::setTexture(const std::shared_ptr<Texture>& value) {
        Size size = value->getSize();
        radius = std::max(size.width,size.height)/2;
        texture = value;

}

void SpellSet::spawnSpell(Vec2 position, float speed, float angle, bool freeze) {
    cugl::Vec2 dir(std::cos(angle), std::sin(angle));
    spawnSpell(position, speed, dir, freeze);
}

void SpellSet::spawnSpell(Vec2 position, float speed, Vec2 angle,bool freeze) {
    cugl::Vec2 pvel =  angle.getNormalization() * speed;

    std::shared_ptr<Spell> spell = std::make_shared<Spell>(position, pvel,freeze);

    pending.emplace(spell);
    
}


void SpellSet::update(){
    for (auto it = current.begin(); it != current.end(); ) {
        (*it)->update();
        it++;
        }
    
    for(auto it = pending.begin(); it != pending.end(); ++it) {
        current.emplace(*it);
    }
    pending.clear();
    
}

void SpellSet::draw(const std::shared_ptr<SpriteBatch>& batch) {
    for(auto it = current.begin(); it != current.end(); ++it) {
        Vec2 pos = (*it)->position;
        Vec2 origin(radius,radius);
        
        Affine2 trans;
        trans.translate(pos);
        if((*it) -> froze){
            batch -> draw(texture,Color4::BLUE,origin,trans);
        }else{
            batch -> draw(texture,Color4::RED, origin, trans);
        }
        
    }
}

void SpellSet::dispose() {
    texture = nullptr;
}

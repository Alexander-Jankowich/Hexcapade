//
//  Spell.cpp
//  gp
//
//  Created by Alex Jankowich on 2/22/26.
//
#include "SpellSet.h"
using namespace cugl;
using namespace cugl::graphics;

#define SPELL_SPEED 5

#pragma mark Spell

SpellSet::Spell::Spell(const cugl::Vec2 pos,const cugl::Vec2 vel, int dmg, bool f){
    damage = dmg;
    froze = f;
    velocity = vel;
    position = pos;
}

void SpellSet::Spell::update(){
    position += velocity;
};

SpellSet::SpellSet(std::shared_ptr<cugl::JsonValue> spellConsts){
    _spellConsts = spellConsts;

    current.clear();
    pending.clear();
    _mass = 0.0;
}

void SpellSet::setTexture(const std::shared_ptr<Texture>& value) {
    Size size = value->getSize();
    _radius = std::max(size.width,size.height)/2;
    _texture = value;
}

void SpellSet::spawnSpell(std::string sid, SpellType type, Vec2 position, float angle) {
    cugl::Vec2 dir(std::cos(angle), std::sin(angle));
    spawnSpell(sid, type, position, dir);
}

void SpellSet::spawnSpell(std::string sid, SpellType type, cugl::Vec2 position, cugl::Vec2 angle) {
    cugl::Vec2 norm = angle.getNormalization();
    std::shared_ptr<Spell> spell;
    std::shared_ptr<JsonValue> values;

    switch (type) {
    case SpellType::FIREBALL:
        values = _spellConsts->get("fireball");
        break;
    case SpellType::FREEZE:
        values = _spellConsts->get("freeze");
        break;
    case SpellType::NONE:
        return;
    default:
        CULog("Spell not implemented!!!!");
        return;
    }

    spell = std::make_shared<Spell>(position, norm * values->getFloat("speed"),
        values->getInt("damage"),
        values->getBool("freeze", false));

    pending.emplace(sid, spell);
}


void SpellSet::update(){
    for (auto it = current.begin(); it != current.end(); ) {
        it->second->update();
        it++;
    }
    
    for(auto it = pending.begin(); it != pending.end(); ++it) {
        current.emplace(*it);
    }
    pending.clear();
    
}

void SpellSet::draw(const std::shared_ptr<SpriteBatch>& batch) {
    for(auto it = current.begin(); it != current.end(); ++it) {
        Vec2 pos = it->second->position;
        Vec2 origin(_radius,_radius);
        
        Affine2 trans;
        trans.translate(pos);
        if(it->second-> froze){
            batch -> draw(_texture,Color4::BLUE,origin,trans);
        }else{
            batch -> draw(_texture,Color4::RED, origin, trans);
        }
        
    }
}

bool SpellSet::contains(std::string sid) {
    return current.contains(sid) || pending.contains(sid);
}

void SpellSet::dispose() {
    _texture = nullptr;
}

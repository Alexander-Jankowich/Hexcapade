//
//  SpellNode.cpp
//  tech_final
//
// This class represents the SceneNode for a Spell
//  Created by Alex Jankowich on 3/16/26.
//

#include "SpellNode.h"

SpellNode::SpellNode(const cugl::Vec2 pos, const cugl::Vec2 vel, int rows, int cols,SpellType t, std::shared_ptr<cugl::graphics::Texture> tex) {
    initWithSheet(tex, rows, cols);
    if(!(t == SpellType::COMBO_FREEZE or t == SpellType::COMBO_FIREBALL or t == SpellType::DEATH)){
        setScale(0.5);
    }
    if(t == SpellType::DEATH){
        setColor(Color4::BLACK);
    }
    if(t == SpellType::EARTH){
        setColor(Color4::PAPYRUS);
    }
    
    _velocity = vel;
    _position = pos;
}

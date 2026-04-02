//
//  SpellNode.cpp
//  tech_final
//
// This class represents the SceneNode for a Spell
//  Created by Alex Jankowich on 3/16/26.
//

#include "SpellNode.h"

SpellNode::SpellNode(const cugl::Vec2 pos, const cugl::Vec2 vel, int rows, int cols, std::shared_ptr<cugl::graphics::Texture> tex) {
    initWithSheet(tex, rows, cols);
    setScale(0.5);
    _velocity = vel;
    _position = pos;
}

//
//  SpellNode.h
//  tech_final
//
//  Created by Alex Jankowich on 3/16/26.
//
#ifndef __SPELL_NODE_H__
#define __SPELL_NODE_H__

#include <cugl/cugl.h>
using namespace cugl;
using namespace cugl::scene2;
enum class SpellType {
    NONE=0,
    FREEZE=1,
    FIREBALL=2,
    HEAL=3,
    COMBO_FIREBALL=4,
    COMBO_FREEZE=5
};

class SpellNode : public SpriteNode{
    
protected:
    Vec2 _velocity;
    Vec2 _pos;
public:
    SpellNode(const cugl::Vec2 pos, cugl::Vec2 vel, int rows, int cols,SpellType t, std::shared_ptr<cugl::graphics::Texture> texture);
    
    void setVel(Vec2 v) { _velocity = v; }
    void setPos(Vec2 p) { _pos = p; }
    Vec2 getPos() { return _pos; }
    Vec2 getVel() { return _velocity; }
};



#endif // !__SPELL_NODE_H__

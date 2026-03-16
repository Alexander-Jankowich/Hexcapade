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

class SpellNode : public SpriteNode{
    
protected:
    Vec2 _velocity;
    Vec2 _pos;
public:
    void setVel(Vec2 v) { _velocity = v; }
    void setPos(Vec2 p) { _pos = p; }
    Vec2 getPos() { return _pos; }
    Vec2 getVel() { return _velocity; }
    
    
};



#endif /* __NP_CRATE_EVENT_H__ */

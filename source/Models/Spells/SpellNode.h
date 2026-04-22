//
//  SpellNode.h
//  tech_final
//
//  Created by Alex Jankowich on 3/16/26.
//
#ifndef __SPELL_NODE_H__
#define __SPELL_NODE_H__

#include <cugl/cugl.h>
#include "../Animations.h"

using namespace cugl;
using namespace cugl::scene2;


enum class SpellType {
    NONE=0,
    FREEZE=1,
    FIREBALL=2,
    HEAL=3,
    COMBO_FIREBALL=4,
    COMBO_FREEZE=5,
    DEATH = 6,
    EARTH = 7,
    COMBO_EFREEZE = 8,
    WATER = 9
};

class SpellNode : public AnimatedSprite {
private:
    enum AnimationType {
        ACTIVE = 0,
        END = 1
    };

    Vec2 _velocity;
    Vec2 _pos;

    AnimationType _nextAnimation;
    bool _deleteAtEnd;

    static int cols;
    static int rows;

    static int mainLoop;
    static int mainLoopFrames;
    
    static int end;
    static int endFrames;

public:
    SpellNode(const cugl::Vec2 pos, cugl::Vec2 vel, SpellType t, std::shared_ptr<cugl::graphics::Texture> texture);

    /**
    * Returns the next animation function and duration
    */
    pair<ActionFunction, float> getNextAnimation() override;

#pragma mark Properties
    static void setConsts(std::shared_ptr<JsonValue> consts) {
        SpellNode::cols = consts->getInt("columns");
        SpellNode::rows = consts->getInt("rows");

        SpellNode::mainLoop = consts->getInt("main-loop");
        SpellNode::mainLoopFrames = consts->getInt("main-loop_frames");
        SpellNode::end = consts->getInt("end");
        SpellNode::endFrames = consts->getInt("end_frames");
    }

    void setVel(Vec2 v) { _velocity = v; }
    void setPos(Vec2 p) { _pos = p; }

    Vec2 getPos() { return _pos; }
    Vec2 getVel() { return _velocity; }

    void setRemove() { _nextAnimation = AnimationType::END; }
    bool shouldRemove() { return _nextAnimation == AnimationType::END; }
};



#endif // !__SPELL_NODE_H__

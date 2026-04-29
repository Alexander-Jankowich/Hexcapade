//
//  SpellNode.cpp
//  tech_final
//
// This class represents the SceneNode for a Spell
//  Created by Alex Jankowich on 3/16/26.
//

#include "SpellNode.h"

using namespace std;
using namespace cugl;

#define FPS 10.0f

int SpellNode::rows = 1;
int SpellNode::cols = 1;
int SpellNode::mainLoop = 0;
int SpellNode::mainLoopFrames = 1;
int SpellNode::end = 0;
int SpellNode::endFrames = 1;

SpellNode::SpellNode(const cugl::Vec2 pos, const cugl::Vec2 vel, SpellType t, std::shared_ptr<cugl::graphics::Texture> tex) {
    initWithSheet(tex, rows, cols);
    if(!(t == SpellType::COMBO_FREEZE or t == SpellType::COMBO_FIREBALL or t == SpellType::WATER)){
        setScale(0.5);
    }
    if(t == SpellType::WATER){
        CULog("im casting water");
        setColor(Color4::BLUE);
    }
    if(t == SpellType::COMBO_EFREEZE){
        setColor(Color4::GRAY);
    }
    if(t == SpellType::BUFF){
        setColor(Color4::RED);
    }
    if(t == SpellType::SHIELD){
        setColor(Color4::YELLOW);
    }
    
    _velocity = vel;
    _position = pos;

    _deleteAtEnd = false;
    _nextAnimation = AnimationType::ACTIVE;
}

pair<ActionFunction, float> SpellNode::getNextAnimation() {
    if (_deleteAtEnd) {
        return { [this](float, ActionState) {}, -1 };
    }

    switch (_nextAnimation) {
    case AnimationType::ACTIVE:
        return {
            [this](float t, ActionState state) {
                int offset = state == ActionState::FINISH ? mainLoopFrames - 1 :
                    state == ActionState::BEGIN ? 0 :
                    min((int)(t * mainLoopFrames), mainLoopFrames - 1);
                setFrame(mainLoop + offset);
            },
            mainLoopFrames / FPS
        };
    case AnimationType::END:
        _deleteAtEnd = true;
        return {
            [this](float t, ActionState state) {
                int offset = state == ActionState::FINISH ? endFrames - 1 :
                    state == ActionState::BEGIN ? 0 : 
                    min((int)(t * endFrames), endFrames - 1);
                setFrame(end + offset);
            },
            endFrames / FPS
        };
    default:
        CULog("How did you get here? There are no other animation types. (SpellNode 74)");
        return { [this](float, ActionState) {}, -1 };
    }
}

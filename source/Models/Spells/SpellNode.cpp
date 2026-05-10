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

int SpellNode::rows = 1;
int SpellNode::cols = 1;
int SpellNode::mainLoop = 0;
int SpellNode::mainLoopFrames = 1;
int SpellNode::end = 0;
int SpellNode::endFrames = 1;

SpellNode::SpellNode(const cugl::Vec2 pos, const cugl::Vec2 vel, SpellType t, std::shared_ptr<cugl::graphics::Texture> tex) {
    initWithSheet(tex, rows, cols);
    if(t == SpellType::COMBO_EFREEZE){
        setColor(Color4::GRAY);
    }
    if(t == SpellType::MUD_WATER){
        setColor(Color4::PAPYRUS);
    }
    
    _velocity = vel;
    _position = pos;

    _deleteAtEnd = false;
    _nextAnimation = AnimationType::ACTIVE;

    setFrame(mainLoop);
    setPriority(1);
}

pair<ActionFunction, float> SpellNode::getNextAnimation() {
    if (_deleteAtEnd) {
        return END_ANIMATION;
    }

    switch (_nextAnimation) {
    case AnimationType::ACTIVE:
        return getAnimationPair(mainLoop, mainLoopFrames);
    case AnimationType::END:
        _deleteAtEnd = true;
        return getAnimationPair(end, endFrames);
    default:
        CULog("How did you get here? There are no other animation types. (SpellNode::getNextAnimation)");
        return END_ANIMATION;
    }
}

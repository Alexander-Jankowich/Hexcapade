//
//  SpellEntityNode.h
//  hexc
//
//  Created by Kaz Tam on 4/29/26.
//

#ifndef __SPELL_ENTITY_NODE_H__
#define __SPELL_ENTITY_NODE_H__

#include <cugl/cugl.h>
#include "../Animations.h"

using namespace std;
using namespace cugl;

class SpellEntityNode : public AnimatedSprite {
private:
    const int CREATED;
    const int CREATED_FRAMES;
    const int EXISTING;
    const int EXISTING_FRAMES;
    const int END;
    const int END_FRAMES;

    bool shouldEnd;
    bool ended;
    bool finishedCreate;

public:
    SpellEntityNode(shared_ptr<graphics::Texture> texture, shared_ptr<JsonValue> consts);

    /**
    * Returns the next animation function and duration
    */
    pair<ActionFunction, float> getNextAnimation() override;

    /**
    * Call this function to trigger the end entity animation
    */
    void end() { shouldEnd = true; }
};

#endif
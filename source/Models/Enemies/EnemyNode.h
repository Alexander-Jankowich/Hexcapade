//
//  SLShip.h
//  Ship Lab
//
//  This class tracks all of the state (position, velocity, rotation) of a
//  single ship. In order to obey the separation of the model-view-controller
//  pattern, controller specific code (such as reading the keyboard) is not
//  present in this class.
//
//  With that said, you will notice several elements that look like they are
//  part of the view, namely the texture and sound.  But this is the texture
//  and sound DATA.  The actual view are the sprite batch and audio engine,
//  respectively, that use this data to provide feedback on the screen.
//
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//
//  Author: Walker White
//  Version: 1/20/25
//
#ifndef __GP_ENEMY_H__
#define __GP_ENEMY_H__
#include <cugl/cugl.h>
#include "EnemyObstacle.h"

using namespace cugl;
using namespace std;

/**
 * Model class representing an enemy.
 */
class EnemyNode : public scene2::SpriteNode {
private:
    Vec2 _velocity;

    bool facingBack;
    bool facingLeft;

    int curFrame;
    int curFrameOffset;

    const int LEFT_FORWARD_WALK;
    const int RIGHT_FORWARD_WALK;
    const int LEFT_BACKWARD_WALK;
    const int RIGHT_BACKWARD_WALK;

public:
#pragma mark Constructors
    /**
     * Creates a enemy node the given position and data.
     *
     * @param pos   The ship position
     * @param data  The data defining the physics constants
     */
    EnemyNode(shared_ptr<graphics::Texture>& tex, shared_ptr<JsonValue>& consts);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~EnemyNode() {}
    
#pragma mark Properties

    void setVelocity(Vec2 value);

    void updateAnimationState(EnemyObstacle::EnemyState s);
};

#endif // !__GP_ENEMY_H__


//
//  EnemyNode.h
//  hexc
//
//  This class represents the enemy node for the scene graph and controls
//  all graphics for an enemy
//
//  Author: Kaz Tam
//  Version: 1/20/25
//

#ifndef __GP_ENEMY_H__
#define __GP_ENEMY_H__
#include <cugl/cugl.h>
#include "EnemyObstacle.h"
#include "../Animations.h"

using namespace cugl;
using namespace std;

/**
 * Model class representing an enemy.
 */
class EnemyNode : public AnimatedSprite {
private:
    enum class AnimationType {
        IDLE,
        WALK,
        ATTACK,
        DEAD
    };

    enum class Direction {
        RIGHT_FORWARD = 0,
        RIGHT_BACKWARD = 1,
        LEFT_FORWARD = 2,
        LEFT_BACKWARD = 3
    };

    Vec2 _velocity;

    bool facingBack;
    bool facingLeft;

    bool frozen;
    bool statue;

    AnimationType nextAnimation;

    const int WALK[4];
    const int WALK_FRAMES;

    const int IDLE[4];
    const int IDLE_FRAMES;

    int getDirection() { return facingLeft * 2 + facingBack; }

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

    /**
    * Returns the next animation function and duration
    */
    pair<ActionFunction, float> getNextAnimation() override;

    void setFrozen(bool f) { frozen = f; }
    bool getFrozen() { return frozen; }

    void setStatue(bool s) { statue = s; }
    bool getStatue() { return statue; }
    
    void updateState();
    
#pragma mark Graphics
};

#endif // !__GP_ENEMY_H__


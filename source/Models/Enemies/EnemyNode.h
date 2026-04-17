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

    Vec2 _velocity;

    bool facingBack;
    bool facingLeft;

    bool frozen;
    bool statue;

    AnimationType nextAnimation;

    const int LEFT_FORWARD_WALK;
    const int RIGHT_FORWARD_WALK;
    const int LEFT_BACKWARD_WALK;
    const int RIGHT_BACKWARD_WALK;
    const int NUM_WALK;


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

    void updateAnimationState(EnemyObstacle::EnemyState s, int health);

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


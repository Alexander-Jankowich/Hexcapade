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
protected:
    enum class AnimationType {
        IDLE,
        WALK,
        ATTACK,
        DAMAGED,
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

    bool _frozen;
    bool _statue;
    bool _hurt;
    bool _dead;

    AnimationType _nextAnimation;
    AnimationType _curAnimation;

    const int WALK[4];
    const int WALK_FRAMES;

    const int IDLE[4];
    const int IDLE_FRAMES;

    const int ATTACK[4];
    const int ATTACK_FRAMES;

    const int DAMAGE[4];
    const int DAMAGE_FRAMES;

    const int DEATH[4];
    const int DEATH_FRAMES;

    int getDirection() override { return facingLeft * 2 + facingBack; }

    Color4 _tint;

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

    void setFrozen(bool f) { _frozen = f; }
    bool getFrozen() { return _frozen; }

    void setStatue(bool s) { _statue = s; }
    bool getStatue() { return _statue; }

    void setHurt(bool h) { _hurt = h; }
    bool isHurt() { return _hurt; }

    bool isDead() { return _dead; }

    bool isAttacking() { return _nextAnimation == AnimationType::ATTACK; }
    
#pragma mark Graphics
    void draw(const std::shared_ptr< graphics::SpriteBatch >& batch, const Affine2& transform, Color4 tint) override;

    void setTint(Color4 t) { _tint = t; }
};

#endif // !__GP_ENEMY_H__


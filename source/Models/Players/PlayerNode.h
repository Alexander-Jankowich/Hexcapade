//
//  Player.h
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#ifndef __GP_PLAYER_H__
#define __GP_PLAYER_H__

#include <cugl/cugl.h>
#include "PlayerObstacle.h"
#include "../Animations.h"

class PlayerNode : public AnimatedSprite {
private:
    enum class AnimationType {
        IDLE,
        WALK,
        CASTING,
        RELEASE_CAST,
        DAMAGED,
        DEAD
    };

    enum class Direction {
        RIGHT_FORWARD = 0,
        RIGHT_BACKWARD = 1,
        LEFT_FORWARD = 2,
        LEFT_BACKWARD = 3
    };

    cugl::Vec2 _velocity;

    bool facingBack;
    bool facingLeft;

    AnimationType _curAnimation;
    AnimationType _nextAnimation;
    
    static int walk[4];
    static int walkFrames;

    static int idle[4];
    static int idleFrames;

    static int startCast[4];
    static int startCastFrames;

    static int casting[4];
    static int castingFrames;

    static int endCast[4];
    static int endCastFrames;

    static int releaseCast[4];
    static int releaseCastFrames;

    static int damage[4];
    static int damageFrames;

    static int dying[4];
    static int dyingFrames;

    static int dead[4];
    static int deadFrames;

    static int cols;
    static int rows;

    static float idleThreshold;
    
    bool _frozen = false;
    bool _mud = false;
    bool _buff = false;
    bool _shield = false;
    bool _hurt = false;
    
    int getDirection() override { return facingLeft * 2 + facingBack; }

public:
#pragma mark Constructors

    /**
     * Creates a player
     */
    PlayerNode(const std::shared_ptr<cugl::graphics::Texture>& t);


    /**
     * Disposes the player, releasing all resources.
     */
    ~PlayerNode() {}
    
#pragma mark Properties
    static void setConsts(std::shared_ptr<JsonValue> consts) {
        //CULog("set consts-------------");
        PlayerNode::cols = consts->getInt("columns");
        PlayerNode::rows = consts->getInt("rows");

        PlayerNode::walk[(int)Direction::LEFT_FORWARD] = consts->getInt("walk-left_forward");
        PlayerNode::walk[(int)Direction::RIGHT_FORWARD] = consts->getInt("walk-right_forward");
        PlayerNode::walk[(int)Direction::LEFT_BACKWARD] = consts->getInt("walk-left_backward");
        PlayerNode::walk[(int)Direction::RIGHT_BACKWARD] = consts->getInt("walk-right_backward");
        PlayerNode::walkFrames = consts->getInt("walk_frames");

        PlayerNode::idle[(int)Direction::LEFT_FORWARD] = consts->getInt("idle-left_forward");
        PlayerNode::idle[(int)Direction::RIGHT_FORWARD] = consts->getInt("idle-right_forward");
        PlayerNode::idle[(int)Direction::LEFT_BACKWARD] = consts->getInt("idle-left_backward");
        PlayerNode::idle[(int)Direction::RIGHT_BACKWARD] = consts->getInt("idle-right_backward");
        PlayerNode::idleFrames = consts->getInt("idle_frames");

        PlayerNode::startCast[(int)Direction::LEFT_FORWARD] = consts->getInt("start_cast-left_forward");
        PlayerNode::startCast[(int)Direction::RIGHT_FORWARD] = consts->getInt("start_cast-right_forward");
        PlayerNode::startCast[(int)Direction::LEFT_BACKWARD] = consts->getInt("start_cast-left_backward");
        PlayerNode::startCast[(int)Direction::RIGHT_BACKWARD] = consts->getInt("start_cast-right_backward");
        PlayerNode::startCastFrames = consts->getInt("start_cast_frames");

        PlayerNode::casting[(int)Direction::LEFT_FORWARD] = consts->getInt("casting-left_forward");
        PlayerNode::casting[(int)Direction::RIGHT_FORWARD] = consts->getInt("casting-right_forward");
        PlayerNode::casting[(int)Direction::LEFT_BACKWARD] = consts->getInt("casting-left_backward");
        PlayerNode::casting[(int)Direction::RIGHT_BACKWARD] = consts->getInt("casting-right_backward");
        PlayerNode::castingFrames = consts->getInt("casting_frames");

        PlayerNode::endCast[(int)Direction::LEFT_FORWARD] = consts->getInt("end_cast-left_forward");
        PlayerNode::endCast[(int)Direction::RIGHT_FORWARD] = consts->getInt("end_cast-right_forward");
        PlayerNode::endCast[(int)Direction::LEFT_BACKWARD] = consts->getInt("end_cast-left_backward");
        PlayerNode::endCast[(int)Direction::RIGHT_BACKWARD] = consts->getInt("end_cast-right_backward");
        PlayerNode::endCastFrames = consts->getInt("end_cast_frames");

        PlayerNode::releaseCast[(int)Direction::LEFT_FORWARD] = consts->getInt("release_cast-left_forward");
        PlayerNode::releaseCast[(int)Direction::RIGHT_FORWARD] = consts->getInt("release_cast-right_forward");
        PlayerNode::releaseCast[(int)Direction::LEFT_BACKWARD] = consts->getInt("release_cast-left_backward");
        PlayerNode::releaseCast[(int)Direction::RIGHT_BACKWARD] = consts->getInt("release_cast-right_backward");
        PlayerNode::releaseCastFrames = consts->getInt("release_cast_frames");

        PlayerNode::damage[(int)Direction::LEFT_FORWARD] = consts->getInt("damage-left_forward");
        PlayerNode::damage[(int)Direction::RIGHT_FORWARD] = consts->getInt("damage-right_forward");
        PlayerNode::damage[(int)Direction::LEFT_BACKWARD] = consts->getInt("damage-left_backward");
        PlayerNode::damage[(int)Direction::RIGHT_BACKWARD] = consts->getInt("damage-right_backward");
        PlayerNode::damageFrames = consts->getInt("damage_frames");

        PlayerNode::dying[(int)Direction::LEFT_FORWARD] = consts->getInt("dying-left_forward");
        PlayerNode::dying[(int)Direction::RIGHT_FORWARD] = consts->getInt("dying-right_forward");
        PlayerNode::dying[(int)Direction::LEFT_BACKWARD] = consts->getInt("dying-left_backward");
        PlayerNode::dying[(int)Direction::RIGHT_BACKWARD] = consts->getInt("dying-right_backward");
        PlayerNode::dyingFrames = consts->getInt("dying_frames");

        PlayerNode::dead[(int)Direction::LEFT_FORWARD] = consts->getInt("dead-left_forward");
        PlayerNode::dead[(int)Direction::RIGHT_FORWARD] = consts->getInt("dead-right_forward");
        PlayerNode::dead[(int)Direction::LEFT_BACKWARD] = consts->getInt("dead-left_backward");
        PlayerNode::dead[(int)Direction::RIGHT_BACKWARD] = consts->getInt("dead-right_backward");
        PlayerNode::deadFrames = consts->getInt("dead_frames");

        PlayerNode::idleThreshold = consts->getFloat("idle_threshold");
    }
    
    void reset() {
        setPosition(getSize() / 2);
        setVelocity(cugl::Vec2::ZERO);

        _curAnimation = AnimationType::IDLE;
        _nextAnimation = AnimationType::IDLE;

        _frozen = false;
        _mud = false;
        _buff = false;
    }
    
    void updateAnimationState(int health, bool casting, bool m);

    void setVelocity(cugl::Vec2 value);

    std::pair<cugl::ActionFunction, float> getNextAnimation() override;

    void releasedSpell() { _nextAnimation = AnimationType::RELEASE_CAST; }

    void setFrozen(bool f) { _frozen = f; }
    bool isFrozen() { return _frozen; }

    void setHurt(bool h) { _hurt = h; }
    bool isHurt() { return _hurt; }

    void setBuff(bool b) { _buff = b; }
    bool isBuffed() { return _buff; }

    void setShield(bool s) { _shield = s; }
    bool hasShield() { return _shield; }

    bool skipToNext();
};

#endif // !__GP_PLAYER_H__

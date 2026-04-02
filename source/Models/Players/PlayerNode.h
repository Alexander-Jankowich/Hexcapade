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

class PlayerNode : public cugl::scene2::SpriteNode {
private:
    cugl::Vec2 _velocity;

    bool facingBack;
    bool facingLeft;
    
    static int leftFrontFrame;
    static int rightFrontFrame;
    static int leftBackFrame;
    static int rightBackFrame;
    static int deadFrame;

    static int cols;
    static int rows;

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
        PlayerNode::leftFrontFrame = consts->getInt("left_forward");
        PlayerNode::rightFrontFrame = consts->getInt("right_forward");
        PlayerNode::leftBackFrame = consts->getInt("left_backward");
        PlayerNode::rightBackFrame = consts->getInt("right_backward");
        PlayerNode::deadFrame = consts->getInt("dead");

        PlayerNode::cols = consts->getInt("columns");
        PlayerNode::rows = consts->getInt("rows");
    }
    
    void reset() {
        setPosition(getSize() / 2);
        setVelocity(cugl::Vec2::ZERO);
    }
    
    void updateAnimationState(PlayerObstacle::AnimationState s);

    void setVelocity(cugl::Vec2 value);
};

#endif // !__GP_PLAYER_H__

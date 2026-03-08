//
//  Player.h
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#ifndef __GP_PLAYER_H__
#define __GP_PLAYER_H__
#include <cugl/cugl.h>
#include "Entity.h"

class Player : public Entity {
private:
    float _speed;

public:
#pragma mark Constructors

    /**
     * Creates a player
     */
    Player(const cugl::Vec2& pos, float speed);

    /**
     * Disposes the ship, releasing all resources.
     */
    ~Player() {}


#pragma mark Properties
    /**
     * Returns the velocity of this ship.
     *
     * This value is necessary to control momementum in ship movement.
     *
     * @return the velocity of this ship
     */
    const int getSpeed() const { return _speed; }

    /**
     * Sets the velocity of this ship.
     *
     * This value is necessary to control momementum in ship movement.
     *
     * @param value the velocity of this ship
     */
    void setSpeed(int value) { _speed = value; }

    void setVelocity(cugl::Vec2 value) override;

    void updateAngle(float angle);

    float getAngle() { return _playerAngle; }

    cugl::Vec2 getDirection() { return cugl::Vec2(std::cos(_playerAngle), std::sin(_playerAngle)); }

protected:
    float _playerAngle;
};

#endif // !__GP_PLAYER_H__

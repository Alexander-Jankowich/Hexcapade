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
#include "Entity.h"

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing an alien ship.
 */
class Enemy : public Entity {
private:
    float _speed;

public:
#pragma mark Constructors
    /**
     * Creates a ship wiht the given position and data.
     *
     * The JsonValue should be a reference of all of the constants
     * that necessary to set the "hidden physical properties".
     *
     * @param pos   The ship position
     * @param data  The data defining the physics constants
     */
    Enemy(const cugl::Vec2& pos, float speed, int health, std::shared_ptr<cugl::graphics::Texture>& tex);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Enemy() {}

    /**
     * Returns the speed of this enemy.
     *
     * @return the speed of this enemy
     */
    const int getSpeed() const { return _speed; }

    /**
     * Sets the speed of this enemy.
     *
     * @param value the speed of this enemy
     */
    void setSpeed(int value) { _speed = value; }

    bool update(cugl::Vec2 target);
    
#pragma mark Graphics
    /**
     * Draws this ship to the sprite batch within the given bounds.
     *
     * This drawing code supports "wrap around". If the ship is partly off of
     * one edge, then it will also be drawn across the edge on the opposite
     * side.
     *
     * @param batch     The sprite batch to draw to
     * @param size      The size of the window (for wrap around)
     */
    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);
};

#endif // !__GP_ENEMY_H__


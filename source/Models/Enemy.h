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

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing an alien ship.
 */
class Enemy {
private:
    /** Position of the enemy */
    cugl::Vec2 _pos;
    /** Speed of the enemy */
    int _speed;
    
    bool _frozen;
    int _frozen_counter;
        
    // The following are protected, because they have no accessors
    /** The amount of health this enemy has */
    int _health;
    
    // Asset references. These should be set by GameScene
    /** Reference to the ships sprite sheet */
    std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;
    /** Radius of the ship in pixels (derived from sprite sheet) */
    float _radius;
    
    cugl::Vec2 _knockback_vel;
    bool _knockback;
    int _knockback_counter;
    bool _alive;

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
    Enemy(const cugl::Vec2& pos, int health);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Enemy() {}

    
#pragma mark Properties
    /**
     * Returns the position of this ship.
     *
     * This is location of the center pixel of the ship on the screen.
     *
     * @return the position of this ship
     */
    const cugl::Vec2& getPosition() const { return _pos; }
    
    /**
     * Sets the position of this ship.
     *
     * This is location of the center pixel of the ship on the screen. Setting
     * this value does NOT respect wrap around. It is possible to use this
     * method to place the ship off screen (so be careful).
     *
     * @param value the position of this ship
     */
    void setPosition(cugl::Vec2 value) { _pos = value; }
    
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
    
    /**
     * Returns the current ship health.
     * 
     * When the health of the ship is 0, it is "dead"
     *
     * @return the current ship health.
     */
    int getHealth() const { return _health; }

    /**
     * Sets the current ship health.
     * 
     * When the health of the ship is 0, it is "dead"
     *
     * @param value The current ship health.
     */
    void setHealth(int value);

    bool isAlive() const { return _alive; }

    /**
     * Returns the radius of the ship.
     *
     * This value is necessary to resolve collisions. It is computed from
     * the sprite sheet.
     *
     * @return the ship radius
     */
    float getRadius() {
        return _radius;
    }

    void update(cugl::Vec2 target);
    
#pragma mark Graphics
    const std::shared_ptr<cugl::graphics::Texture>& getTexture() const {
        return _texture;
    }

    /**
     * Sets the texture for this ship.
     *
     * The texture should be formated as a sprite sheet, and the size and 
     * layout of the sprite sheet should already be specified in the 
     * initializing JSON. If so, this method will construct a sprite sheet
     * from this texture. Otherwise, the texture will be ignored.
     *
     * @param texture   The texture for the sprite sheet
     */
    void setTexture(const std::shared_ptr<cugl::graphics::Texture>& texture);
    
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
    
    void setKnockback(cugl::Vec2 target);
    
    void setFrozen(){_frozen=true;}
    
private:
    std::shared_ptr<cugl::graphics::Texture> _texture;

};

#endif // !__GP_ENEMY_H__


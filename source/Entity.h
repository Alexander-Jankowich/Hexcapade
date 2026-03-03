//
//  Entity.h
//  Hexcapade
//
//  This class tracks all of the state (position, velocity, rotation) of a
//  single entity.
//
//  Author: Kaz Tam
//  Version: 3/02/26
//
#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <cugl/cugl.h>

/**
 * Model class representing an entity.
 */
class Entity {
protected:
    /** Position of entity */
    cugl::Vec2 _pos;

    /** Velocity of entity */
    cugl::Vec2 _velocity;

    /** Variables to track if entity is frozen and for how long */
    bool _frozen;
    int _frozen_counter;

    /** The amount of health this entity has */
    int _health;

    // Asset references. These should be set by GameScene
    /** Reference to the ships sprite sheet */
    std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;

    /** Radius of the ship in pixels (derived from sprite sheet) */
    cugl::Size _size;

    cugl::Vec2 _knockback_vel;
    bool _knockback;
    int _knockback_counter;

    std::shared_ptr<cugl::graphics::Texture> _texture;
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
    Entity(const cugl::Vec2& pos, int health);

    /**
     * Disposes the ship, releasing all resources.
     */
    ~Entity() {}


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
    const cugl::Vec2 getVelocity() const { return _velocity; }

    /**
     * Sets the velocity of this ship.
     *
     * This value is necessary to control momementum in ship movement.
     *
     * @param value the velocity of this ship
     */
    virtual void setVelocity(cugl::Vec2 value) { _velocity = value; }

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

    bool isAlive() const { return _health > 0; }

    /**
     * Returns the radius of the ship.
     *
     * This value is necessary to resolve collisions. It is computed from
     * the sprite sheet.
     *
     * @return the ship radius
     */
    cugl::Size getSize() { return _size; }

    virtual void update();

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

    void setFrozen() { _frozen = true; }
};

#endif // !__GP_ENEMY_H__


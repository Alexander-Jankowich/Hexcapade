//
//  Player.h
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#ifndef __GP_PLAYER_H__
#define __GP_PLAYER_H__
#include <cugl/cugl.h>

class Player {
private:
    /** Position of the player */
    cugl::Vec2 _pos;
    /** Speed of the player */
    int _speed;
        
    // The following are protected, because they have no accessors
    /** The amount of health this player has */
    int _health;
    
    // Asset references. These should be set by GameScene
    /** Reference to the players? sprite sheet */
    std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;
    /** Radius of the ship in pixels (derived from sprite sheet) */
    float _radius;
    
    /** the transformation for the player shape */
    cugl::Affine2 _playerTrans;
    
    cugl::Poly2 _polyPlayer;



public:
    /** rn this is the player shape lol, fuck it its public i cant be assed to do a getter */
    std::shared_ptr<cugl::physics2::PolygonObstacle> _shape;
    /** current angle the player is pointed towards */
    float _playerAngle;
#pragma mark Constructors
    
    /**
     * Creates a player
     *
     *
     */
    Player(const cugl::Vec2& pos);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~Player() {}

    
#pragma mark Properties
    void resetTrans(){_playerTrans=cugl::Affine2::IDENTITY;}
    void setPoly(cugl::Poly2 p) {_polyPlayer = p;}
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
    void setHealth(int value) {_health = value;};

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
    
    void updateAngle(float angle);
    
    void postUpdate(float remain);
    
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
    
private:
    std::shared_ptr<cugl::graphics::Texture> _texture;
    
};

#endif // !__GP_PLAYER_H__

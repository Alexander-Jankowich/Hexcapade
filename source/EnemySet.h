//
//  EnemySet.h
//
#ifndef __ENEMY_SET_H__
#define __ENEMY_SET_H__
#include <cugl/cugl.h>

class EnemySet {
public:
    class Enemy {
    private:
        /** Position of the enemy */
        cugl::Vec2 _pos;
        /** Speed of the enemy */
        int _speed;

        bool _frozen;
        int _frozen_counter;

        /** The amount of health this enemy has */
        int _health;

        cugl::Vec2 _knockback_vel;
        bool _knockback;
        int _knockback_counter;

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

        /**
        * Returns true if enemy is alive
        */
        bool update(cugl::Vec2 target);

        void setKnockback(cugl::Vec2 target);

        void setFrozen() { _frozen = true; }
    };

    std::unordered_set<std::shared_ptr<Enemy>> current;
    
    EnemySet();

    void dispose();

    void reset();
    
#pragma mark Graphics
    void setTexture(const std::shared_ptr<cugl::graphics::Texture>& value);

    void spawnEnemy(const cugl::Vec2 position, int health);

    void update(cugl::Vec2 target);

    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

    float getRadius() { return _radius; }

private:
    float _radius;
    std::shared_ptr<cugl::graphics::Texture> _texture;
};
#endif // !__ENEMY_SET_H__

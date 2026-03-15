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
#include "SpellSet.h"

class Player : public Entity {
private:
    float _speed;
    float _playerAngle;
    SpellType _curSpell;
    std::string _playerId; // player identifier, rn its the uuid

    // Counter for number of spells player has fired, used for spell id generation
    unsigned int _curSpellNum;
    
    // temp for now
    int _inventoryVal;

public:
#pragma mark Constructors

    /**
     * Creates a player
     */
    Player(const cugl::Vec2& pos, float speed, std::string player_id, const std::shared_ptr<cugl::graphics::Texture>& t);
    
    Player(const cugl::Vec2& pos, float speed); // need both


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
    
    void addItem(int val) {_inventoryVal+=val;}
    int getInventoryValue() {return _inventoryVal;}
    
    const std::string getPID() const {return _playerId;}
    void setPID(std::string pid) {_playerId = pid;}
    
    void reset() {
        setHealth(100);
        setPosition(getSize() / 2);
        setVelocity(cugl::Vec2::ZERO);
    }

    /**
     * Sets the max speed of the player
     *
     * @param value the max speed of this player
     */
    void setSpeed(int value) { _speed = value; }

    void setVelocity(cugl::Vec2 value) override;

    void updateAngle(float angle);

    float getAngle() { return _playerAngle; }

    cugl::Vec2 getDirection() { return cugl::Vec2(std::cos(_playerAngle), std::sin(_playerAngle)); }

    /**
    * Sets the current spell of the player.
    * Also increments _curSpellNum if this spell is different than the previous one.
    */
    void setSpell(SpellType spell);
    void resolveSpell() { _curSpell = SpellType::NONE; }

    SpellType getSpell() { return _curSpell; }

    const std::string getSpellId() const;
};

#endif // !__GP_PLAYER_H__

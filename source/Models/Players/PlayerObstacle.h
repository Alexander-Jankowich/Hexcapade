//
//  PlayerObstacle.h
//  tech_final
//
//  Created by Serena Duncan on 3/18/26.
//

#ifndef __PLAYER_OBSTACLE_H__
#define __PLAYER_OBSTACLE_H__

#include <cugl/cugl.h>
#include "../Spells/SpellType.h"
#include "../Items/Artifact.h"

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

#define MAX_HEALTH 6

/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class PlayerObstacle : public physics2::CapsuleObstacle {
    /** enum for animation state bc its impacted by collisions (haha pun)*/
public:
    /** Category stored on each carried item. */
    enum class ItemKind {
        ARTIFACT,
        SPELLBOOK
    };

    struct CarriedItem {
        std::string itemType;
        int value;
        ItemKind kind;
        int bombTimer;
        ArtifactEffect effect;

        static ItemKind kindFromType(const std::string& t);
    };
    
protected:
    std::string _pid;

    float _speed;

    SpellType _curSpell = SpellType::NONE;
    bool _releasedSpell = false;

    /** Velocity of entity */
    cugl::Vec2 _velocity;

    /** The amount of health this entity has */
    int _health;

    /** The angle the enity is facing */
    cugl::Vec2 _angle;

    /** carried items for the current pickup/deposit flow */
    vector<CarriedItem> _inventory;

    /** max carried item count for the current inventory flow */
    size_t _inventoryLimit;
    bool _frozen = false;
    int _frozenTimer = 0;

    bool _mud = false;
    int _mudTimer = 0;
    
    bool _buff = false;
    int _buffTimer = 0;

    bool _hurt = false;
    
    bool _shield = false;
    
    int _shieldCount = 0;

public:
    PlayerObstacle(Vec2 pos, string pid, cugl::Size s);

    static shared_ptr<PlayerObstacle> alloc(Vec2 pos, string pid, cugl::Size& s) {
        auto f = make_shared<PlayerObstacle>(pos, pid, s);
        f -> createFixtures();
        return f;
    }
    bool _bombExpired = false;

#pragma mark PO_Getters_and_Setters
    int getHealth() {return _health;}

    cugl::Vec2 getDirection() { return _angle; }

    void setHealth(int h);
    bool isHurt() { return _hurt; }
    void setHurt(bool h) { _hurt = h; }

    void setSpeed(float s) { _speed = s; }
    float getSpeed() {
        float base = hasHeavyArtifact() ? _speed * 0.7f : _speed;
        return (_aimSlowTimer <= 0.0f) ? base * 0.5f : base;
    }

    float _aimSlowTimer = 2.0f;
    void setAimSlowTimer(float t) { _aimSlowTimer = t; }
    float getAimSlowTimer() const { return _aimSlowTimer; }

    void setMud(bool m) {_mud = m;}
    bool getMud() {return _mud;}
    void setMudTimer(int i) { _mudTimer = i; }

    void setFrozen(bool f) { _frozen = f; }
    bool isFrozen() { return _frozen; }
    void setFrozeTimer(int i) {_frozenTimer = i;}
    
    void setShield(bool s) {_shield = s; }
    bool hasShield() { return _shield; }
    void setShieldCount(int c) { _shieldCount = c; }
    int getShieldCount() { return _shieldCount; }

    bool getBuff() {return _buff;}
    void setBuff(bool b) {_buff = b;}
    void setBuffTimer(int i) {_buffTimer =i;}

    void setReleasedSpell(bool released) { _releasedSpell = released; }
    bool hasReleasedSpell() { return _releasedSpell; }
    
    void setVelocity(Vec2 v);
    cugl::Vec2 getVelocity() {return _velocity;}

    /**
    * Sets the current spell of the player.
    */
    void setSpell(SpellType spell) {_curSpell = spell;}
    void resolveSpell() { setSpell(SpellType::NONE); }
    bool isCasting() { return _curSpell != SpellType::NONE; }
    SpellType getSpell() { return _curSpell; }

    const std::string getPID() const { return _pid; }
    void setPID(std::string pid) { _pid = pid; }
    
#pragma mark PO_Inventory
    int getInventoryVal() const;
    const vector<CarriedItem>& getInventory() const { return _inventory; }
    size_t getInventoryCount() const { return _inventory.size(); }
    size_t getInventoryLimit() const { return _inventoryLimit; }
    void setInventoryLimit(size_t limit) { _inventoryLimit = limit; }
    bool canAddItem() const { return _inventory.size() < _inventoryLimit; }
    bool hasInventory() const { return !_inventory.empty(); }
    void clearInventory() { _inventory.clear(); }
    int  depositItems();
    int  depositItem(size_t index);
    bool addItem(const std::string& itemType, int item_val);

    // ── Inventory query methods ──────────────────────────────────────────────
    // These mirror the Inventory class queries but work directly on CarriedItem.
    // Effect checks use the itemType string so no separate enum is needed. NOT USED YET.

    bool hasSpell(SpellType spell) const;

    bool blocksSpellcasting()    const;
    int  getDamageBonus()        const;
    bool hasProtectionArtifact() const;
    void consumeItem(const std::string& itemType);
    bool hasHeavyArtifact()      const;
    void updateTimers(float dt);
    bool isNonDroppable(size_t index) const;
};

#endif // !__PLAYER_OBSTACLE_H__


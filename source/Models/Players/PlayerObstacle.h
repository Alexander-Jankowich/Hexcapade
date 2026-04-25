//
//  PlayerObstacle.h
//  tech_final
//
//  Created by Serena Duncan on 3/18/26.
//

#ifndef __PLAYER_OBSTACLE_H__
#define __PLAYER_OBSTACLE_H__

#include "../Spells/SpellFactory.h"

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

        static ItemKind kindFromType(const std::string& t) {
            return (t.rfind("spellbook", 0) == 0) ? ItemKind::SPELLBOOK : ItemKind::ARTIFACT;
        }
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
    vector<CarriedItem> _inventory = { { "spellbook_fire", /*value*/0, CarriedItem::kindFromType("spellbook_fire"), /*bombTimer*/-1 } };

    /** max carried item count for the current inventory flow */
    size_t _inventoryLimit;
    bool _frozen = false;
    int _frozenTimer = 0;

    bool _mud = false;
    int _mudTimer = 0;
    
    bool _buff = false;
    int _buffTimer = 0;

    bool _hurt = false;

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
    void setHealth(int h);
    bool isHurt() { return _hurt; }
    void setHurt(bool h) { _hurt = h; }

    void setSpeed(float s) { _speed = s; }
    float getSpeed() { return hasHeavyArtifact() ? _speed * 0.5f : _speed; }

    void setMud(bool m) {_mud = m;}
    bool getMud() {return _mud;}
    void setMudTimer(int i) { _mudTimer = i; }

    void setFrozen(bool f) { _frozen = f; }
    bool isFrozen() { return _frozen; }
    void setFrozeTimer(int i) {_frozenTimer = i;}

    bool getBuff() {return _buff;}
    void setBuff(bool b) {_buff = b;}
    void setBuffTimer(int i) {_buffTimer =i;}
    
    cugl::Vec2 getDirection() { return _angle; }
    #pragma mark PO_Inventory
    int getInventoryVal() const {
        int sum = 0;
        for (const auto& item : _inventory) sum += item.value;
        return sum;
    }

    void setReleasedSpell(bool released) { _releasedSpell = released; }
    bool hasReleasedSpell() { return _releasedSpell; }
    
    const vector<CarriedItem>& getInventory() const { return _inventory; }
    size_t getInventoryCount() const { return _inventory.size(); }
    size_t getInventoryLimit() const { return _inventoryLimit; }
    void setInventoryLimit(size_t limit) { _inventoryLimit = limit; }
    bool canAddItem() const { return _inventory.size() < _inventoryLimit; }
    bool hasInventory() const { return !_inventory.empty(); }
    void clearInventory() { _inventory.clear(); }
    int depositItems() {
        int deposited = getInventoryVal();
        _inventory.clear();
        return deposited;
    }
    int depositItem(size_t index) {
        if (index >= _inventory.size()) {
            return 0;
        }
        int deposited = _inventory[index].value;
        _inventory.erase(_inventory.begin() + index);
        return deposited;
    }

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
    
    bool addItem(const std::string& itemType, int item_val) {
        if (!canAddItem()) {
            return false;
        }
        int timer = (itemType == "artifact_bomb") ? 3600 : -1;
        _inventory.push_back({ itemType, item_val, CarriedItem::kindFromType(itemType), timer});
        return true;
    }

    bool hasSpell(SpellType spell) const {
        std::vector<string> type;
        switch (spell) {
        case SpellType::FIREBALL:
        case SpellType::COMBO_FIREBALL:
            type.push_back("spellbook_fire");
            break;
        case SpellType::FREEZE:
        case SpellType::COMBO_FREEZE:
            type.push_back("spellbook_freeze");
            break;
        case SpellType::HEAL:
            type.push_back("spellbook_heal");
            break;
        case SpellType::EARTH:
            type.push_back("spellbook_earth");
            break;
        case SpellType::DEATH:
            type.push_back("spellbook_death");
            break;
        case SpellType::COMBO_EFREEZE:
            type.push_back("spellbook_earth");
            type.push_back("spellbook_freeze");
            break;
        case SpellType::WATER:
            type.push_back("spellbook_water");
            break;
        default:
            return false;
        }
        for (const auto& item : _inventory) {
            for (const auto& t : type){
                if (item.kind == ItemKind::SPELLBOOK && item.itemType == t) {
                    return true;
                }
            }
            
        }
        return false;
    }
    // ── Inventory query methods ──────────────────────────────────────────────
    // These mirror the Inventory class queries but work directly on CarriedItem.
    // Effect checks use the itemType string so no separate enum is needed. NOT USED YET.

    bool blocksSpellcasting() const {
        for (const auto& item : _inventory) {
            if (item.kind == ItemKind::ARTIFACT &&
                item.itemType == "artifact_scroll")
                return true;
        }
        return false;
    }

    int getDamageBonus() const {
        int bonus = 0;
        for (const auto& item : _inventory) {
            if (item.kind == ItemKind::ARTIFACT) {
                if (item.itemType == "artifact_boost")
                    bonus += 2;
                else if (item.itemType == "artifact_reduce")
                    bonus -= 2;
            }
        }
        return bonus;
    }

    ///**
    // * Returns true if the player carries a protection artifact
    // * (absorbs one lethal hit). Keyed on itemType == "artifact_protection".
    // */
    bool hasProtectionArtifact() const {
        for (const auto& item : _inventory) {
            if (item.kind == ItemKind::ARTIFACT &&
                item.itemType == "artifact_protection")
                return true;
        }
        return false;
    }

    void consumeProtectionArtifact() {
        for (auto it = _inventory.begin(); it != _inventory.end(); ++it) {
            if (it->kind == ItemKind::ARTIFACT && it->itemType == "artifact_protection") {
                _inventory.erase(it);
                return;
            }
        }
    }

    bool hasHeavyArtifact() const {
        for (const auto& item : _inventory) {
            if (item.kind == ItemKind::ARTIFACT &&
                item.itemType == "artifact_heavy")
                return true;
        }
        return false;
    }
    
    void updateTimers(float dt){
        if (isFrozen()) {
            _frozenTimer -= 1;
            if (_frozenTimer <= 0) {
                _frozenTimer = 0;
                _frozen = false;
            }
            if (getBody()) {
                getBody()->SetLinearVelocity(b2Vec2(0, 0));
            }
        }
        if (_mud) {
            _mudTimer -= 1;
            if(_mudTimer <= 0){
                _mudTimer = 0;
                _mud = false;
            }
        }
        if(_buff){
            _buffTimer -= 1;
            if(_buffTimer <= 0){
                _buffTimer = 0;
                _buff = false;
            }
        }

        //bomb erase
        for (size_t i = 0; i < _inventory.size(); ++i) {
            auto& item = _inventory[i];
            if (item.bombTimer < 0) continue;
            item.bombTimer -= 1;
            if (item.bombTimer <= 0) {
                _inventory.erase(_inventory.begin() + i);
                _bombExpired = true;
                setHealth(_health - 2);
                break;
            }
        }
    }

    bool isNonDroppable(size_t index) const {
        if (index >= _inventory.size()) return false;
        const std::string& t = _inventory[index].itemType;
        return t == "artifact_bomb" || t == "artifact_gold";
    }


    ///**
    // * Returns true if the player carries a healing artifact
    // * (passive HP regen). Keyed on itemType == "artifact_healing".
    // */
    //bool hasHealingArtifact() const {
    //    for (const auto& item : _inventory) {
    //        if (item.kind == ItemKind::ARTIFACT &&
    //            item.itemType == "artifact_healing")
    //            return true;
    //    }
    //    return false;
    //}

    ///**
    // * Returns all artifact itemType strings currently in inventory.
    // * Useful for effect-dispatch loops without exposing internals.
    // */
    //std::vector<std::string> getActiveArtifactTypes() const {
    //    std::vector<std::string> types;
    //    for (const auto& item : _inventory) {
    //        if (item.kind == ItemKind::ARTIFACT)
    //            types.push_back(item.itemType);
    //    }
    //    return types;
    //}
};

#endif // !__PLAYER_OBSTACLE_H__


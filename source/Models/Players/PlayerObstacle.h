//
//  PlayerObstacle.h
//  tech_final
//
//  Created by Serena Duncan on 3/18/26.
//
#include "../Spells.h"

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
    enum class AnimationState : int {
        /** moving forward */
        WALKING = 0,
        /** not moving */
        IDLE = 1,
        /** casting spell */
        CASTING = 2,
        /** hit by enemy idk */
        ATTACKED = 3,
        /** frozen  */
        FROZEN = 4,
        DEAD = 5
    };

    /** Category stored on each carried item. */
    enum class ItemKind {
        ARTIFACT,
        SPELLBOOK
    };

    struct CarriedItem {
        std::string itemType;
        int value;
        ItemKind kind;

        static ItemKind kindFromType(const std::string& t) {
            return (t.rfind("spellbook", 0) == 0) ? ItemKind::SPELLBOOK : ItemKind::ARTIFACT;
        }
    };
    
protected:

    AnimationState _state;
    std::string _pid;

    float _speed;
    SpellType _curSpell = SpellType::NONE;

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

public:
    PlayerObstacle(Vec2 pos, string pid, cugl::Size s);

    static shared_ptr<PlayerObstacle> alloc(Vec2 pos, string pid, cugl::Size& s) {
        auto f = make_shared<PlayerObstacle>(pos, pid, s);
        f -> createFixtures();
        return f;
    }

    #pragma mark PO_Getters_and_Setters
    int getHealth() {return _health;}
    float getSpeed() {return _speed;}
    AnimationState getState(){return _state;}
    void setAniState(AnimationState s) {_state = s;}
    
    cugl::Vec2 getDirection() { return _angle; }
    #pragma mark PO_Inventory
    int getInventoryVal() const {
        int sum = 0;
        for (const auto& item : _inventory) sum += item.value;
        return sum;
    }
    
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
    void resolveSpell() { _curSpell = SpellType::NONE; }
    void setHealth(int h);
    void setSpeed(float s) {_speed = s;}

    SpellType getSpell() { return _curSpell; }

    const std::string getPID() const { return _pid; }
    void setPID(std::string pid) { _pid = pid; }
    
    bool addItem(const std::string& itemType, int item_val) {
        if (!canAddItem()) {
            return false;
        }
        _inventory.push_back({ itemType, item_val, CarriedItem::kindFromType(itemType) });
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
        for (const auto& item : _inventory) {
            if (item.kind == ItemKind::ARTIFACT &&
                item.itemType == "artifact_gem")
                return 2;
        }
        return 0;
    }

    ///**
    // * Returns true if the player carries a protection artifact
    // * (absorbs one lethal hit). Keyed on itemType == "artifact_protection".
    // */
    //bool hasProtectionArtifact() const {
    //    for (const auto& item : _inventory) {
    //        if (item.kind == ItemKind::ARTIFACT &&
    //            item.itemType == "artifact_protection")
    //            return true;
    //    }
    //    return false;
    //}

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

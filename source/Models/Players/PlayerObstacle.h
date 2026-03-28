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

#define MAX_HEALTH 10

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

    struct CarriedItem {
        std::string itemType;
        int value;
    };
    
protected:

    AnimationState _state;
    std::string _pid;

    float _speed;
    SpellType _curSpell;

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
        _inventory.push_back({ itemType, item_val });
        return true;
    }

    bool addItem(int item_val) {
        if (!canAddItem()) {
            return false;
        }
        _inventory.push_back({ "", item_val });
        return true;
    }
};

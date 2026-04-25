//
//  PlayerObstacle.h
//  tech_final
//
//  Created by Serena Duncan on 3/18/26.
//

#ifndef __PLAYER_OBSTACLE_H__
#define __PLAYER_OBSTACLE_H__

#include "../Spells/SpellFactory.h"
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
        DEAD = 5,
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
        int bombTimer;
        ArtifactEffect effect;

        static ItemKind kindFromType(const std::string& t);
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
    int frozenTimer = 0;
    bool mud = false;
    int mudTimer = 0;
    bool buff = false;
    int buffTimer = 0;

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
    float getSpeed();
    AnimationState getState(){return _state;}
    void setAniState(AnimationState s){_state = s;}
    void setMud(bool m) {mud = m;}
    bool getMud() {return mud;}
    void setFrozeTimer(int i) {frozenTimer = i;}
    void setMudTimer(int i) {mudTimer = i;}
    bool getBuff() {return buff;}
    void setBuff(bool b) {buff = b;}
    void setBuffTimer(int i) {buffTimer =i;}
    
    cugl::Vec2 getDirection() { return _angle; }
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
    void setVelocity(Vec2 v);
    cugl::Vec2 getVelocity() {return _velocity;}

    /**
    * Sets the current spell of the player.
    */
    void setSpell(SpellType spell) {_curSpell = spell;}
    void resolveSpell() { setSpell(SpellType::NONE); }
    void setHealth(int h);
    void setSpeed(float s) {_speed = s;}

    SpellType getSpell() { return _curSpell; }

    const std::string getPID() const { return _pid; }
    void setPID(std::string pid) { _pid = pid; }
    
    bool addItem(const std::string& itemType, int item_val);

    // artifact stuff
    bool hasSpell(SpellType spell) const;
    bool blocksSpellcasting()    const;
    int  getDamageBonus()        const;
    bool hasProtectionArtifact() const;
    bool hasHeavyArtifact()      const;
    bool isNonDroppable(size_t index) const;

    void consumeProtectionArtifact();
    void updateTimers(float dt);


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


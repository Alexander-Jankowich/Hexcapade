//
//  PlayerObstacle.h
//  tech_final
//
//  Created by Serena Duncan on 3/18/26.
//
#include "SpellSet.h"
using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;


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
        FROZEN = 4
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

    vector<int> _inventory;

public:
    PlayerObstacle(Vec2 pos, string pid, cugl::Size s);

    static shared_ptr<PlayerObstacle> alloc(Vec2 pos, string pid, cugl::Size& s) {
        auto f = make_shared<PlayerObstacle>(pos, pid, s);
        return f;
    }

    #pragma mark PO_Getters_and_Setters
    int getHealth() {return _health;}
    float getSpeed() {return _speed;}
    AnimationState getState(){return _state;}
    void setAniState(AnimationState s) {_state = s;}
    
    cugl::Vec2 getDirection() { return _angle; }
    int getInventoryVal() {
        int sum = 0;
        for (auto i : _inventory) sum +=i;
        return sum;
    }

    void setVelocity(Vec2 v);

    /**
    * Sets the current spell of the player.
    */
    void setSpell(SpellType spell) {_curSpell = spell;}
    void resolveSpell() { _curSpell = SpellType::NONE; }
    void setHealth(int h) {_health = h;}
    void setSpeed(float s) {_speed = s;}

    SpellType getSpell() { return _curSpell; }

    const std::string getPID() const { return _pid; }
    void setPID(std::string pid) { _pid = pid; }
    
    void addItem(int item_val) {_inventory.push_back(item_val);}
};

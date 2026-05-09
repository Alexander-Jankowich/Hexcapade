//
//  SpellFactory.h
//  tech_final
//
//  Created by Kaz Tam on 3/16/26.
//

#ifndef __SPELL_FACTORY_H__
#define __SPELL_FACTORY_H__

#include <cugl/cugl.h>
#include "SpellNode.h"

#define HEAL_AMT 2;

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;


/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class SpellObstacle : public physics2::WheelObstacle {
protected:
    float _damage;
    std::string _caster;
    cugl::Vec2 _start;
    float _range;
    bool _atEnd;
    bool _weak;

public:
    const SpellType TYPE;
    SpellObstacle(const cugl::Vec2 pos, cugl::Vec2 vel, float radius, int dmg, float range, SpellType type, std::string c, bool weak);

    static std::shared_ptr<SpellObstacle> alloc(const cugl::Vec2 pos, cugl::Vec2 vel, float radius,
        int dmg, float range, SpellType type, std::string c, bool weak) {
        auto f = std::make_shared<SpellObstacle>(pos, vel, radius, dmg, range, type, c, weak);
        f->createFixtures();
        return f;
    }

    float getDamage() { return _damage; }
    std::string getCasterUID() {return _caster;}
    float getRange() { return _range; }
    float getTraveled() { return getPosition().distance(_start); }
    bool getWeak() { return _weak; }

    void setRemove() { _atEnd = true; }
    bool shouldRemove() { return _atEnd; }
};

/**
 * The factory class for crate objects.
 *
 * This class is used to support automatically syncing newly added obstacle mid-simulation.
 * Obstacles added throught the ObstacleFactory class from one client will be added to all
 * clients in the simulations.
 */
class SpellFactory : public ObstacleFactory {
protected:
    /** Pointer to the AssetManager for texture access, etc. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** Serializer for supporting parameters */
    cugl::netcode::NetcodeSerializer  _serializer;
    /** Deserializer for supporting parameters */
    cugl::netcode::NetcodeDeserializer _deserializer;

    std::shared_ptr<cugl::JsonValue> _spellConsts;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, Vec2 velocity, SpellType type, std::string c_uid, int damageBonus, bool buff, bool weak);


public:
    SpellFactory() {};

    /** Set of all current enemies */
    std::unordered_set<std::shared_ptr<SpellObstacle>> current;
    
    void reset() {current.clear();}
    /**
     * Allocates a new instance of the factory using the given AssetManager.
     */
    static std::shared_ptr<SpellFactory> alloc(std::shared_ptr<AssetManager>& assets) {
        auto f = std::make_shared<SpellFactory>();
        f->init(assets);
        return f;
    };

    /**
     * Initializes empty factories using the given AssetManager.
     */
    void init(std::shared_ptr<AssetManager>& assets) {
        _assets = assets;
        _spellConsts = _assets->get<JsonValue>("constants")->get("spells");
        SpellNode::setConsts(_spellConsts);
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    std::shared_ptr<std::vector<std::byte>> serializeParams(Vec2 pos, Vec2 velocity, SpellType type, std::string c, int damageBonus = 0, bool buff = false, bool weak = false);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(const std::vector<std::byte>& params) override;

    /**
    * Updates all spells
    * 
    * @return a set of all spells to be removed from the world
    */
    std::unordered_set<std::shared_ptr<SpellObstacle>> update();

    /**
    * The linking function for spell obstacles -> spell nodes
    */
    void linkFunc(const shared_ptr<SpellObstacle> obs, const shared_ptr<SpellNode> node, shared_ptr<Animator> animations);
};

#endif // !__SPELL_FACTORY_H__

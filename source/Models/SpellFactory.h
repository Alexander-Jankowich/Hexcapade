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

#define HEAL_AMT 3;
using namespace cugl;
using namespace cugl::physics2::distrib;

enum class SpellType {
    NONE=0,
    FREEZE=1,
    FIREBALL=2,
    HEAL=3,
    COMBO_FIREBALL=4,
    COMBO_FREEZE=5
};

/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class SpellObstacle : public physics2::WheelObstacle {
protected:
    float _damage;
    bool _froze;
    bool _healed;
    int _caster;

    cugl::Vec2 _start;
    float _range;

public:
    SpellObstacle(const cugl::Vec2 pos, cugl::Vec2 vel, int dmg, float range, bool freeze, bool healed, int c);

    static std::shared_ptr<SpellObstacle> alloc(const cugl::Vec2 pos,
        cugl::Vec2 vel, int dmg, float range, bool freeze, bool healed, int c) {
        auto f = std::make_shared<SpellObstacle>(pos, vel, dmg, range, freeze, healed, c);
        f->createFixtures();
        return f;
    }

    bool getHealed() { return _healed; }
    bool getFroze() { return _froze; }
    float getDamage() { return _damage; }
    int getCasterUID() {return _caster;}
    float getRange() { return _range; }
    float getTraveled() { return getPosition().distance(_start); }
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
    LWSerializer _serializer;
    /** Deserializer for supporting parameters */
    LWDeserializer _deserializer;

    std::shared_ptr<cugl::JsonValue> _spellConsts;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, Vec2 velocity, SpellType type, int c_uid);


public:
    SpellFactory() {};

    /** Set of all current enemies */
    std::unordered_set<std::shared_ptr<SpellObstacle>> current;

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
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    std::shared_ptr<std::vector<std::byte>> serializeParams(Vec2 pos, Vec2 velocity, SpellType type, int c);

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
};

#endif // !__SPELL_FACTORY_H__

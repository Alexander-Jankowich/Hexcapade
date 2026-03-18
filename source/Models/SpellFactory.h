//
//  SpellFactory.h
//  tech_final
//
//  Created by Kaz Tam on 3/16/26.
//

#ifndef __SPELL_FACTORY_H__
#define __SPELL_FACTORY_H__

#include <cugl/cugl.h>
#include "SpellSet.h"

using namespace cugl;
using namespace cugl::physics2::distrib;

//enum class SpellType {
//    NONE,
//    FREEZE,
//    FIREBALL,
//    HEAL
//};

/**
* An extension of WheelObstacle to represent spell collision objects.
*/
class SpellObstacle : public physics2::WheelObstacle {
protected:
    SpellType _type;

public:
    SpellObstacle(Vec2 pos, SpellType t);

    static std::shared_ptr<SpellObstacle> alloc(Vec2 pos, SpellType t) {
        auto f = std::make_shared<SpellObstacle>(pos, t);
        return f;
    }

    SpellType getType() { return _type; }
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

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(Vec2 pos, Vec2 velocity, SpellType type);


public:
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
    }

    /**
     * Helper method for converting normal parameters into byte vectors used for syncing.
     */
    std::shared_ptr<std::vector<std::byte>> serializeParams(Vec2 pos, Vec2 velocity, SpellType type);

    /**
     * Generate a pair of Obstacle and SceneNode using serialized parameters.
     */
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> createObstacle(const std::vector<std::byte>& params) override;
};

#endif // !__SPELL_FACTORY_H__

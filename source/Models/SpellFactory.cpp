//
//  SpellFactory.cpp
//  tech_final
//
//  Created by Kaz Tam on 3/16/26.
//

#include "SpellFactory.h"

using namespace std;
using namespace cugl;

SpellObstacle::SpellObstacle(const cugl::Vec2 pos, cugl::Vec2 vel, int dmg, bool freeze, bool heal) {
    init(pos, 10);
    damage = dmg;
    froze = freeze;
    healed = heal;
    setLinearVelocity(vel);
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> SpellFactory::createObstacle(Vec2 pos, Vec2 angle, SpellType type) {
    auto image = _assets->get<graphics::Texture>("spell");
    float radius(image->getSize().width / 2.f);
    
    // Create the SpellObstacle
    cugl::Vec2 norm = angle.getNormalization();
    std::shared_ptr<JsonValue> values;

    switch (type) {
    case SpellType::FIREBALL:
        values = _spellConsts->get("fireball");
        break;
    case SpellType::FREEZE:
        values = _spellConsts->get("freeze");
        break;
    case SpellType::HEAL:
        values = _spellConsts->get("heal");
        break;
    case SpellType::NONE:
        CULog("Creating non-existent spell!");
    default:
        CULog("Spell not implemented!!!! %d", (int) type);
    }

    Vec2 vel = norm * values->getFloat("speed");
    auto spell = SpellObstacle::alloc(pos, vel,
        values->getInt("damage"),
        values->getBool("freeze", false),
        values->getBool("heal", false));
    //auto spell = SpellObstacle::alloc(pos, type);

    spell->setAngleSnap(0); // Snap to the nearest degree

    // Set the physics attributes

    spell->setShared(true);

    // Create Spell! Hard code values for now, change later
    auto sprite = std::make_shared<SpellSet::Spell>(pos, vel,
        5, false, false, 1, 1,
        _assets->get<graphics::Texture>("spell"));

    switch (type) {
    case SpellType::FIREBALL:
        sprite->setColor(Color4::RED);
        break;
    case SpellType::FREEZE:
        sprite->setColor(Color4::BLUE);
        break;
    case SpellType::HEAL:
        sprite->setColor(Color4::GREEN);
        break;
    default:
        break;
    }

    return std::make_pair(spell, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
std::shared_ptr<std::vector<std::byte>> SpellFactory::serializeParams(Vec2 pos, Vec2 angle, SpellType type) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeFloat(angle.x);
    _serializer.writeFloat(angle.y);
    _serializer.writeUint16((uint16_t)type);
    return std::make_shared<std::vector<std::byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> SpellFactory::createObstacle(const std::vector<std::byte>& params) {
    /**
     * TODO: Use _deserializer to deserialize byte vectors packed by
     * {@link serializeParams()} and call the regular createObstacle() method
     * with them.
     */
    _deserializer.reset();
    _deserializer.receive(params);
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    float vx = _deserializer.readFloat();
    float vy = _deserializer.readFloat();
    Vec2 velocity = Vec2(vx, vy);
    SpellType type = (SpellType) _deserializer.readUint16();
    return createObstacle(pos, velocity, type);
}
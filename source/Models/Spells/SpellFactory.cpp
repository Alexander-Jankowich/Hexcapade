//
//  SpellFactory.cpp
//  tech_final
//
//  Created by Kaz Tam on 3/16/26.
//

#include "SpellFactory.h"

#define PHYSICS_SCALE 100.0f

using namespace std;
using namespace cugl;

SpellObstacle::SpellObstacle(const cugl::Vec2 pos, cugl::Vec2 vel, float radius, int dmg, float range ,SpellType type, std::string c) : TYPE(type) {    
    init(pos, radius / 3);
    
    setAwake(true);
    setEnabled(true);
    //setBullet(true);
    setBodyType(b2_dynamicBody);
    setAngle(vel.getAngle());
    setFixedRotation(true);
    
    _damage = dmg;
    

    _start = pos;
    _range = range;
    
    _caster = c;
    _atEnd = false;

    setLinearVelocity(vel);
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> SpellFactory::createObstacle(Vec2 pos, Vec2 angle, SpellType type, std::string c_uid, int damageBonus, bool buff) {
    auto image = _assets->get<graphics::Texture>("fireball"); // default?
    
    // Create the SpellObstacle
    cugl::Vec2 norm = angle.getNormalization();
    std::shared_ptr<JsonValue> values;

    switch (type) {
    case SpellType::WATER:
        CULog("this case");
        values = _spellConsts->get("water");
        image = _assets->get<graphics::Texture>("freeze");
        break;
    case SpellType::COMBO_FIREBALL:
        values = _spellConsts->get("combo_fireball");
        break;
    case SpellType::FIREBALL:
        values = _spellConsts->get("fireball");
        break;
    case SpellType::COMBO_FREEZE:
        values = _spellConsts->get("combo_freeze");
        image = _assets->get<graphics::Texture>("freeze");
        break;
    case SpellType::FREEZE:
        values = _spellConsts->get("freeze");
        image = _assets->get<graphics::Texture>("freeze");
        break;
    case SpellType::HEAL:
        values = _spellConsts->get("heal");
        image = _assets->get<graphics::Texture>("heal");
        break;
    case SpellType::DEATH:
        values =_spellConsts->get("death");
        image = _assets->get<graphics::Texture>("death");
        break;
    case SpellType::EARTH:
        values = _spellConsts->get("earth");
        image = _assets -> get<graphics::Texture>("freeze");
        break;
    case SpellType::COMBO_EFREEZE:
        values = _spellConsts->get("combo_efreeze");
        image =_assets -> get<graphics::Texture>("freeze");
        break;
    case SpellType::BUFF:
        values = _spellConsts->get("buff");
        image = _assets->get<graphics::Texture>("heal");
        break;
    case SpellType::NONE:
        CULog("Creating non-existent spell!");
    default:
        CULog("Spell not implemented!!!! %d", (int) type);
    }

    float radius = image->getHeight() / PHYSICS_SCALE;

    Vec2 vel = norm * values->getFloat("speed");
    auto spell = SpellObstacle::alloc(pos, vel, radius,
        std::max(1, buff ? 2 * (values->getInt("damage") + damageBonus) : values->getInt("damage") + damageBonus),
        values->getFloat("range"),type ,c_uid);
    CULog("buff is %d", buff);
    //auto spell = SpellObstacle::alloc(pos, type);

    spell->setAngleSnap(0); // Snap to the nearest degree

    // Set the physics attributes

    spell->setShared(true);
    CULog("going to emplace the spell");
    current.emplace(spell);

    // Create Spell! Hard code values for now, change later
    auto sprite = std::make_shared<SpellNode>(pos, vel, type, image);

    return std::make_pair(spell, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
std::shared_ptr<std::vector<std::byte>> SpellFactory::serializeParams(Vec2 pos, Vec2 angle, SpellType type, string c, int damageBonus, bool buff) {
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
    _serializer.writeUint32((uint32_t)type);
    _serializer.writeString(c);
    _serializer.writeSint32(damageBonus);
    _serializer.writeBool(buff);
   
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
    SpellType type = (SpellType) _deserializer.readUint32();
    string c = _deserializer.readString();
    int damageBonus = _deserializer.readSint32();
    int buff = _deserializer.readBool();
    return createObstacle(pos, velocity, type, c, damageBonus, buff);
}

std::unordered_set<std::shared_ptr<SpellObstacle>> SpellFactory::update() {
    std::unordered_set<std::shared_ptr<SpellObstacle>> remove;

    for (auto it = current.begin(); it != current.end();) {
        // Remove from set if removed from scene
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
            continue;
        }
        it++;
    }

    return remove;
}

void SpellFactory::linkFunc(const shared_ptr<SpellObstacle> obs, const shared_ptr<SpellNode> node, shared_ptr<Animator> animations) {
    // Mark spells to be removed when out of range
    if (obs->getTraveled() > obs->getRange()) {
        obs->setRemove();
    }

    if (obs->shouldRemove() && !node->shouldRemove()) {
        node->setRemove();
        obs->setLinearVelocity(Vec2::ZERO);
        animations->addNextListener(node, [obs](string key, float time, float actual) {
            obs->markRemoved(true);
        });
        animations->endAnimation(node);
    }

    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
    node->setPosition(pos * PHYSICS_SCALE);
    node->setAngle(angle);
}

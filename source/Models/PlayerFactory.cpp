//
//  PlayerFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//
#include "PlayerFactory.h"

using namespace std;
using namespace cugl;

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, s); // TODO WHAT THE FUCK SHOULD THIS BE
    _pid = pid;
}

void PlayerObstacle::setSpell(SpellType spell) {
    //if (spell != _curSpell && spell != SpellType::NONE) {
    //    _curSpellNum++;
    //}
    _curSpell = spell;
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> PlayerFactory::createObstacle(Vec2 pos, string pid) {
    
    auto image = _assets->get<graphics::Texture>("player"); // TODO this will need to update with additional metadata (which asset to get)
    auto size = image->getSize();
    
    // Create the SpellObstacle
    auto pl = PlayerObstacle::alloc(pos, pid, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
//    pl->setLinearVelocity(velocity);
    // which of those are needed...

    pl->setShared(true);

    current.emplace(pl);

    // Create player
    auto sprite = std::make_shared<Player>(pos, 12, pid, image);

    return std::make_pair(pl, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> PlayerFactory::serializeParams(Vec2 pos, string pid) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeString(pid);
    return make_shared<vector<byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> PlayerFactory::createObstacle(const vector<byte>& params) {
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
    string id =  _deserializer.readString(); // this is why i had to switch just this class to netcode de/serializers sorry abt that - serena
    return createObstacle(pos, id);
}

PlayerFactory::PlayerFactory() {
    current.clear();
}

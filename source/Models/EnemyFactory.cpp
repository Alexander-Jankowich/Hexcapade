//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"

using namespace std;
using namespace cugl;

EnemyObstacle::EnemyObstacle(Vec2 pos, Size s, int h) {
    init(pos, s);
    _health = h;
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(Vec2 pos, Vec2 velocity, int h) {
    
    auto image = _assets->get<graphics::Texture>("enemy"); // TODO this will need to update with additional metadata (which asset to get)
    Size size = image->getSize();
    
    // Create the SpellObstacle
    auto en = EnemyObstacle::alloc(pos, size, h);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(velocity);
    // which of those are needed...

    en->setShared(true);

    // Create enemy
    auto sprite = make_shared<Enemy>(pos, 2.5, h, image);

    return std::make_pair(en, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> EnemyFactory::serializeParams(Vec2 pos, Vec2 velocity, int h) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeFloat(velocity.x);
    _serializer.writeFloat(velocity.y);
    _serializer.writeUint32(h);
    return make_shared<vector<byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(const vector<byte>& params) {
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
    int health =  _deserializer.readUint32();
    return createObstacle(pos, velocity, health);
}

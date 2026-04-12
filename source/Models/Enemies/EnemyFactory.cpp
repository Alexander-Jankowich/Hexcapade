//
//  EnemyFactory.cpp
//  hexc
//
//  Created by Serena Duncan on 3/17/26.
//

#include "EnemyFactory.h"

using namespace std;
using namespace cugl;

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> EnemyFactory::createObstacle(EnemyType type, Vec2 pos) {
    
    auto image = _assets->get<graphics::Texture>("enemy"); // TODO this will need to update with additional metadata (which asset to get)
    Size size = image->getSize();
    
    // Create the EnemyObstacle

    auto en = EnemyObstacle::alloc(type, pos, _enemyConsts, size);

//    pl->setAngleSnap(0); // Snap to the nearest degree
//
//    // Set the physics attributes
    en->setLinearVelocity(Vec2::ZERO);
    // which of those are needed...

    en->setShared(true);

    current.emplace(en);

    // Create enemy
    auto sprite = make_shared<EnemyNode>(image);
    sprite -> setScale(0.5);

    return std::make_pair(en, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
shared_ptr<vector<byte>> EnemyFactory::serializeParams(EnemyType type, Vec2 pos) {
    /**
     * TODO: Use _serializer to serialize pos and scale (remember to make a
     * shared copy of the serializer reference, otherwise it will be lost if
     * the serializer is reset).
     */
    _serializer.reset();
    _serializer.writeUint16((uint16) type);
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
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
    EnemyType type = (EnemyType) _deserializer.readUint16();
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    return createObstacle(type, pos);
}


std::unordered_set<std::shared_ptr<EnemyObstacle>> EnemyFactory::getRemoved() {
    std::unordered_set<std::shared_ptr<EnemyObstacle>> remove;

    for (auto it = current.begin(); it != current.end(); ) {
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
        }
        else {
            it++;
        }
    }

    return remove;
}

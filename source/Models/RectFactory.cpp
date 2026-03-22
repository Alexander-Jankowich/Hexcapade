//
//  RectFactory.cpp
//  tech_final
//
//  Created by Serena Duncan on 3/22/26.
//

#include "RectFactory.h"
#define PHYSICS_SCALE 100

using namespace std;
using namespace cugl;

RectObstacle::RectObstacle(const cugl::Vec2 pos, float w, float h) {
    init(pos);
    setAwake(false);
    setBodyType(b2_staticBody);
    _pos = pos;
    _dimens = Size(w, h);
}

/**
 * Generate a pair of Obstacle and SceneNode using the given parameters
 */
pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> RectFactory::createObstacle(Vec2 pos, float w, float h, int t_gid) {
    auto obs = RectObstacle::alloc(pos, w, h);


    // Set the physics attributes
    obs->setShared(true);

    // Create Spell! Hard code values for now, change later
    cugl::Rect r = cugl::Rect(pos.x, pos.y, w, h);
    auto sprite = std::make_shared<cugl::scene2::PolygonNode>();
    sprite->initWithTexturePoly(_tiledData->getTextureForGid(t_gid), r);

    return std::make_pair(obs, sprite);
}

/**
 * Helper method for converting normal parameters into byte vectors used for syncing.
 */
std::shared_ptr<std::vector<std::byte>> RectFactory::serializeParams(Vec2 pos, float w, float h, int gid) {
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeFloat(w);
    _serializer.writeFloat(h);
    _serializer.writeUint32(gid);
    return std::make_shared<std::vector<std::byte>>(_serializer.serialize());
}

/**
 * Generate a pair of Obstacle and SceneNode using serialized parameters.
 */
std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> RectFactory::createObstacle(const std::vector<std::byte>& params) {
    _deserializer.reset();
    _deserializer.receive(params);
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    Vec2 pos = Vec2(x, y);
    float w = _deserializer.readFloat();
    float h = _deserializer.readFloat();
    int gid = _deserializer.readUint32();
    return createObstacle(pos, w, h, gid);
}

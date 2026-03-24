//
//  ItemFactory.h
//  tech_final
//
//  Created by Frank Xie on 3/23/26
//

#include "ItemFactory.h"

#define PHYSICS_SCALE 100
// Item half-size in physics units
#define ITEM_SIZE 0.48f

using namespace std;
using namespace cugl;

ItemObstacle::ItemObstacle(const cugl::Vec2& pos, int value, const std::string& itemType) {
    init(pos, cugl::Size(ITEM_SIZE, ITEM_SIZE));
    setBodyType(b2_staticBody);
    setAwake(true);
    setEnabled(true);
    _value = value;
    _itemType = itemType;
}

pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> ItemFactory::createObstacle(Vec2 pos, int value, const string& itemType) {
    auto item = ItemObstacle::alloc(pos, value, itemType);
    item->setShared(true);

    current.emplace(item);

    // Grab the item's texture based on itemType 
    auto texture = _assets->get<graphics::Texture>(itemType);
    auto sprite = std::make_shared<scene2::PolygonNode>();
    if (texture) {
        float halfPx = ITEM_SIZE * PHYSICS_SCALE * 0.5f;
        cugl::Rect rect(-halfPx, -halfPx, halfPx * 2.0f, halfPx * 2.0f);
        sprite->initWithTexturePoly(texture, rect);
    }

    return std::make_pair(item, sprite);
}

shared_ptr<vector<byte>> ItemFactory::serializeParams(Vec2 pos, int value, const string& itemType) {
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeSint32(value);
    _serializer.writeString(itemType);
    return make_shared<vector<byte>>(_serializer.serialize());
}

pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> ItemFactory::createObstacle(const vector<byte>& params) {
    _deserializer.reset();
    _deserializer.receive(params);
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    int   value = _deserializer.readSint32();
    string itemType = _deserializer.readString();
    return createObstacle(Vec2(x, y), value, itemType);
}

unordered_set<shared_ptr<ItemObstacle>> ItemFactory::update() {
    unordered_set<shared_ptr<ItemObstacle>> remove;

    for (auto it = current.begin(); it != current.end(); ) {
        if ((*it)->isRemoved()) {
            remove.emplace(*it);
            it = current.erase(it);
        }
        else {
            ++it;
        }
    }

    return remove;
}
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

ItemObstacle::ItemObstacle(const cugl::Vec2& pos, int value, const std::string& itemType, int i) {
    init(pos, cugl::Size(ITEM_SIZE, ITEM_SIZE));
    setBodyType(b2_staticBody);
    setEnabled(true);
    _value = value;
    _itemType = itemType;
    _itemID = i;
}

pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> ItemFactory::createObstacle(Vec2 pos, int value, const string& itemType, int i) {
    
    auto item = ItemObstacle::alloc(pos, value, itemType, i);
    item->setShared(true);

    current.insert(std::pair<int, std::shared_ptr<ItemObstacle>>(i, item));

    // Grab the item's texture based on itemType 
    auto texture = _assets->get<graphics::Texture>(itemType);

    std::shared_ptr<scene2::PolygonNode> sprite;
    if (texture) {
        sprite = scene2::PolygonNode::allocWithTexture(texture);
        sprite->setScale(0.25f);
    }
    else {
        // Fallback with solid colored rect
        sprite = scene2::PolygonNode::allocWithPoly(Rect(0, 0, 48, 48));
        sprite->setColor(Color4(255, 170, 60, 255));
    }
    
    return std::make_pair(item, sprite);
}

shared_ptr<vector<byte>> ItemFactory::serializeParams(Vec2 pos, int value, const string& itemType) {
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeSint32(value);
    _serializer.writeString(itemType);
    _serializer.writeUint32(_count);
    _count++;
    return make_shared<vector<byte>>(_serializer.serialize());
}

pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> ItemFactory::createObstacle(const vector<byte>& params) {
    _deserializer.reset();
    _deserializer.receive(params);
    float x = _deserializer.readFloat();
    float y = _deserializer.readFloat();
    int   value = _deserializer.readSint32();
    string itemType = _deserializer.readString();
    int i = _deserializer.readUint32();
    return createObstacle(Vec2(x, y), value, itemType, i);
}

unordered_set<shared_ptr<ItemObstacle>> ItemFactory::update() {
    unordered_set<shared_ptr<ItemObstacle>> remove;

    for (auto it = current.begin(); it != current.end(); ) {
        if ((*it).second->isRemoved()) {
            remove.emplace((*it).second);
            it = current.erase(it);
        }
        else {
            ++it;
        }
    }

    return remove;
}

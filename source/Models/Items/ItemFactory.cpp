//
//  ItemFactory.h
//  tech_final
//
//  Created by Frank Xie on 3/23/26
//

#include "ItemFactory.h"

#define PHYSICS_SCALE 50
// Item half-size in physics units
#define ITEM_SIZE 0.48f

using namespace std;
using namespace cugl;

using ItemType = ItemTypeClass::ItemType;

ItemObstacle::ItemObstacle(const cugl::Vec2& pos, int value, ItemType t, int i) {
    init(pos, cugl::Size(ITEM_SIZE, ITEM_SIZE));
    setBodyType(b2_staticBody);
    setEnabled(true);
    _value = value;
    _type = t;
    _itemID = i;
}

pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> ItemFactory::createObstacle(Vec2 pos, int value,
    float maxHeight, ItemType t, int i) {
    
    auto item = ItemObstacle::alloc(pos, value, t, i);
    item->setShared(true);

    current.insert(std::pair<int, std::shared_ptr<ItemObstacle>>(i, item));

    // Grab the item's texture based on itemType 
    auto texture = _itemInfo.getTexture(t);

    std::shared_ptr<scene2::PolygonNode> sprite;
    if (texture) {
        sprite = scene2::PolygonNode::allocWithTexture(texture);
        sprite->setScale(0.25f);
    }
    else {
        // Fallback with solid colored rect
        sprite = scene2::PolygonNode::allocWithPoly(Rect(0, 0, 48, 48));
        sprite->setColor(Color4(255, 170, 60, 255));
        CULog("ItemFactory: missing texture for itemType '%d'", t);
    }

    sprite->setPriority((maxHeight - pos.y) / maxHeight * 2);
    
    return std::make_pair(item, sprite);
}

shared_ptr<vector<byte>> ItemFactory::serializeParams(Vec2 pos, int value, float maxHeight, ItemType itemType) {
    _serializer.reset();
    _serializer.writeFloat(pos.x);
    _serializer.writeFloat(pos.y);
    _serializer.writeSint32(value);
    _serializer.writeFloat(maxHeight);
    _serializer.writeSint32((int) itemType);
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
    float maxHeight = _deserializer.readFloat();
    ItemType itemType = (ItemType) _deserializer.readSint32();
    int i = _deserializer.readUint32();
    return createObstacle(Vec2(x, y), value, maxHeight, itemType, i);
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

//
//  ItemFactory.h
//  tech_final
//
//  Created by Frank Xie on 3/23/26
//

#ifndef __ITEM_FACTORY_H__
#define __ITEM_FACTORY_H__

#include <cugl/cugl.h>
#include "ItemTypeClass.h"
using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;


/**
 * An extension of BoxObstacle to represent a world item (artifact or spellbook).
 */
class ItemObstacle : public physics2::BoxObstacle {
protected:
    /** Deposit value */
    int _value;
    ItemTypeClass::ItemType _type;
    int _itemID;

public:
    /** Constructs an ItemObstacle at the given position. */
    ItemObstacle(const cugl::Vec2& pos, int value, ItemTypeClass::ItemType t, int i);

    static std::shared_ptr<ItemObstacle> alloc(const cugl::Vec2& pos, int value, ItemTypeClass::ItemType t, int i) {
        auto f = std::make_shared<ItemObstacle>(pos, value, t, i);
        f->createFixtures();
        return f;
    }

    int getValue() const { return _value; }
    ItemTypeClass::ItemType getItemType() const { return _type; }
    int getID() const {return _itemID;}
};

/**
 * Factory for world item obstacles.
 */
class ItemFactory : public ObstacleFactory {
protected:
    // Need to use netcode serializer for string parsing - Frank

    /** Pointer to the AssetManager for texture access, etc. */
    shared_ptr<cugl::AssetManager> _assets;
    /** Serializer for supporting parameters */
    cugl::netcode::NetcodeSerializer _serializer;
    /** Deserializer for supporting parameters */
    cugl::netcode::NetcodeDeserializer _deserializer;
    int _count =0;
    ItemTypeClass _itemInfo;

    /**
     * Generate a pair of Obstacle and SceneNode using the given parameters
     */
    std::pair<std::shared_ptr<physics2::Obstacle>,std::shared_ptr<scene2::SceneNode>> createObstacle(cugl::Vec2 pos, int value, ItemTypeClass::ItemType t, int i);

public:
    ItemFactory() {};

    /** All currently live item obstacles in the world */
    std::map<int, std::shared_ptr<ItemObstacle>> current;
    
    void reset() {
        _count = 0;
        current.clear();
    }

    static std::shared_ptr<ItemFactory> alloc(std::shared_ptr<cugl::AssetManager>& assets) {
        auto f = std::make_shared<ItemFactory>();
        f->init(assets);
        return f;
    }

    void init(std::shared_ptr<cugl::AssetManager>& assets) {
        _assets = assets;
        _itemInfo = ItemTypeClass(assets);
    }
    
    std::shared_ptr<ItemObstacle> getItem(int i) {return current.contains(i) ? current.at(i) : nullptr;}

    std::shared_ptr<std::vector<std::byte>> serializeParams(cugl::Vec2 pos, int value, ItemTypeClass::ItemType itemType);
    
    std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>>createObstacle(const std::vector<std::byte>& params) override;

    std::unordered_set<std::shared_ptr<ItemObstacle>> update();
};

#endif // __ITEM_FACTORY_H__

//
//  ItemSet.h
//  Hexcapade
//
//  ItemSet owns the item instances that currently exist in world space. This
//  matches the architecture spec by separating world pickup state from both the
//  GameScene and each player's Inventory.
//
#ifndef __HEXCAPADE_ITEM_SET_H__
#define __HEXCAPADE_ITEM_SET_H__
#include <cugl/cugl.h>
#include <memory>
#include <random>
#include <vector>
#include "Item.h"

class ItemSet {
public:
    /**
     * One world item entry.
     *
     * The item payload stays shared_ptr-based so ownership can move cleanly
     * from world space into a player's Inventory and back out again on drop.
     */
    struct WorldItem {
        std::shared_ptr<Item> item;
        cugl::Vec2 position;
        float radius;
    };

private:
    /** Active pickup items that currently exist in the world. */
    std::vector<WorldItem> _items;
    /** Valid world area for temporary random spawns and clamped drops. */
    cugl::Rect _spawnBounds;
    /** Prototype RNG used until room/procgen-driven spawn data is ported. */
    std::mt19937 _rng;
    /** Default pickup overlap radius for prototype item circles. */
    float _defaultRadius;
    /** Simple counters used to generate stable placeholder ids. */
    int _nextArtifactIndex;
    int _nextSpellbookIndex;

public:
    ItemSet();

    /** Clears the world item list and resets the placeholder item counters. */
    void reset();

    /** Defines the playable bounds used by spawn and drop helpers. */
    void setSpawnBounds(const cugl::Rect& bounds);

    size_t size() const { return _items.size(); }
    const std::vector<WorldItem>& getItems() const { return _items; }

    /**
     * Spawns the initial prototype item set.
     *
     * // GA: itemnet
     * This is intentionally a temporary non-procgen helper for the networking
     * branch. Procedural room spawn plans should replace it later.
     */
    void spawnInitialPrototypeItems();

    /**
     * Spawns one extra prototype item.
     *
     * // GA: itemnet
     * This helper exists so the team can keep testing item networking before
     * the room/procgen pipeline is brought over.
     */
    void spawnRandomItem();

    /**
     * Inserts a dropped inventory item back into world space.
     *
     * @param item      The item being returned to the world.
     * @param position  Desired drop position near the player.
     * @param radius    Optional custom pickup radius. Negative uses default.
     */
    void addDroppedItem(const std::shared_ptr<Item>& item, const cugl::Vec2& position,
                        float radius = -1.0f);

    /**
     * Finds the nearest overlapping pickup candidate for the given player.
     *
     * @return the selected item index or -1 if none overlap.
     */
    int findNearestPickupIndex(const cugl::Vec2& playerPos, const cugl::Size& playerSize) const;

    /**
     * Removes one world item and returns ownership to the caller.
     *
     * @param index        Item index to remove.
     * @param outPosition  Optional world-space output of the removed item.
     *
     * @return the removed item or nullptr if the index was invalid.
     */
    std::shared_ptr<Item> takeAt(size_t index, cugl::Vec2* outPosition = nullptr);

private:
    /** Builds one temporary prototype item instance. */
    WorldItem createRandomItem(bool forceArtifact, bool forceSpellbook);
    /** Chooses a random point inside the current spawn bounds. */
    cugl::Vec2 createRandomPosition();
    /** Keeps a position inside the current spawn bounds. */
    cugl::Vec2 clampToBounds(const cugl::Vec2& pos) const;

    /** Utility collision helper used by the pickup query. */
    static bool overlapsRectCircle(const cugl::Vec2& rectCenter, const cugl::Size& rectSize,
                                   const cugl::Vec2& circleCenter, float radius);
};

#endif // __HEXCAPADE_ITEM_SET_H__

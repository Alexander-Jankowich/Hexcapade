//
//  Inventory.h
//  Hexcapade
//
//  Inventory stores the item state carried by a single player. This follows the
//  architecture document by keeping per-player carrying rules out of the scene
//  and out of the networking serializer itself.
//
#ifndef __HEXCAPADE_INVENTORY_H__
#define __HEXCAPADE_INVENTORY_H__
#include <cstddef>
#include <memory>
#include <vector>
#include "Artifact.h"
#include "SpellBook.h"

class Inventory {
private:
    /** Maximum number of items this player may carry at one time. */
    size_t _capacity;
    /** Ordered item slots shown by the HUD and inventory panel. */
    std::vector<std::shared_ptr<Item>> _items;

public:
    /**
     * Creates an empty inventory.
     *
     * The default of 4 matches the current design target for the prototype
     * networking branch.
     */
    explicit Inventory(size_t capacity = 4);

    /** Clears all carried items. Used on reset or host-authoritative resync. */
    void clear();

    size_t getCapacity() const { return _capacity; }
    size_t getCount() const { return _items.size(); }
    bool isEmpty() const { return _items.empty(); }
    bool canAddItem() const;

    /**
     * Adds one item to the next free inventory slot.
     *
     * @param item  The item being transferred from the world item set.
     *
     * @return true if the item was accepted.
     */
    bool addItem(const std::shared_ptr<Item>& item);

    /**
     * Removes the most recently added item.
     *
     * This wrapper exists because the older item branch declared it, and it is
     * convenient for keyboard/mobile quick-drop flows.
     */
    std::shared_ptr<Item> dropLastItem();

    /**
     * Removes the item at the given inventory index.
     *
     * @param index  The inventory slot to remove.
     *
     * @return the removed item or nullptr if the index was invalid.
     */
    std::shared_ptr<Item> dropItemAt(size_t index);

    /**
     * Deposits all currently carried items and returns their combined value.
     *
     * The host uses this value to update the shared team quota state.
     */
    int depositAllItems();

    /** Returns the quota value currently carried by this player. */
    int getCarriedValue() const;

    /** Placeholder gameplay queries preserved for future spell/artifact work. */
    bool hasSpell(SpellBook::Spell spell) const;
    bool blocksSpellcasting() const;
    float getDamageMultiplier() const;
    bool hasProtectionArtifact() const;
    std::vector<Artifact::Effect> getActiveArtifactEffects() const;

    /** Exposes the current ordered inventory contents for HUD rendering. */
    const std::vector<std::shared_ptr<Item>>& getItems() const { return _items; }
};

#endif // __HEXCAPADE_INVENTORY_H__

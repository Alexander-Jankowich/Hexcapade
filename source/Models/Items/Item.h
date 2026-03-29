//
//  Item.h
//  Hexcapade
//
//  Shared base class for every collectible item in the architecture-spec
//  item lifecycle. Concrete item types such as Artifact and SpellBook inherit
//  from this class so world items, inventories, UI, and networking snapshots
//  can all talk about a common item interface.
//
#ifndef __HEXCAPADE_ITEM_H__
#define __HEXCAPADE_ITEM_H__
#include <cugl/cugl.h>
#include <string>

class Item {
public:
    /**
     * High-level category used by inventory/UI/networking code.
     *
     * We keep this intentionally small for the first networking item pass.
     * Future item families can be added here without changing the shared
     * inventory container shape.
     */
    enum class Kind {
        ARTIFACT,
        SPELLBOOK
    };

protected:
    /** Stable identifier used for debugging, snapshots, and future data lookup. */
    std::string _id;
    /** Prototype display name shown in the HUD/inventory panel. */
    std::string _name;
    /** Quota value contributed when this item is deposited at origin. */
    int _value;
    /** Runtime discriminator so callers can branch without RTTI-heavy code. */
    Kind _kind;

public:
    /**
     * Initializes the item fields shared by all concrete item types.
     *
     * @param id     Stable item identifier.
     * @param name   Prototype display name.
     * @param value  Deposit value contributed to the shared quota.
     * @param kind   Concrete family represented by this item.
     */
    Item(std::string id, std::string name, int value, Kind kind);

    /** Virtual destructor so derived item classes clean up correctly. */
    virtual ~Item() = default;

    const std::string& getId() const { return _id; }
    const std::string& getName() const { return _name; }
    int getValue() const { return _value; }
    Kind getKind() const { return _kind; }

    /**
     * Returns the debug color used by the temporary prototype renderer.
     *
     * This is intentionally part of the item interface because the current
     * networking branch still renders items as colored circles instead of
     * textured pickups.
     */
    virtual cugl::Color4 getDebugColor() const = 0;
};

#endif // __HEXCAPADE_ITEM_H__

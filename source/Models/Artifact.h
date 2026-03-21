//
//  Artifact.h
//  Hexcapade
//
//  Concrete collectible item used by the architecture-spec inventory system.
//  Artifact gameplay effects are still placeholder-only in this networking pass;
//  we keep the API in place so later spell/combat integration can grow without
//  rewriting the item/inventory structure.
//
#ifndef __HEXCAPADE_ARTIFACT_H__
#define __HEXCAPADE_ARTIFACT_H__
#include "Item.h"

class Artifact : public Item {
public:
    /**
     * Placeholder artifact effects for the current prototype.
     *
     * // GA: itemnet
     * Real effect types can replace these placeholders after the networking
     * item lifecycle is stable across desktop and mobile.
     */
    enum class Effect {
        ARTIFACT_EFFECT
    };

private:
    /** Which artifact effect this pickup carries. */
    Effect _effect;

public:
    /**
     * Creates one artifact instance.
     *
     * @param id      Stable item identifier.
     * @param name    Display name for UI/debug rendering.
     * @param value   Shared quota contribution on deposit.
     * @param effect  Placeholder artifact effect metadata.
     */
    Artifact(std::string id, std::string name, int value, Effect effect);

    Effect getEffect() const { return _effect; }

    /**
     * Placeholder behavior helpers kept from the original item branch.
     *
     * These helpers let inventory/combat code query artifact behavior without
     * needing to know Artifact internals. They intentionally return prototype
     * defaults for now because this port only targets item networking.
     */
    bool blocksSpellcasting() const;
    float getDamageMultiplierModifier() const;
    bool hasProtectionTrigger() const;

    cugl::Color4 getDebugColor() const override;

    /** Returns a debug label for logs or future UI surfaces. */
    static const char* effectToString(Effect effect);
};

#endif // __HEXCAPADE_ARTIFACT_H__

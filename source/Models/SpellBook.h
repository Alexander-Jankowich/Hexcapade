//
//  SpellBook.h
//  Hexcapade
//
//  Concrete collectible item representing a carried spell source. The current
//  networking port keeps this intentionally lightweight so inventory syncing
//  works before the full spellbook rules are migrated.
//
#ifndef __HEXCAPADE_SPELLBOOK_H__
#define __HEXCAPADE_SPELLBOOK_H__
#include "Item.h"

class SpellBook : public Item {
public:
    /**
     * Placeholder spell identifiers carried by this book.
     *
     * // GA: itemnet
     * Future work can map these to the real spell system once the team chooses
     * how spell unlocks should interact with the networking branch.
     */
    enum class Spell {
        SPELL_EFFECT
    };

private:
    /** Which spell this book currently represents. */
    Spell _spell;

public:
    /**
     * Creates one spellbook instance.
     *
     * @param id      Stable item identifier.
     * @param name    Display name for UI/debug rendering.
     * @param value   Shared quota contribution on deposit.
     * @param spell   Placeholder spell metadata for inventory queries.
     */
    SpellBook(std::string id, std::string name, int value, Spell spell);

    Spell getSpell() const { return _spell; }
    cugl::Color4 getDebugColor() const override;

    /** Returns a debug label for logs or future UI surfaces. */
    static const char* spellToString(Spell spell);
};

#endif // __HEXCAPADE_SPELLBOOK_H__

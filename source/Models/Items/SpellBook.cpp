//
//  SpellBook.cpp
//  Hexcapade
//
//  Prototype spellbook implementation.
//
#include "SpellBook.h"

using namespace cugl;

SpellBook::SpellBook(std::string id, std::string name, int value, Spell spell) :
Item(std::move(id), std::move(name), value, Item::Kind::SPELLBOOK),
_spell(spell) {
}

Color4 SpellBook::getDebugColor() const {
    (void)_spell;
    return Color4(120, 180, 255, 255);
}

const char* SpellBook::spellToString(Spell spell) {
    (void)spell;
    return "spell_effect";
}

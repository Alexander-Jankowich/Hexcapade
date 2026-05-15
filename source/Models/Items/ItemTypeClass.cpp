//
//  ItemTypeClass.cpp
//  hexcapade
//
//  Created by Serena Duncan on 5/11/26.
//

#include "ItemTypeClass.h"

std::shared_ptr<cugl::JsonValue> ItemTypeClass::getConsts(ItemType t) {
    switch (t) {
        case ItemType::BOOK_FIRE:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-fire");
        case ItemType::BOOK_FREEZE:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-freeze");
        case ItemType::BOOK_EARTH:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-earth");
        case ItemType::BOOK_WATER:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-water");
        case ItemType::BOOK_DEATH:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-death");
        case ItemType::BOOK_HEAL:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spellbook-heal");
        case ItemType::SCROLL:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("scroll");
        case ItemType::RIBBON:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("ribbon");
        case ItemType::GEM:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("gem");
        case ItemType::OCTOLING:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("octoling");
        case ItemType::SPELL_RESTRICTION:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell-restriction");
        case ItemType::SPELL_BOOST:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell-boost");

        case ItemType::SPELL_REDUCE:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell-reduce");

        case ItemType::PROTECTION:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("protection");

        case ItemType::HEAVY:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("heavy");

        case ItemType::GOLD:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("gold-real");
        case ItemType::GOLD_FAKE:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("gold-fake");

        case ItemType::BOMB:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("bomb");

        default:
            return nullptr;
    }
}

std::shared_ptr<cugl::graphics::Texture> ItemTypeClass::getTexture(ItemType t) {
    switch (t) {
    case ItemType::BOOK_FIRE:
        return _assets->get<cugl::graphics::Texture>("spellbook_fire");
    case ItemType::BOOK_FREEZE:
        return _assets->get<cugl::graphics::Texture>("spellbook_freeze");
    case ItemType::BOOK_EARTH:
        return _assets->get<cugl::graphics::Texture>("spellbook_earth");
    case ItemType::BOOK_WATER:
        return _assets->get<cugl::graphics::Texture>("spellbook_water");
    case ItemType::BOOK_DEATH:
        return _assets->get<cugl::graphics::Texture>("spellbook_death");
    case ItemType::BOOK_HEAL:
        return _assets->get<cugl::graphics::Texture>("spellbook_heal");
    case ItemType::SCROLL:
        return _assets->get<cugl::graphics::Texture>("artifact_scroll");
    case ItemType::RIBBON:
        return _assets->get<cugl::graphics::Texture>("artifact_ribbon");
    case ItemType::GEM:
        return _assets->get<cugl::graphics::Texture>("artifact_gem");
    case ItemType::OCTOLING:
        return _assets->get<cugl::graphics::Texture>("artifact_octoling");
    case ItemType::SPELL_RESTRICTION:
        return _assets->get<cugl::graphics::Texture>("artifact_restrict");
    case ItemType::SPELL_BOOST:
        return _assets->get<cugl::graphics::Texture>("artifact_boost");
    case ItemType::SPELL_REDUCE:
        return _assets->get<cugl::graphics::Texture>("artifact_reduce");
    case ItemType::PROTECTION:
        return _assets->get<cugl::graphics::Texture>("artifact_protection");
    case ItemType::HEAVY:
        return _assets->get<cugl::graphics::Texture>("artifact_heavy");
    case ItemType::GOLD:
    case ItemType::GOLD_FAKE:
        return _assets->get<cugl::graphics::Texture>("artifact_gold");
    case ItemType::BOMB:
        return _assets->get<cugl::graphics::Texture>("artifact_bomb");
    default:
        return nullptr;
    }
}

std::shared_ptr<cugl::graphics::Texture> ItemTypeClass::getGlowTexture(ItemType t) {
    switch (t) {
    case ItemType::BOOK_FIRE:
        return _assets->get<cugl::graphics::Texture>("spellbook_fire_glow");
    case ItemType::BOOK_FREEZE:
        return _assets->get<cugl::graphics::Texture>("spellbook_freeze_glow");
    case ItemType::BOOK_EARTH:
        return _assets->get<cugl::graphics::Texture>("spellbook_earth_glow");
    case ItemType::BOOK_WATER:
        return _assets->get<cugl::graphics::Texture>("spellbook_water_glow");
    case ItemType::BOOK_DEATH:
        return _assets->get<cugl::graphics::Texture>("spellbook_death_glow");
    case ItemType::BOOK_HEAL:
        return _assets->get<cugl::graphics::Texture>("spellbook_heal_glow");
    case ItemType::SCROLL:
        return _assets->get<cugl::graphics::Texture>("artifact_scroll_glow");
    case ItemType::RIBBON:
        return _assets->get<cugl::graphics::Texture>("artifact_ribbon_glow");
    case ItemType::GEM:
        return _assets->get<cugl::graphics::Texture>("artifact_gem_glow");
    case ItemType::OCTOLING:
        return _assets->get<cugl::graphics::Texture>("artifact_octoling_glow");
    case ItemType::SPELL_RESTRICTION:
        return _assets->get<cugl::graphics::Texture>("artifact_restrict_glow");
    case ItemType::SPELL_BOOST:
        return _assets->get<cugl::graphics::Texture>("artifact_boost_glow");
    case ItemType::SPELL_REDUCE:
        return _assets->get<cugl::graphics::Texture>("artifact_reduce_glow");
    case ItemType::PROTECTION:
        return _assets->get<cugl::graphics::Texture>("artifact_protection_glow");
    case ItemType::HEAVY:
        return _assets->get<cugl::graphics::Texture>("artifact_heavy_glow");
    case ItemType::GOLD:
    case ItemType::GOLD_FAKE:
        return _assets->get<cugl::graphics::Texture>("artifact_gold_glow");
    case ItemType::BOMB:
        return _assets->get<cugl::graphics::Texture>("artifact_bomb_glow");
    default:
        return nullptr;
    }
}

std::shared_ptr<cugl::graphics::Texture> ItemTypeClass::getCard(ItemType t) {
    switch (t) {
    case ItemType::BOOK_FIRE:
        return _assets->get<cugl::graphics::Texture>("fire_card");
    case ItemType::BOOK_FREEZE:
        return _assets->get<cugl::graphics::Texture>("freeze_card");
    case ItemType::BOOK_EARTH:
        return _assets->get<cugl::graphics::Texture>("earth_card");
    case ItemType::BOOK_WATER:
        return _assets->get<cugl::graphics::Texture>("water_card");
    case ItemType::BOOK_DEATH:
        return _assets->get<cugl::graphics::Texture>("disintegrate_card");
    case ItemType::BOOK_HEAL:
        return _assets->get<cugl::graphics::Texture>("heal_card");
    case ItemType::SPELL_RESTRICTION:
        return _assets->get<cugl::graphics::Texture>("chains_info");
    case ItemType::SPELL_BOOST:
        return _assets->get<cugl::graphics::Texture>("brooch_info");
    case ItemType::SPELL_REDUCE:
        return _assets->get<cugl::graphics::Texture>("skull_info");
    case ItemType::PROTECTION:
        return _assets->get<cugl::graphics::Texture>("zombieTotem_info");
    case ItemType::HEAVY:
        return _assets->get<cugl::graphics::Texture>("heavyTreasure_info");
    case ItemType::GOLD:
    case ItemType::GOLD_FAKE:
        return _assets->get<cugl::graphics::Texture>("foolsGold_info");
    case ItemType::BOMB:
        return _assets->get<cugl::graphics::Texture>("bomb_info");
    case ItemType::SCROLL:
        return _assets->get<cugl::graphics::Texture>("scroll_info");
    case ItemType::RIBBON:
        return _assets->get<cugl::graphics::Texture>("ribbon_info");
    case ItemType::GEM:
        return _assets->get<cugl::graphics::Texture>("gem_info");
    case ItemType::OCTOLING:
        return _assets->get<cugl::graphics::Texture>("octoling_info");
    default:
        return nullptr;
    }
}

SpellType ItemTypeClass::getSpell(ItemType t) {
    switch (t) {
    case ItemType::BOOK_FIRE:
        return SpellType::FIREBALL;
    case ItemType::BOOK_FREEZE:
        return SpellType::FREEZE;
    case ItemType::BOOK_EARTH:
        return SpellType::EARTH;
    case ItemType::BOOK_WATER:
        return SpellType::WATER;
    case ItemType::BOOK_DEATH:
        return SpellType::DEATH;
    case ItemType::BOOK_HEAL:
        return SpellType::HEAL;
    default:
        return SpellType::NONE;
    }
}
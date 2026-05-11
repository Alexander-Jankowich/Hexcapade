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
        case ItemType::SKULL:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("skull");
        case ItemType::GEM:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("gem");
        case ItemType::SPELL_RESTRICTION:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell_restriction");
        case ItemType::SPELL_BOOST:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell_boost");

        case ItemType::SPELL_REDUCE:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("spell_reduce");

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
        return nullptr; // no asset yet
    case ItemType::SKULL:
        return nullptr; // no asset yet
    case ItemType::GEM:
        return nullptr; // no asset yet
    case ItemType::SPELL_RESTRICTION:
        return _assets->get<cugl::graphics::Texture>("artifact_scroll");
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
        return _assets->get<cugl::graphics::Texture>("antiMagic_info");
    case ItemType::SPELL_BOOST:
        return _assets->get<cugl::graphics::Texture>("mysticalBrooch_info");
    case ItemType::SPELL_REDUCE:
        return _assets->get<cugl::graphics::Texture>("cursedScroll_info");
    case ItemType::PROTECTION:
        return _assets->get<cugl::graphics::Texture>("zombieTotem_info");
    case ItemType::HEAVY:
        return _assets->get<cugl::graphics::Texture>("heavyTreasure_info");
    case ItemType::GOLD:
    case ItemType::GOLD_FAKE:
        return _assets->get<cugl::graphics::Texture>("foolsGold_info");
    case ItemType::BOMB:
        return _assets->get<cugl::graphics::Texture>("explodingPotion_info");
    case ItemType::SCROLL:
    case ItemType::SKULL:
    case ItemType::GEM:
    default:
        return nullptr; // no card assets yet
    }
}
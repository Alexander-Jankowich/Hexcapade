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
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("fools-gold");

        case ItemType::BOMB:
            return _assets->get<cugl::JsonValue>("constants")->get("artifacts")->get("bomb");

        default:
            return nullptr;
    }
}

std::shared_ptr<cugl::graphics::Texture> ItemTypeClass::getTexture(ItemType t){
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
            return _assets->get<cugl::graphics::Texture>("spellbook_disintegrate");

        case ItemType::BOOK_HEAL:
            return _assets->get<cugl::graphics::Texture>("spellbook_heal");

        case ItemType::SCROLL:
            return _assets->get<cugl::graphics::Texture>("artifact_scroll");

        case ItemType::SKULL:
            return _assets->get<cugl::graphics::Texture>("skull_incense");

        case ItemType::GEM:
            return _assets->get<cugl::graphics::Texture>("artifact_gem");

        case ItemType::SPELL_RESTRICTION:
            return _assets->get<cugl::graphics::Texture>("forbidden_chains");

        case ItemType::SPELL_BOOST:
            return _assets->get<cugl::graphics::Texture>("ystical_brooch");

        case ItemType::SPELL_REDUCE:
            //TODO: frank which one is it lol
            return _assets->get<cugl::graphics::Texture>("gem_brown");

        case ItemType::PROTECTION:
            return _assets->get<cugl::graphics::Texture>("zombie_totem");

        case ItemType::HEAVY:
            return _assets->get<cugl::graphics::Texture>("heavy_treasure");

        case ItemType::GOLD:
            return _assets->get<cugl::graphics::Texture>("fools_gold");

        case ItemType::BOMB:
            return _assets->get<cugl::graphics::Texture>("exploding_potion");

        default:
            return nullptr;
    }
}

std::shared_ptr<cugl::graphics::Texture> ItemTypeClass::getCard(ItemType t){
    switch (t) {
        case ItemType::BOOK_FIRE:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/fire_card");
        case ItemType::BOOK_FREEZE:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/freeze_card");

        case ItemType::BOOK_EARTH:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/earth_card");

        case ItemType::BOOK_WATER:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/water_card");

        case ItemType::BOOK_DEATH:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/disintegrate_card");

        case ItemType::BOOK_HEAL:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/heal_card");

        case ItemType::SPELL_RESTRICTION:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/spellRestrict_info");

        case ItemType::SPELL_BOOST:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/spellBoost_info");

        case ItemType::SPELL_REDUCE:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/spellReduce_info");

        case ItemType::PROTECTION:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/protection_info");

        case ItemType::HEAVY:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/heavy_info");

        case ItemType::GOLD:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/foolsGold_info");

        case ItemType::BOMB:
            return _assets->get<cugl::graphics::Texture>("inventory_cards/bomb_info");
        default:
            return nullptr;
    }
}

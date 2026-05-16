//
//  SpellType.h
//  alpha_final
//
//  Created by Kaz Tam on 5/8/26.
//

#ifndef __SPELL_TYPE_H__
#define __SPELL_TYPE_H__

enum class SpellType {
    NONE = 0,
    FREEZE = 1,
    FIREBALL = 2,
    HEAL = 3,
    COMBO_FIREBALL = 4,
    COMBO_FREEZE = 5,
    DEATH = 6,
    EARTH = 7,
    COMBO_EFREEZE = 8,
    WATER = 9,
    BUFF = 10,
    SHIELD = 11,
    MUD_WATER = 12,
    STEAM = 13,
    OCTO_ATTACK = 14
};

inline SpellType strToSpell(std::string s) {
    static std::map<std::string, SpellType> gesturemap = {
        {"fire", SpellType::FIREBALL},
        {"freeze", SpellType::FREEZE},
        {"heal", SpellType::HEAL },
        {"death", SpellType::DEATH},
        {"earth", SpellType::EARTH},
        {"water", SpellType::WATER}
    };

    try {
        return gesturemap[s];
    }
    catch (...) {
        return SpellType::NONE;
    }
}

inline std::string spellToStr(SpellType t) {
    static std::map<SpellType, std::string> gesturemap = {
        {SpellType::FIREBALL, "fire"},
        {SpellType::FREEZE, "freeze"},
        {SpellType::HEAL, "heal"},
        {SpellType::DEATH, "death"},
        {SpellType::EARTH, "earth"},
        {SpellType::WATER, "water"},
        {SpellType::COMBO_FIREBALL, "combo-fire"},
        {SpellType::COMBO_EFREEZE, "combo-efreeze"},
        {SpellType::COMBO_FREEZE, "combo-freeze"},
        {SpellType::MUD_WATER, "mud-water"},
        {SpellType::SHIELD, "shield"},
        {SpellType::BUFF, "buff"},
        {SpellType::STEAM, "steam"}
    };

    try {
        return gesturemap[t];
    }
    catch (...) {
        CULog("missing string for spell %d", t);
        return "none";
    }
}

#endif
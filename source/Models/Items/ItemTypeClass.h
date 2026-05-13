//
//  ItemType.h
//  hexcapade
//
//  Created by Serena Duncan on 5/10/26.
//
#ifndef __ITEM_TYPE_CLASS_H__
#define __ITEM_TYPE_CLASS_H__

#include <cugl/cugl.h>
#include "../Spells/SpellType.h"

using namespace cugl;
using namespace cugl::physics2::distrib;
using namespace std;


class ItemTypeClass {
private:
    std::shared_ptr<cugl::AssetManager> _assets;
    
public:
    // one entry per texture, populate as more item textures come in
    // add a corresponding entry in constants.json under artifacts w the value
    enum class ItemType {
        BOOK_FIRE, // 0
        BOOK_FREEZE,
        BOOK_EARTH,
        BOOK_WATER,
        BOOK_DEATH,
        BOOK_HEAL, // 5
        /** No effect - low value */
        SCROLL,
        /** No effect - medium value */
        RIBBON,
        /** No effect - high value */
        GEM,
        /** Player cannot cast spells. */
        SPELL_RESTRICTION,
        /** Attack spells deal +2 damage. */
        SPELL_BOOST,
        /** Attack spells deal -2 damage. */
        SPELL_REDUCE,
        /** Tanks one lethal hit, then consumed. */
        PROTECTION,
        /** Player moves at half speed. */
        HEAVY,
        /** High-value item (real). Non-droppable. */
        GOLD,
        /** fake gold */
        GOLD_FAKE,
        /** Explodes after a timer, dealing damage. Non-droppable. */
        BOMB
    };
    ItemTypeClass() {};
    ItemTypeClass(std::shared_ptr<cugl::AssetManager>& a) {
        _assets = a;
    }
    std::shared_ptr<cugl::JsonValue> getConsts(ItemType t);
    
    std::shared_ptr<cugl::graphics::Texture> getTexture(ItemType t);
    
    std::shared_ptr<cugl::graphics::Texture> getCard(ItemType t);

    std::shared_ptr<cugl::graphics::Texture> getGlowTexture(ItemType t);

    static SpellType getSpell(ItemType t);

};

#endif // !__ITEM_TYPE_CLASS_H__

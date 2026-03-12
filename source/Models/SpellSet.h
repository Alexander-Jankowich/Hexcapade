//
//  SpellSet.h
//  gp
//
//  Created by Alex Jankowich on 2/22/26.
//
#ifndef __SPELL_SET_H__
#define __SPELL_SET_H__
#include <cugl/cugl.h>

#pragma mark Spell

/** Add more spells to this list later */
enum class SpellType {
    NONE,
    FREEZE,
    FIREBALL
};

class SpellSet {
#pragma mark Spell
public:
    class Spell{
    public:
        cugl::Vec2 position;
        cugl::Vec2 velocity;
        float damage;
        bool froze;
        
        Spell(const cugl::Vec2 pos,cugl::Vec2 vel, int dmg, bool freeze);
        
        void update();
    };
    
#pragma mark SpellSet
    std::unordered_map<std::string, std::shared_ptr<Spell>> current;
    std::unordered_map<std::string, std::shared_ptr<Spell>> pending;
    
    SpellSet(std::shared_ptr<cugl::JsonValue>);
    
#pragma mark Graphics

    void setTexture(const std::shared_ptr<cugl::graphics::Texture>& value);

    void spawnSpell(std::string sid, SpellType type, cugl::Vec2 position, float angle);
    void spawnSpell(std::string sid, SpellType type, cugl::Vec2 position, cugl::Vec2 angle);

    void update();

    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

    void dispose();

    bool contains(std::string sid);

private:
    float _mass;
    float _radius;

    std::shared_ptr<cugl::graphics::Texture> _texture;
    std::shared_ptr<cugl::JsonValue> _spellConsts;
};
#endif // !__SPELL_SET_H__

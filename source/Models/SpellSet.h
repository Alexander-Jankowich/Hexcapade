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
        
        Spell(const cugl::Vec2 pos,cugl::Vec2 vel, bool freeze);
        
        void update();
    };
    
#pragma mark SpellSet
    std::unordered_map<std::string, std::shared_ptr<Spell>> current;
    std::unordered_map<std::string, std::shared_ptr<Spell>> pending;
    
    SpellSet();
    
    float mass;
    float radius;
    
    std::shared_ptr<cugl::graphics::Texture> texture;
#pragma mark Graphics
    

    void setTexture(const std::shared_ptr<cugl::graphics::Texture>& value);

    void spawnSpell(std::string sid, cugl::Vec2 position, float angle, bool freeze);
    void spawnSpell(std::string sid, cugl::Vec2 position, cugl::Vec2 angle, bool freeze);

    void update();

    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

    void dispose();

    bool contains(std::string sid);
};
#endif // !__SPELL_SET_H__

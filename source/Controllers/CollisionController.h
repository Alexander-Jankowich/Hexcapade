//
//  CollisionController.h
//
//  Handles all collision detection and resolution between
//  the player, enemies, and items.
//
#ifndef __COLLISION_CONTROLLER_H__
#define __COLLISION_CONTROLLER_H__

#include <cugl/cugl.h>
#include "../Models/EnemySet.h"
#include "../Models/SpellSet.h"
#include "../Models/Player.h"

class CollisionController {
private:
    cugl::Size _size;

public:
    CollisionController() {}
    ~CollisionController() {}

    /**
     * Initializes the collision controller with the screen size.
     *
     * @param size  The size of the game window
     */
    void init(cugl::Size size) { _size = size; }


    /**
     * Checks collision between the player and an item at a given position.
    */
    bool resolvePlayerItem(std::shared_ptr<Player>& player, const cugl::Vec2& itemPos,
        float itemRadius);

    void resolvePlayerEnemies(std::shared_ptr<Player>& player, const cugl::Vec2& playerPos,
        float playerRadius, const std::shared_ptr<EnemySet>& enemy);

    void resolveEnemiesSpell(SpellSet& spells, std::shared_ptr<EnemySet>& enemy,
        cugl::Vec2 knockback);

private:
    /**
     * Checks and resolves collision between the player and an enemy.
     */
    void resolvePlayerEnemy(std::shared_ptr<Player>& player, const cugl::Vec2& playerPos,
        float playerRadius, std::shared_ptr<Enemy>& enemy, float enemyRadius);

    void resolveEnemySpell(SpellSet& spells, std::shared_ptr<Enemy>& enemy,
        cugl::Vec2 knockback, float enemyRadius);
    
};

#endif /* __COLLISION_CONTROLLER_H__ */

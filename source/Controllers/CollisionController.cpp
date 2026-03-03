//
//  CollisionController.cpp
//
#include "CollisionController.h"

using namespace cugl;

void CollisionController::resolvePlayerEnemy(std::shared_ptr<Player>& player,
    const Vec2& playerPos, float playerRadius, std::shared_ptr<EnemySet::Enemy>& enemy,
    float enemyRadius) {
    Vec2 diff = playerPos - enemy->getPosition();
    float dist = diff.length();
    float impactDist = playerRadius + enemyRadius;

    if (dist < impactDist) {
        player->setHealth(player->getHealth() - 3); // Change health depletion to depend on enemy damage
        enemy->setKnockback(-1 * playerPos);
    }
}


void CollisionController::resolvePlayerEnemies(std::shared_ptr<Player>& player,
    const cugl::Vec2& playerPos, float playerRadius, const std::shared_ptr<EnemySet>& enemy) {
    for (auto it = enemy->current.begin(); it != enemy->current.end(); ++it) {
        std::shared_ptr<EnemySet::Enemy> ptr = *it;
        resolvePlayerEnemy(player, playerPos, playerRadius, ptr, enemy->getRadius());
    }
}

bool CollisionController::resolvePlayerItem(const Vec2& playerPos, float playerRadius,
    const Vec2& itemPos, float itemRadius) {
    Vec2 diff = playerPos - itemPos;
    float dist = diff.length();

    return dist < (playerRadius + itemRadius);
}

void CollisionController::resolveEnemySpell(SpellSet& spells, std::shared_ptr<EnemySet::Enemy>& enemy,
    Vec2 knockback, float enemyRadius) {
    auto sp_iter = spells.current.begin();
    while(sp_iter != spells.current.end()) {
        float dist = (sp_iter->get()->position - enemy->getPosition()).length();
        if (dist <= enemyRadius + 10) { // hard coding a spell radius bc WHATEVER
            // handle collision
            if(sp_iter->get()->froze){
                enemy->setFrozen();
            }
            else {
                enemy->setKnockback(knockback);
            }
            enemy->setHealth(enemy->getHealth() - sp_iter->get()->damage);
            
            sp_iter = spells.current.erase(sp_iter);
        }
        
        else ++sp_iter;
    }
}

void CollisionController::resolveEnemiesSpell(SpellSet& spells, std::shared_ptr<EnemySet>& enemy,
    cugl::Vec2 knockback) {
    for (auto it = enemy->current.begin(); it != enemy->current.end(); ++it) {
        std::shared_ptr<EnemySet::Enemy> ptr = *it;
        resolveEnemySpell(spells, ptr, knockback, enemy->getRadius());
    }
}
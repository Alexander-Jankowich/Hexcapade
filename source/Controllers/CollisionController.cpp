//
//  CollisionController.cpp
//
#include "CollisionController.h"

using namespace cugl;
using namespace std;

//void CollisionController::resolvePlayerEnemy(std::shared_ptr<Player>& player,
//    const Vec2& playerPos, float playerRadius, std::shared_ptr<Enemy>& enemy,
//    float enemyRadius) {
//    Vec2 diff = playerPos - enemy->getPosition();
//    float dist = diff.length();
//    float impactDist = playerRadius + enemyRadius;
//
//    if (dist < impactDist) {
//        player->setHealth(player->getHealth() - 3); // Change health depletion to depend on enemy damage
//        enemy->setKnockback(-1 * playerPos);
//    }
//}
//
//
//void CollisionController::resolvePlayerEnemies(std::shared_ptr<Player>& player,
//    const cugl::Vec2& playerPos, float playerRadius, const std::shared_ptr<EnemySet>& enemy) {
//    for (auto it = enemy->current.begin(); it != enemy->current.end(); ++it) {
//        std::shared_ptr<Enemy> ptr = std::get<1>(*it);
//        resolvePlayerEnemy(player, playerPos, playerRadius, ptr, enemy->getRadius());
//    }
//}
//
//void CollisionController::resolvePlayersEnemies( std::unordered_set<std::shared_ptr<Player>> players,
//                           float playerRadius, const std::shared_ptr<EnemySet>& enemies) {
//    for (auto it = players.begin(); it != players.end(); ++it) {
//        std::shared_ptr<Player> ptr = (*it);
//        resolvePlayerEnemies(ptr, ptr->getPosition(), playerRadius, enemies);
//    }
//    
//}
//
//
//bool CollisionController::resolvePlayerItem(std::shared_ptr<Player>& player,
//    const Vec2& itemPos, float itemRadius) {
//    Vec2 p1 = player->getPosition() - player->getSize() / 2;
//    Vec2 p2 = player->getPosition() + player->getSize() / 2;
//    Vec2 closest = Vec2(max(p1.x, min(itemPos.x, p2.x)),
//        max(p1.y, min(itemPos.y, p2.y)));
//
//    Vec2 dist = closest - itemPos;
//
//    return dist.length() < itemRadius;
//}
//
//void CollisionController::resolveEnemySpell(SpellSet& spells, std::shared_ptr<Enemy>& enemy,
//    Vec2 knockback, float enemyRadius) {
//    auto sp_iter = spells.current.begin();
//    while(sp_iter != spells.current.end()) {
//        float dist = (sp_iter->second->_position - enemy->getPosition()).length();
//        if (sp_iter->second->healed) {
//            ++sp_iter;
//            continue;
//        }
//        if (dist <= enemyRadius + 10) { // hard coding a spell radius bc WHATEVER
//            // handle collision
//            if(sp_iter->second->froze){
//                enemy->setFrozen();
//            }
//            else {
//                enemy->setKnockback(knockback);
//            }
//            enemy->setHealth(enemy->getHealth() - sp_iter->second->damage);
//            
//            sp_iter = spells.current.erase(sp_iter);
//        }
//        
//        else ++sp_iter;
//    }
//}
//
//void CollisionController::resolvePlayersSpell(SpellSet& spells,
//    std::unordered_set<std::shared_ptr<Player>>& players,
//    int healAmount,
//    const std::string& casterPID) {
//    auto sp_iter = spells.current.begin();
//    while (sp_iter != spells.current.end()) {
//        if (!sp_iter->second->healed) { 
//            ++sp_iter; continue; 
//        }
//        bool consumed = false;
//        for (auto& p : players) {
//            if (p->getPID() == casterPID) continue;
//            Vec2 diff = sp_iter->second->_position - p->getPosition();
//            if (diff.length() < 20.0f) {
//                int newHealth = std::min(p->getHealth() + healAmount, 100);
//                p->setHealth(newHealth);
//                consumed = true;
//                break;
//            }
//        }
//        if (consumed) sp_iter = spells.current.erase(sp_iter);
//        else ++sp_iter;
//    }
//}
//
//void CollisionController::resolveEnemiesSpell(SpellSet& spells, std::shared_ptr<EnemySet>& enemy,
//    cugl::Vec2 knockback) {
//    for (auto it = enemy->current.begin(); it != enemy->current.end(); ++it) {
//        std::shared_ptr<Enemy> ptr = it->second;
//        resolveEnemySpell(spells, ptr, knockback, enemy->getRadius());
//    }
//}

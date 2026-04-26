//
//  Pathfinder.h
//  Hexcapade
//
//  This class updates enemy target positions based on player positions and data.
//
//  Author: Kaz Tam
//  Version: 3/31/26
//

#ifndef __PATHFINDER_H__
#define __PATHFINDER_H__

#include <cugl/cugl.h>

#include "../Models/Items/ItemFactory.h"
#include "../Models/Players/PlayerFactory.h"
#include "../Controllers/NetworkController.h"
#include "../Models/Enemies/EnemyFactory.h"
#include "../Models/Map/Map.h"

using namespace std;
using namespace cugl;

class Pathfinder {
private:
    // All helper functions will be private

    Vec2 getClosestPlayer(Vec2 enemPos, unordered_set<shared_ptr<PlayerObstacle>> targs);
    Vec2 getClosestPlayer(Vec2 enemPos, unordered_set<PlayerObstacle*> targs);

    /**
    * Returns a set of all players who are not dead and are within range of the enemy
    */
    unordered_set<shared_ptr<PlayerObstacle>> getValidTargets(const Vec2 pos, float range, shared_ptr<PlayerFactory> playerFact);

    unordered_set<PlayerObstacle*> visiblePos; // Only used for getClosestPlayerRaycast!!
    unordered_set<PlayerObstacle*> getVisiblePlayers(shared_ptr<physics2::distrib::NetWorld> world, Vec2 enemPos,
        float range, unordered_set<shared_ptr<PlayerObstacle>> targs);

    Vec2 finalTarg;
    physics2::PolygonObstacle* wall;
    Vec2 findVelocity(shared_ptr<physics2::distrib::NetWorld> world, const Vec2 targ, const Vec2 pos, float radius);

    /** Returns false if no walls were detected (path is clear), true otherwise */
    bool performRaycastCheck(shared_ptr<physics2::distrib::NetWorld> world, const Vec2 targ, const Vec2 pos, float radius);

public:
    Pathfinder() {}

    /**
    * Updates enemy targets, movement, and state based on the players
    */
    void updateEnemies(shared_ptr<EnemyFactory> enemyFact, shared_ptr<PlayerFactory> playerFact, shared_ptr<SpellFactory> spellFact, std::shared_ptr<NetworkController> networkCtrl,Uint32 spellFactId, shared_ptr<physics2::distrib::NetWorld> world);
};

#endif // __PATHFINDER_H__

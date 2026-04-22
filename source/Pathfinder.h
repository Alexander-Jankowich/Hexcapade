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

#include "Models/Enemies.h"
#include "Models/Players.h"

using namespace std;
using namespace cugl;

class Pathfinder {
private:
    // All helper functions will be private

    Vec2 getClosestPlayer(Vec2 enemPos, unordered_set<shared_ptr<PlayerObstacle>> targs);

    /**
    * Returns a set of all players who are not dead and are within range of the enemy
    */
    unordered_set<shared_ptr<PlayerObstacle>> getValidTargets(Vec2 pos, float range, shared_ptr<PlayerFactory> playerFact);

public:
    Pathfinder() {}

    /**
    * Updates enemy targets, movement, and state based on the players
    */
    void updateEnemies(shared_ptr<EnemyFactory> enemyFact, shared_ptr<PlayerFactory> playerFact);
};

#endif // __PATHFINDER_H__

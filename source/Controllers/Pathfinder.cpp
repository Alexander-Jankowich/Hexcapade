#include "Pathfinder.h"

#define FROZEN_FRAMES 60

unordered_set<shared_ptr<PlayerObstacle>> Pathfinder::getValidTargets(const Vec2 pos, float range,
        shared_ptr<PlayerFactory> playerFact) {
    unordered_set <shared_ptr<PlayerObstacle>> out;

    for (auto p : playerFact->current) {
        if (p->getHealth() > 0 && pos.distance(p->getPosition()) < range) {
            out.emplace(p);
        }
    }
    return out;
}


Vec2 Pathfinder::getClosestPlayer(Vec2 enemPos, unordered_set<shared_ptr<PlayerObstacle>> targs) {
    float minDist = FLT_MAX;
    Vec2 minPos = Vec2(0, 0); // default
    for (auto t : targs) {
        float d = enemPos.distance(t->getPosition());
        if (d < minDist) {
            minDist = d;
            minPos = t->getPosition();
        }
    }
    return minPos;
}

Vec2 Pathfinder::getClosestPlayer(Vec2 enemPos, unordered_set<PlayerObstacle*> targs) {
    float minDist = FLT_MAX;
    Vec2 minPos = Vec2(0, 0); // default
    for (auto t : targs) {
        float d = enemPos.distance(t->getPosition());
        if (d < minDist) {
            minDist = d;
            minPos = t->getPosition();
        }
    }
    return minPos;
}

unordered_set<PlayerObstacle*> Pathfinder::getVisiblePlayers(shared_ptr<physics2::distrib::NetWorld> world,
        Vec2 enemPos, float range, unordered_set<shared_ptr<PlayerObstacle>> targs) {
    visiblePos.clear();
    function<float(b2Fixture*, const Vec2&, const Vec2&, float)> callback =
        [this](b2Fixture* fixture, const Vec2& point, const Vec2& normal, float fraction) {
            physics2::Obstacle* obs = reinterpret_cast<physics2::Obstacle*>(fixture->GetBody()->GetUserData().pointer);
            if (auto p = dynamic_cast<PlayerObstacle*>(obs)) {
                visiblePos.emplace(p);
                //CULog("see player");
                return fraction;
            }
            if (auto p = dynamic_cast<physics2::PolygonObstacle*>(obs)) {
                //CULog("hit wall");
                return 0.0f;
            }
            return 1.0f;
        };

    for (auto t : targs) {
        if (t->getPosition().distance(enemPos) < range) {
            world->rayCast(callback, enemPos, t->getPosition());
        }
    }

    return visiblePos;
}

Vec2 Pathfinder::findVelocity(shared_ptr<physics2::distrib::NetWorld> world, const Vec2 targ, const Vec2 pos, float radius) {
    finalTarg = targ;

    // Loop until finding a solution
    Vec2 direction = Vec2(targ.y - pos.y, pos.x - targ.x).getNormalization();
    while (performRaycastCheck(world, pos + direction * radius, finalTarg + direction * radius, radius) ||
        performRaycastCheck(world, pos - direction * radius, finalTarg - direction * radius, radius)) {
        direction = Vec2(targ.y - pos.y, pos.x - targ.x).getNormalization();
        //CULog("in loop! from %s to %s", pos.toString().c_str(), targ.toString().c_str());
    }

    return finalTarg - pos;
}

bool Pathfinder::performRaycastCheck(shared_ptr<physics2::distrib::NetWorld> world, const Vec2 targ, const Vec2 pos, float radius) {
    wall = nullptr;
    function<float(b2Fixture*, const Vec2&, const Vec2&, float)> callback =
        [this](b2Fixture* fixture, const Vec2& point, const Vec2& normal, float fraction) {
        physics2::Obstacle* obs = reinterpret_cast<physics2::Obstacle*>(fixture->GetBody()->GetUserData().pointer);
        if (auto p = dynamic_cast<physics2::PolygonObstacle*>(obs)) {
            wall = p;
            //CULog("see wall");
            return 0.0f;
        }
        //CULog("hit something");
        return 1.0f;
    };

    //CULog("perform raycast");
    world->rayCast(callback, pos, targ);

    if (wall) {
        float leftFar = 0, rightFar = 0;
        Vec2 leftPoint, rightPoint;

        auto exterior = wall->getPolygon().exterior();

        // Loop through all points
        for (const auto i : exterior) {
            const auto point = wall->getPolygon().at(i);

            // Find distance from line
            float len = fabs((targ.y - pos.y) * point.x - (targ.x - pos.x) * point.y + targ.y * pos.x - targ.x * pos.y) /
                    targ.distance(pos);

            // Decide which side the point is on and keep farthest point
            // Note: may want to ignore points on a concave side of the shape, add that if this code doesn't work...
            // I think it should work without ignoring concave points...
            if ((targ.x - pos.x) * (point.y - pos.y) - (targ.y - pos.y) * (point.x - pos.x) > 0) {
                if (len > rightFar) {
                    rightFar = len;
                    rightPoint = point;
                }
            }
            else {
                if (len > leftFar) {
                    leftFar = len;
                    leftPoint = point;
                }
            }
        }
        
        if (leftFar == 0 || rightFar == 0) {
            // Just skimmed wall, can act as if no touch
            return false;
        }

        // Go around side 
        if (leftFar < rightFar) {
            Vec2 norm = Vec2(targ.y - pos.y, pos.x - targ.x).getNormalization();
            finalTarg = leftPoint + norm * radius;
        }
        else {
            Vec2 norm = Vec2(pos.y - targ.y, targ.x - pos.x).getNormalization();
            finalTarg = rightPoint + norm * radius;
        }

        return true;
    }
    return false;
}


void Pathfinder::updateEnemies(shared_ptr<EnemyFactory> enemyFact, shared_ptr<PlayerFactory> playerFact,
        shared_ptr<physics2::distrib::NetWorld> world) {

    for (auto en : enemyFact->current) {
        if (en->isFroze() || en->getWaterPushTimer() > 0) {
            continue;
        }

        // Get and set new target depending on enemy type
        Vec2 targ;
        switch (en->TYPE) {
        case EnemyType::GHOST:
            targ = getClosestPlayer(en->getPosition(),
                getValidTargets(en->getPosition(), en->getRange(), playerFact));
            break;
        case EnemyType::BAT:
            targ = getClosestPlayer(en->getPosition(),
                getVisiblePlayers(world, en->getPosition(), en->getRange(), playerFact->current));
            break;
        case EnemyType::OCTO:
            targ = getClosestPlayer(en->getPosition(),
                getVisiblePlayers(world, en->getPosition(), en->getRange(), playerFact->current));
            break;
        default:
            CULog("Error! Enemy type not implemented!!!");
            break;
        }

        if (!targ.isZero()) { // Found a valid target
            en->setTarget(targ);
            //CULog("set target!");
        }

        // if chasing
        if (en->getState() == EnemyObstacle::EnemyState::CHASING) {
            // Check if enemy has reached its target
            if (en->getTarget().distance(en->getPosition()) < 0.01) {
                en->setState(EnemyObstacle::EnemyState::IDLE);
                en->setLinearVelocity(Vec2::ZERO);
                //CULog("reached target!");
                continue;
            }

            // Set velocity to go to target
            Vec2 currentVel = en->getLinearVelocity();

            //Vec2 desiredVel = en->getTarget() - en->getPosition();
            Vec2 desiredVel = findVelocity(world, en->getTarget(), en->getPosition(), en->getRadius());
            desiredVel.normalize();
            if (en -> isMud()){
                desiredVel *= en->getSpeed()/2;
                int t = en -> getMudTimer() - 1;
                en -> setMudTimer(t);
                if (en -> getMudTimer() == 0){
                    en -> setMud(false);
                }
            }
            else{
                desiredVel *= en->getSpeed();
            }

            float recovery = 0.2f;
            Vec2 newVel = currentVel * (1 - recovery) + desiredVel * recovery;
            en->setVelocity(newVel);
        }
    }
}

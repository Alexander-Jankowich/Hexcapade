#include "Pathfinder.h"

#define FROZEN_FRAMES 60

unordered_set<shared_ptr<PlayerObstacle>> Pathfinder::getValidTargets(Vec2 pos, float range,
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
            if (auto p = dynamic_cast<RectObstacle*>(obs)) {
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

void Pathfinder::updateEnemies(shared_ptr<EnemyFactory> enemyFact, shared_ptr<PlayerFactory> playerFact,
        shared_ptr<physics2::distrib::NetWorld> world) {

    for (auto en : enemyFact->current) {
        // Check if frozen
        if (en->getWaterPushTimer() > 0) {
            en->tickWaterPush();
            Vec2 pushVel = en->getWaterPushDir() * en->getSpeed() * 2.5f; 
            en->setVelocity(pushVel);
            continue;
        }
        if (en->isFroze()) {
            en->setLinearVelocity(Vec2::ZERO);
            int i = en->getFrozenTimer() - 1;
            en->setFrozenTimer(i);
            if (en->getFrozenTimer() == 0) {
                en->setFrozen(false);
            }
            continue;
        }

        // If attack is on cooldown, wait
        if (en->getState() == EnemyObstacle::EnemyState::ATTACK_RESET) {
            en->updateAttackCounter();
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

            Vec2 desiredVel = en->getTarget() - en->getPosition();
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

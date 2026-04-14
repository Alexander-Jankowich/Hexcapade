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

void Pathfinder::updateEnemies(shared_ptr<EnemyFactory> enemyFact, shared_ptr<PlayerFactory> playerFact) {
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

        // Update target
        auto targets = getValidTargets(en->getPosition(), en->getRange(), playerFact);

        // Set new target depending on enemy type
        Vec2 targ;
        switch (en->TYPE) {
        case EnemyType::GHOST:
            targ = getClosestPlayer(en->getPosition(), targets);
            break;
        default:
            CULog("Error! Enemy type not implemented!!!");
        }
        if (!targ.isZero()) { // Found a valid target
            en->setTarget(targ);
            //CULog("found target!!");
        }else{
            en -> setVelocity(Vec2::ZERO);
        }

        // if chasing
        if (en->getState() == EnemyObstacle::EnemyState::CHASING) {
            // Check if enemy has reached its target
            if (en->getTarget().distance(en->getPosition()) < 0.01) {
                en->setState(EnemyObstacle::EnemyState::IDLE);
                en->setVelocity(Vec2::ZERO);
                continue;
            }

            // Set velocity to go to target
            Vec2 currentVel = en->getLinearVelocity();

            Vec2 desiredVel = en->getTarget() - en->getPosition();
            desiredVel.normalize();
            if (en -> isMud() ){
                desiredVel *= en->getSpeed()/2;
                int t = en -> getMudTimer() - 1;
                en -> setMudTimer(t);
                if(en -> getMudTimer() == 0){
                    en -> setMud(false);
                }
                
            }else{
                desiredVel *= en->getSpeed();
            }
            

            float recovery = 0.2f;
            Vec2 newVel = currentVel * (1 - recovery) + desiredVel * recovery;
            en->setVelocity(newVel);
        }
    }
}

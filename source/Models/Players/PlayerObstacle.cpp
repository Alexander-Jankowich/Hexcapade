#include "PlayerObstacle.h"
#define PHYSICS_SCALE 50.0f

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, (s * 0.4) / PHYSICS_SCALE);
    setBodyType(b2_dynamicBody);
    setDensity(1.0f);
    //setLinearDamping(15.0f);

    createFixtures();
    setAwake(true);
    setEnabled(true);
    setFixedRotation(true);
    _pid = pid;
    _speed = 4;
    _health = 6;
    _inventoryLimit = 5;
}

void PlayerObstacle::setVelocity(Vec2 v) {
    _velocity = v;
    //CULog("setVelocity called, state is %d", (int)_state);
    if (isFrozen()) {
        if (getBody()) getBody()->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }

    if (!v.isZero()) {
        _angle = v.getNormalization();
    }

    if (!getBody()) return; // body not ready yet
    b2Vec2 currentVel = getBody()->GetLinearVelocity();
    b2Vec2 targetVel;

    if (_mud){
        targetVel = b2Vec2(v.x * 0.5f, v.y * 0.5f);
    }
    else{
        targetVel = b2Vec2(v.x, v.y);
    }

    b2Vec2 velDiff = targetVel - currentVel;
    float mass = getBody()->GetMass();
    float scale = mass * 60.0f;
    b2Vec2 force = b2Vec2(velDiff.x * scale, velDiff.y * scale);
    //CULog("mass: %f, force: %f %f", mass, force.x, force.y);
    getBody()->ApplyForceToCenter(force, true);
}

void PlayerObstacle::setHealth(int h) {
    if (h < _health) {
        if (_hurt) {
            // Cannot be hit if already hit very recently
            return;
        }
        _hurt = true;
    }

    _health = min(h, MAX_HEALTH);

    if (_health <= 0) {
        setVelocity(Vec2::ZERO);
        setLinearVelocity(Vec2::ZERO);

        _health = 0;
    }
}


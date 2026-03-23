

#include "PlayerObstacle.h"
#define PHYSICS_SCALE 100

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, (s * 0.08) / PHYSICS_SCALE);
    setBodyType(b2_dynamicBody);
    createFixtures();
    setAwake(true);
    setEnabled(true);
    _pid = pid;
    _speed = 6;
    _health = 10;
}

void PlayerObstacle::setVelocity(Vec2 v) {
    _velocity = v;
    setLinearVelocity(_velocity);
    if (!v.isZero()) {
        _angle = v.getNormalization();
    }
}

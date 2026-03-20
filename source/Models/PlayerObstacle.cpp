

#include "PlayerObstacle.h"
#define PHYSICS_SCALE 100

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, s / PHYSICS_SCALE);
    setBodyType(b2_dynamicBody);
    setAwake(true);
    setEnabled(true);
    _pid = pid;
    _speed = 8;
    _health = 10;
}

void PlayerObstacle::setVelocity(Vec2 v) {
    _velocity = v;
    setLinearVelocity(_velocity);
    if (!v.isZero()) {
        _angle = v.getNormalization();
    }
}



#include "PlayerObstacle.h"

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, s); // TODO WHAT THE FUCK SHOULD THIS BE
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
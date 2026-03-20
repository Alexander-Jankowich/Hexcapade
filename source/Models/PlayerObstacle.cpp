

#include "PlayerObstacle.h"

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, s); // TODO WHAT THE FUCK SHOULD THIS BE
    _pid = pid;
    _speed = 100;
    _health = 10;
}

#include "TutorialObstacle.h"


TutorialObstacle::TutorialObstacle(const cugl::Rect b, std::shared_ptr<graphics::Texture> tex) {
    setPosition(b.origin);
    setDimension(b.size);
    _png = tex;
    _seen = false;
}

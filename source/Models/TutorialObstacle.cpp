
#include "TutorialObstacle.h"


TutorialObstacle::TutorialObstacle(const cugl::Rect b, std::string tex) {
    setPosition(b.origin);
    setDimension(b.size);
    _png = tex;
    _seen = false;

    markRemoved(false);
}

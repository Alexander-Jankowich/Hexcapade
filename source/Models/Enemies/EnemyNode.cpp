#include "EnemyNode.h"

using namespace cugl;
using namespace cugl::graphics;
/**
 * Creates a ship wiht the given position and data.
 *
 * The JsonValue should be a reference of all of the constants
 * that necessary to set the "hidden physical properties".
 *
 * @param pos   The ship position
 * @param data  The data defining the physics constants
 */
EnemyNode::EnemyNode(std::shared_ptr<cugl::graphics::Texture>& tex) {
    initWithSheet(tex, 1, 1);
    frozen = false;
    statue = false;
}

void EnemyNode::updateState() {
    if(frozen){
        setColor(Color4::BLUE);
    }else if(statue){
        setColor(Color4::GRAY);
    }else{
        setColor(Color4::WHITE);
    }
}

//
//  EnemyNode.h
//  hexc
//
//  This class represents the enemy node for the scene graph and controls
//  all graphics for an enemy
//
//  Author: Kaz Tam
//  Version: 1/20/25
//

#ifndef __OCTOPLUSH_NODE_H__
#define __OCTOPLUSH_NODE_H__
#include <cugl/cugl.h>
#include "EnemyNode.h"

using namespace cugl;
using namespace std;

/**
 * Model class representing an enemy.
 */
class OctoplushNode : public EnemyNode {
private:
    shared_ptr<SpriteNode> _attack;

    Vec2 _offset;

public:
#pragma mark Constructors
    /**
     * Creates a enemy node the given position and data.
     *
     * @param pos   The ship position
     * @param data  The data defining the physics constants
     */
    OctoplushNode(shared_ptr<graphics::Texture>& tex, shared_ptr<JsonValue>& consts, const shared_ptr<graphics::Texture>& attack);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~OctoplushNode() {}
    
#pragma mark Properties

    /**
    * Returns the next animation function and duration
    */
    pair<ActionFunction, float> getNextAnimation() override;
    
#pragma mark Graphics
    void draw(const std::shared_ptr< graphics::SpriteBatch >& batch, const Affine2& transform, Color4 tint) override;
};

#endif // !__GP_ENEMY_H__


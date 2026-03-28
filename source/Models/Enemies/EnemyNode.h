//
//  SLShip.h
//  Ship Lab
//
//  This class tracks all of the state (position, velocity, rotation) of a
//  single ship. In order to obey the separation of the model-view-controller
//  pattern, controller specific code (such as reading the keyboard) is not
//  present in this class.
//
//  With that said, you will notice several elements that look like they are
//  part of the view, namely the texture and sound.  But this is the texture
//  and sound DATA.  The actual view are the sprite batch and audio engine,
//  respectively, that use this data to provide feedback on the screen.
//
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//
//  Author: Walker White
//  Version: 1/20/25
//
#ifndef __GP_ENEMY_H__
#define __GP_ENEMY_H__
#include <cugl/cugl.h>


/**
 * Model class representing an alien ship.
 */
class EnemyNode : public cugl::scene2::SpriteNode {
private:
    float _speed;

public:
#pragma mark Constructors
    /**
     * Creates a ship wiht the given position and data.
     *
     * The JsonValue should be a reference of all of the constants
     * that necessary to set the "hidden physical properties".
     *
     * @param pos   The ship position
     * @param data  The data defining the physics constants
     */
    EnemyNode(std::shared_ptr<cugl::graphics::Texture>& tex);
    
    /**
     * Disposes the ship, releasing all resources.
     */
    ~EnemyNode() {}
    
#pragma mark Graphics
};

#endif // !__GP_ENEMY_H__


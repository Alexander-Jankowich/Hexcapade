//
//  Player.cpp
//  gp
//
//  Created by Serena Duncan on 2/23/26.
//

#include "Player.h"

using namespace cugl;
using namespace cugl::graphics;

#define PI 3.14159265358979323
#define PHYSICS_SCALE 50


float PLAYER_SHAPE[] = {     -15, 0, 0, 50, 15, 0};

Player::Player(const cugl::Vec2& pos) {
    _health = 100;
    
    size_t starCount = sizeof(PLAYER_SHAPE) / (2 * sizeof(float));
    const cugl::Vec2* starArray = reinterpret_cast<const cugl::Vec2*>(PLAYER_SHAPE);
    auto star_path = cugl::Path2(starArray, starCount);
    star_path = star_path.reversed();
    
    _playerTrans = Affine2();
    _playerAngle = -PI/2; // initialize to point right
    _playerTrans.rotate(_playerAngle);
    
    //Triangulator pattern - build geometry stuff
    EarclipTriangulator tri = EarclipTriangulator();
    tri.set(star_path);
    tri.calculate();
    _polyPlayer = tri.getPolygon();
    
    //This is how to create the physics body
    // PLEASE WORK PLEASE I DONT KNOW WHAT THIS DOES
    Poly2 starCopy = _polyPlayer;
    starCopy /= PHYSICS_SCALE;
    _shape = cugl::physics2::PolygonObstacle::alloc(starCopy);
    _shape -> setBodyType(b2_dynamicBody);
    _shape -> setDensity(1);
    _shape -> setPosition(pos/(2 * PHYSICS_SCALE));
}

void Player::updateAngle(float angle) {
    _playerAngle = angle;
    _shape->setAngle(angle);
}

void Player::postUpdate(float remain) {
    _playerTrans.translate((_shape -> getLinearVelocity()) * remain);
}

void Player::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch){
    _playerTrans.rotate(_playerAngle - PI/2);
    _playerTrans.translate(_shape -> getPosition() * PHYSICS_SCALE);
    batch -> setColor(Color4::BLUE);
    batch -> fill(_polyPlayer, Vec2(0,0),_playerTrans);
}

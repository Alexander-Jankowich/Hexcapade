//
//  AnimatedSprite.h
//  hexc
//
//  Created by Kaz Tam on 4/16/26.
//

#ifndef __ANIMATED_SPRITE_H__
#define __ANIMATED_SPRITE_H__

#include <cugl/cugl.h>

using namespace cugl;
using namespace std;

class AnimatedSprite : public scene2::SpriteNode {
public:
	virtual ~AnimatedSprite() = default;


	/**
	* Returns the next animation function and duration
	* If there is no next animation (node has been removed), duration = -1
	*/
	virtual pair<ActionFunction, float> getNextAnimation() = 0;
};

#endif
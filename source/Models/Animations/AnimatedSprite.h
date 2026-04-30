//
//  AnimatedSprite.h
//  hexc
//
//  Created by Kaz Tam on 4/16/26.
//

#ifndef __ANIMATED_SPRITE_H__
#define __ANIMATED_SPRITE_H__

#include <cugl/cugl.h>

#define FPS 10.0f

using namespace cugl;
using namespace std;

class AnimatedSprite : public scene2::SpriteNode {
protected:
	void updateFrame(float t, ActionState state, int start, int numFrames) {
		int offset = state == ActionState::FINISH ? numFrames - 1 :
			state == ActionState::BEGIN ? 0 :
			min((int)(t * numFrames), numFrames - 1);
		setFrame(start + offset);
	}

	void updateFrame(float t, ActionState state, int* start, int direction, int numFrames) {
		int offset = state == ActionState::FINISH ? numFrames - 1 :
			state == ActionState::BEGIN ? 0 :
			min((int)(t * numFrames), numFrames - 1);
		setFrame(start[direction] + offset);
	}

	const pair<ActionFunction, float> END_ANIMATION = { [this](float, ActionState) {}, -1 };

public:
	virtual ~AnimatedSprite() = default;

	/**
	* Returns the next animation function and duration
	* If there is no next animation (node has been removed), duration = -1
	*/
	virtual pair<ActionFunction, float> getNextAnimation() = 0;

};

#endif // !__ANIMATED_SPRITE_H__

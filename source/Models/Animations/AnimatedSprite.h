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
	pair<ActionFunction, float> getAnimationPair(const int* start, const int numFrames) {
		return {
			[this, start, numFrames](float t, ActionState state) {
				int offset = state == ActionState::FINISH ? numFrames - 1 :
					state == ActionState::BEGIN ? 0 :
					min((int)(t * numFrames), numFrames - 1);
				setFrame(start[getDirection()] + offset);
			},
			numFrames / FPS
		};
	}

	pair<ActionFunction, float> getAnimationPair(const int start, const int numFrames) {
		return {
			[this, start, numFrames](float t, ActionState state) {
				int offset = state == ActionState::FINISH ? numFrames - 1 :
					state == ActionState::BEGIN ? 0 :
					min((int)(t * numFrames), numFrames - 1);
				setFrame(start + offset);
			},
			numFrames / FPS
		};
	}

	const pair<ActionFunction, float> END_ANIMATION = { [this](float, ActionState) {}, -1 };

	virtual int getDirection() { return 0; };

public:
	virtual ~AnimatedSprite() = default;

	/**
	* Returns the next animation function and duration
	* If there is no next animation (node has been removed), duration = -1
	*/
	virtual pair<ActionFunction, float> getNextAnimation() = 0;

};

#endif // !__ANIMATED_SPRITE_H__

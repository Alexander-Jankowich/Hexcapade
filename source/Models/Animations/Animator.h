//
//  Animator.h
//  hexc
//
//  Author: Kaz Tam
//  Version: 4/16/2026
//

#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include <cugl/cugl.h>
#include "AnimatedSprite.h"

using namespace cugl;
using namespace std;


class Animator {
private:
	unsigned long int _nextKey;
	map<string, shared_ptr<AnimatedSprite>> _active;

	shared_ptr<ActionTimeline> _timeline;

public:
	Animator();

	void update(float dt);

	/**
	* Adds a node to be animated. The animation will begin on the
	* next update call.
	*/
	void addNode(const shared_ptr<AnimatedSprite> node);
};

#endif
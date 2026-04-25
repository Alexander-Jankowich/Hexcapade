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

	// Allows us to find the animation with the node
	map<shared_ptr<AnimatedSprite>, string> _active;

	// A set of listners to be added on the next animation
	map<shared_ptr<AnimatedSprite>, ActionListener> _listeners;

	shared_ptr<ActionTimeline> _timeline;

public:
	Animator();

	static shared_ptr<Animator> alloc() {
		return make_shared<Animator>();
	}

	/**
	* Clears and resets the animator to its base state
	*/
	void reset();

	/**
	* Updates all animations. If an animation has completed, gets and starts
	* the next animation. If there is no next animation, assumes that the node
	* is no longer in use and removes it from the active list.
	*/
	void update(float dt);

	/**
	* Adds a node to be animated. The animation will begin on the
	* next update call.
	*/
	void addNode(const shared_ptr<AnimatedSprite> node);

	/**
	* Pauses the animation for this node.
	*/
	void pauseAnimation(const shared_ptr<AnimatedSprite> node);

	/**
	* Resumes the animation for this node.
	*/
	void resumeAnimation(const shared_ptr<AnimatedSprite> node);

	/**
	* Stops the current animation and will get the next animation on the next update call.
	*/
	void endAnimation(const shared_ptr<AnimatedSprite> node);

	/**
	* Adds the provided listener on the next animation for this node.
	*/
	void addNextListener(const shared_ptr<AnimatedSprite> node, ActionListener listener);
};

#endif // !__ANIMATOR_H__

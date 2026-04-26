#include "Animator.h"

Animator::Animator() {
	_timeline = ActionTimeline::alloc();
	_nextKey = 0;
	_active.clear();
	_listeners.clear();
}

void Animator::reset() {
	_active.clear();
	_listeners.clear();
	_timeline.reset();
	_nextKey = 0;
}

void Animator::update(float dt) {
	_timeline->update(dt);

	for (auto it = _active.begin(); it != _active.end(); ) {
		string key = it->second;
		shared_ptr<AnimatedSprite> node = it->first;
        node -> setPriority(0.0f);
		if (!_timeline->isActive(key)) {
			// Action has ended, need to re-add
			auto ani = node->getNextAnimation();
			if (ani.second < 0) {
				// No next animation, remove from active list
				it = _active.erase(it);
				continue;
			}
			else {
				_timeline->add(key, ani.first, ani.second);
				auto it = _listeners.find(node);
				if (it != _listeners.end()) {
					CULog("added listener");
					_timeline->addCompletionListener(key, it->second);
					_listeners.erase(it);
				}
			}
		}

		it++;
	}
}

void Animator::addNode(const shared_ptr<AnimatedSprite> node) {
	// In case we somehow get so many nodes that it overflows, make sure
	// to skip keys already in use. Normally shouldn't get here!
    node -> setPriority(-99.0);
	while (_timeline->isActive(to_string(_nextKey))) {
		_nextKey++;
	}

	_active.emplace(node, to_string(_nextKey));
	_nextKey++;
}

void Animator::pauseAnimation(const shared_ptr<AnimatedSprite> node) {
	auto it = _active.find(node);
	if (it != _active.end()) {
		// Node is in active
		_timeline->pause(it->second);
	}
}

void Animator::resumeAnimation(const shared_ptr<AnimatedSprite> node) {
	auto it = _active.find(node);
	if (it != _active.end()) {
		// Node is in active
		_timeline->unpause(it->second);
	}
}

void Animator::endAnimation(const shared_ptr<AnimatedSprite> node) {
	auto it = _active.find(node);
	if (it != _active.end()) {
		// Node is in active
		if (!_timeline->isPaused(it->second)) {
			_timeline->remove(it->second);
		}
	}
}

void Animator::addNextListener(const shared_ptr<AnimatedSprite> node, ActionListener listener) {
	_listeners.emplace(node, listener);
}

#include "Animator.h"

Animator::Animator() {
	_timeline = ActionTimeline::alloc();
	_nextKey = 0;
	_active.clear();
	_ended.clear();
	_listeners.clear();
}

void Animator::reset() {
	_active.clear();
	_ended.clear();
	_listeners.clear();
	_timeline.reset();
	_nextKey = 0;
}

void Animator::update(float dt) {
	_timeline->update(dt);

	for (const auto& pair : _ended) {
		string key = pair.second;
		shared_ptr<AnimatedSprite> node = pair.first;

		// Action has ended, need to re-add
		auto ani = node->getNextAnimation();
		if (ani.second < 0) {
			// No next animation, remove from active list
			_active.erase(node);
		}
		else {
			_timeline->add(key, ani.first, ani.second);

			// Add listener to add node to ended list when animation ends
			_timeline->addCompletionListener(key, [this, node](string key, float time, float actual) {
				endedAnimation(node, key);
			});

			// Find all listeners to be added for this node
			auto range = _listeners.equal_range(node);
			for (auto it = range.first; it != range.second; ++it) {
				//CULog("added listener");
				if (it->second.second < 0) {
					_timeline->addCompletionListener(key, it->second.first);
				}
				else {
					_timeline->addListener(key, it->second.second, it->second.first);
				}
			}
			_listeners.erase(node);
		}
	}

	_ended.clear();
}

void Animator::addNode(const shared_ptr<AnimatedSprite> node) {
	// In case we somehow get so many nodes that it overflows, make sure
	// to skip keys already in use. Normally shouldn't get here!
	while (_timeline->isActive(to_string(_nextKey))) {
		_nextKey++;
	}

	_active.emplace(node, to_string(_nextKey));
	endedAnimation(node, to_string(_nextKey));
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
			endedAnimation(it->first, it->second);
		}
	}
}

void Animator::addNextListener(const shared_ptr<AnimatedSprite> node, ActionListener listener) {
	addNextListener(node, -1, listener);
}

void Animator::addNextListener(const shared_ptr<AnimatedSprite> node, float time, ActionListener listener) {
	_listeners.insert({ node, make_pair(listener, time) });
	//CULog("to add listeners");
}

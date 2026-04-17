#include "Animator.h"

Animator::Animator() {
	_timeline = ActionTimeline::alloc();
	_nextKey = 0;
	_active.clear();
}

void Animator::update(float dt) {
	_timeline->update(dt);

	for (auto it = _active.begin(); it != _active.end(); ) {
		string key = it->first;
		shared_ptr<AnimatedSprite> node = it->second;

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
			}
		}

		it++;
	}
}

void Animator::addNode(const shared_ptr<AnimatedSprite> node) {
	_active.emplace(to_string(_nextKey), node);
	_nextKey++;
}
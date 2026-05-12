//
//  SpellEntityNode.cpp
//  hexc
//
//  Created by Kaz Tam on 4/29/26.
//

#include "SpellEntityNode.h"

using namespace cugl;
using namespace std;

SpellEntityNode::SpellEntityNode(shared_ptr<graphics::Texture> texture, shared_ptr<JsonValue> consts) :
		CREATED(consts->getInt("created")),
		CREATED_FRAMES(consts->getInt("created_frames")),
		EXISTING(consts->getInt("existing")),
		EXISTING_FRAMES(consts->getInt("existing_frames")),
		END(consts->getInt("end")),
		END_FRAMES(consts->getInt("end_frames")) {
	initWithSheet(texture, consts->getInt("rows"), consts->getInt("columns"));
	_shouldEnd = false;
	_ended = false;
	_finishedCreate = false;
	_existingOffset = 0;

	setFrame(CREATED);
}

pair<ActionFunction, float> SpellEntityNode::getNextAnimation() {
	if (_ended) {
		return END_ANIMATION;
	}

	if (!_finishedCreate) {
		_finishedCreate = true;
		return getAnimationPair(CREATED, CREATED_FRAMES);
	}

	if (_shouldEnd) {
		_ended = true;
		return getAnimationPair(END, END_FRAMES);
	}

	return getAnimationPair(EXISTING + _existingOffset, EXISTING_FRAMES);
}
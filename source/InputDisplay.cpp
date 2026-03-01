//
//  InputDisplay.cpp
// 
//  Created by Kaz Tam on 2/28/2026
//

#include "InputDisplay.h"

using namespace cugl;
using namespace cugl::graphics;

InputDisplay::InputDisplay(InputController* input) {
	_input = input;

	PolyFactory factory = PolyFactory();
	_joybase = factory.makeCircle(0, 0, _input->getJoystickRadius());
	_joystick = factory.makeCircle(0, 0, _input->getJoystickRadius() / 2);

	_showJoystick = false;
}

void InputDisplay::update() {
	static SimpleExtruder extruder = SimpleExtruder();

	_showJoystick = _input->showJoystick();
	if (_showJoystick) {
		// Update component locations
		_jbaseTrans = Affine2::createTranslation(_input->getJoystickCenter());
		_jstickTrans = Affine2::createTranslation(_input->getJoystickPos());
	}

	_gesture.set(_input->getCurStroke());
	extruder.set(_gesture);
	extruder.calculate(_gestureWidth);
	_gestureDisplay = extruder.getPolygon();
}

void InputDisplay::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
	batch->setTexture(nullptr);

	if (_showJoystick) {
		batch->setColor(Color4(144, 144, 144, 150));
		batch->fill(_joybase, Vec2(0, 0), _jbaseTrans);

		batch->setColor(Color4(89, 89, 89, 240));
		batch->fill(_joystick, Vec2(0, 0), _jstickTrans);
	}

	if (_gesture.size() > 3) {
		batch->setColor(Color4(199, 140, 6, 150));
		batch->fill(_gestureDisplay);
	}
}

void InputDisplay::dispose() {
	// Nothing for now
	_input = nullptr;
}
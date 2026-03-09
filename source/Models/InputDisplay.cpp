//
//  InputDisplay.cpp
// 
//  Created by Kaz Tam on 2/28/2026
//

#include "InputDisplay.h"

using namespace cugl;
using namespace cugl::graphics;

float ARROW[] = {
	0, 5,
	0, -5,
	70, -5,
	70, -10,
	75, 0,
	70, 10,
	70, 5
};

InputDisplay::InputDisplay(InputController* input) {
	_input = input;

	PolyFactory factory = PolyFactory();
	_joybase = factory.makeCircle(0, 0, _input->getJoystickRadius());
	_joystick = factory.makeCircle(0, 0, _input->getJoystickRadius() / 2);

	_showJoystick = false;

	_showAim = false;
	EarclipTriangulator triangulator = EarclipTriangulator();
	Path2 arrow = Path2(reinterpret_cast<Vec2*>(ARROW), 7);
    triangulator.set(arrow);
    triangulator.calculate();
	_arrow = triangulator.getPolygon();
}

void InputDisplay::update(Vec2 playerPos) {
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

	_showAim = _input->showAim() && !_input->getSpellAngle().isZero();
	if (_showAim) {
		_arrowTrans = Affine2::createRotation(_input->getSpellAngle().getAngle());
		_arrowTrans.translate(playerPos);
	}
}

void InputDisplay::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
	batch->setTexture(nullptr);

	if (_showJoystick) {
		batch->setColor(Color4(144, 144, 144, 150));
		batch->fill(_joybase, Vec2::ZERO, _jbaseTrans);

		batch->setColor(Color4(89, 89, 89, 240));
		batch->fill(_joystick, Vec2::ZERO, _jstickTrans);
	}

	if (_gesture.size() > 3) {
		batch->setColor(Color4(199, 140, 6, 150));
		batch->fill(_gestureDisplay);
	}

	if (_showAim) {
		batch->setColor(Color4(199, 140, 6, 255));
		batch->fill(_arrow, Vec2::ZERO, _arrowTrans);
	}
}

void InputDisplay::dispose() {
	_input = nullptr;
}
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

Color4 colorFromVec(std::vector<int> vec) {
	return Color4(vec[0], vec[1], vec[2], vec[3]);
}

InputDisplay::InputDisplay(InputController* input, std::shared_ptr<JsonValue> values) {
	_input = input;
	_uiOffset = Vec2::ZERO;

	// Set joystick constants
	PolyFactory factory = PolyFactory();
	_joybase = factory.makeCircle(0, 0, values->get("joystick")->getInt("radius"));
	_joystick = factory.makeCircle(0, 0, values->get("joystick")->getInt("radius") / 2);
	_baseColor = colorFromVec(values->get("joystick")->get("base_color")->asIntArray());
	_joystickColor = colorFromVec(values->get("joystick")->get("joystick_color")->asIntArray());

	_showJoystick = false;

	// Set aim arrow constants
	_showAim = false;
	EarclipTriangulator triangulator = EarclipTriangulator();
	Path2 arrow = Path2(reinterpret_cast<Vec2*>(ARROW), 7);
    triangulator.set(arrow);
    triangulator.calculate();
	_arrow = triangulator.getPolygon();
	_arrowColor = colorFromVec(values->get("aim")->get("color")->asIntArray());

	// Set gesture constants
	_gestureWidth = values->get("gesture_trail")->getInt("width");
	_gestureColor = colorFromVec(values->get("gesture_trail")->get("color")->asIntArray());
	_gestureColorCorrect = colorFromVec(values->get("gesture_trail")->get("correct-color")->asIntArray());
	_gestureColorWrong = colorFromVec(values->get("gesture_trail")->get("incorrect-color")->asIntArray());
}

void InputDisplay::update(Vec2 playerPos) {
	static SimpleExtruder extruder = SimpleExtruder();

	_showJoystick = _input->showJoystick();
	if (_showJoystick) {
		// keep the on-screen joystick anchored to the camera-adjusted hud space
		_jbaseTrans = Affine2::createTranslation(_input->getJoystickCenter());
		_jstickTrans = Affine2::createTranslation(_input->getJoystickPos());
	}

	_numStrokes = _input->getNumStrokes();
	_gestureTime = _input->getTimeSinceLastStroke();
	_gestureCorrect = _input->wasGestureRecognized();
	if (!_gestureCorrect && _numStrokes < 3) {
		_gesture = _input->getCurStroke();
		_gesture.set(_input->getCurStroke());
		extruder.set(_gesture);
		extruder.set(_gesture);
		extruder.calculate(_gestureWidth);
		_gestureDisplay = extruder.getPolygon();
		_gesture2.set(_input->getsecondStroke());
		extruder.set(_gesture2);
		extruder.calculate(_gestureWidth);
		_secondDisplay = extruder.getPolygon();
		_gesture3.set(_input->getthirdStroke());
		extruder.set(_gesture3);
		extruder.calculate(_gestureWidth);
		_thirdDisplay = extruder.getPolygon();
	}

	_showAim = _input->showAim() && !_input->getSpellAngle().isZero();
	if (_showAim) {
		// keep the aim arrow in world space so it still follows the player
		_arrowTrans = Affine2::createRotation(_input->getSpellAngle().getAngle());
		_arrowTrans.translate(playerPos);
	}
}

void InputDisplay::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
	batch->setTexture(nullptr);

	if (_showJoystick) {
		batch->setColor(_baseColor);
		batch->fill(_joybase, Vec2::ZERO, _jbaseTrans.translate(_uiOffset));

		batch->setColor(_joystickColor);
		batch->fill(_joystick, Vec2::ZERO, _jstickTrans.translate(_uiOffset));
	}

	if (_gestureCorrect) {
		if (_gestureTime < 0.5) {
			batch->setColor(_gestureColorCorrect);
			if (_gestureDisplay.size() > 3) {
				batch->fill(_gestureDisplay, _uiOffset);
			}
			if (_secondDisplay.size() > 3) {
				batch->fill(_secondDisplay, _uiOffset);
			}
			if (_thirdDisplay.size() > 3) {
				batch->fill(_thirdDisplay, _uiOffset);
			}
		}
	}
	else if (_numStrokes > 2) {
		if (_gestureTime < 0.25) {
			batch->setColor(_gestureColorWrong);
			batch->fill(_gestureDisplay, _uiOffset);
			batch->fill(_secondDisplay, _uiOffset);
			batch->fill(_thirdDisplay, _uiOffset);
		}
	}
	else if (_gestureTime < 1 || _gestureDisplay.size() > 3) {
		batch->setColor(_gestureColor);
		if (_gestureDisplay.size() > 3) {
			batch->fill(_gestureDisplay, _uiOffset);
		}
		if (_secondDisplay.size() > 3) {
			batch->fill(_secondDisplay, _uiOffset);
		}
		if (_thirdDisplay.size() > 3) {
			batch->fill(_thirdDisplay, _uiOffset);
		}
	}

	if (_showAim) {
		batch->setColor(_arrowColor);
		batch->fill(_arrow, Vec2::ZERO, _arrowTrans);
	}

	batch->setColor(Color4::WHITE);
}

void InputDisplay::dispose() {
	_input = nullptr;
}

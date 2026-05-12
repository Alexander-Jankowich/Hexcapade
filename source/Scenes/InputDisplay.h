//
//  InputDisplay.h
//  Hexcapade
//
//  This class takes data from InputController and draws it on screen
//  to give the user feedback about what they're doing.
//
//  Author: Kaz Tam
//  Version: 3/1/26
//

#ifndef __INPUT_DISPLAY_H__
#define __INPUT_DISPLAY_H__
#include <cugl/cugl.h>
#include "../Controllers/InputController.h"

class InputDisplay {
public:
	InputDisplay(InputController* input, std::shared_ptr<cugl::JsonValue> values);

	void setInputController(InputController* input) { _input = input; }

	// this lets gamescene keep screen-space ui visuals anchored while the
	// camera follows the world map
	void setUIOffset(const cugl::Vec2& offset) { _uiOffset = offset; }
	void setUIZoom(float zoom) { _uiZoom = std::max(zoom, 0.01f); }

	/**
	* Updates values based on _input (should be called after _input is
	* updated). Does not render the input.
	*/
	void update(cugl::Vec2 playerPos);

	/**
	* Draws the input using batch. Does not update the input, call update()
	* before to do so.
	*/
	void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

	void dispose();

	/** Resets the state of the display */
	void reset();

private:
	/** The input controller */
	InputController* _input;

	/** Whether the joystick should be drawn */
	bool _showJoystick;

	/** Joystick attributes */
	cugl::Affine2 _jbaseTrans;
	cugl::Affine2 _jstickTrans;
	cugl::Poly2 _joybase;
	cugl::Poly2 _joystick;
	cugl::Color4 _baseColor;
	cugl::Color4 _joystickColor;

	/** Gesture attributes */
	cugl::Path2 _gesture;
    cugl::Path2 _gesture2;
    cugl::Path2 _gesture3;
	cugl::Poly2 _gestureDisplay;
    cugl::Poly2 _secondDisplay;
    cugl::Poly2 _thirdDisplay;
	int _gestureWidth;
	cugl::Color4 _gestureColor;
	cugl::Color4 _gestureColorWrong;
	cugl::Color4 _gestureColorCorrect;
    cugl::Color4 _gestureColorLess;
	int _numStrokes;
	float _gestureTime;
	bool _gestureCorrect;
    bool _gestureLessCorrect;

	// this stores the current camera offset for screen-space ui visuals
	cugl::Vec2 _uiOffset;
	float _uiZoom;

	bool _showAim;
	cugl::Affine2 _arrowTrans;
	cugl::Poly2 _arrow;
	cugl::Color4 _arrowColor;
};

#endif // !__INPUT_DISPLAY_H__

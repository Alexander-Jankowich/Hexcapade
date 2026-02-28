#ifndef __INPUT_MODEL_H__
#define __INPUT_MODEL_H__
#include <cugl/cugl.h>
#include "GLInputController.h"

class InputDisplay {
public:
	InputDisplay(InputController* input);

	void setInputController(InputController* input) { _input = input; }

	void update();

	void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

	//void setTextures();

	void dispose();

private:
	InputController* _input;

	bool _showJoystick;

	cugl::Affine2 _jbaseTrans;
	cugl::Affine2 _jstickTrans;
	cugl::Poly2 _joybase;
	cugl::Poly2 _joystick;
};

#endif
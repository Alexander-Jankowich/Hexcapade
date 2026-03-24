//
//  GLInputController.cpp
//  Geometry Lab
//
//  This class buffers in input from the devices and converts it into its
//  semantic meaning. If your game had an option that allows the player to
//  remap the control keys, you would store this information in this class.
//  That way, the main game scene does not have to keep track of the current
//  key mapping.
//
//  Author: Walker M. White
//  Version: 1/29/24
//
#include <cugl/cugl.h>
#include "GLInputController.h"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::D
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

/** How close we need to be for a multi touch */
#define NEAR_TOUCH      100
/** The key for the event handlers */
#define LISTENER_KEY      1

/** How fast a double click must be in milliseconds */
#define DOUBLE_CLICK    400

std::vector<std::vector<Vec2>> X_GESTURE = {
    {
        {1026.695312, 544.195312},
        {1026.695312, 543.953125},
        {1012.515625, 529.476562},
        {969.273438, 487.671875},
        {902.906250, 426.726562},
        {847.929688, 377.593750},
        {815.179688, 349.468750},
        {794.531250, 333.039062},
        {779.953125, 321.648438},
        {774.023438, 317.710938},
        {772.375000, 317.421875},
        {772.328125, 319.281250},
        {772.328125, 319.281250}
    },
    {
        {786.523438, 544.015625},
        {788.179688, 543.070312},
        {813.078125, 526.609375},
        {864.593750, 501.273438},
        {943.875000, 457.046875},
        {988.789062, 430.570312},
        {1036.046875, 401.820312},
        {1097.742188, 360.757812},
        {1111.539062, 351.742188},
        {1116.757812, 348.515625},
        {1117.578125, 348.007812},
        {1117.578125, 348.007812}
    }
};


std::vector<std::vector<Vec2>> X_GESTURE_2 = {
    {
        {1338.625977, 449.465637},
        {1312.977051, 431.755707},
        {1301.984741, 419.541992},
        {1286.106934, 403.053406},
        {1243.969482, 354.198425},
        {1200.000000, 294.961823},
        {1171.297729, 253.435089},
        {1152.977051, 226.564911},
        {1141.374023, 210.687042},
        {1136.488525, 203.358765},
        {1134.656494, 199.083939},
        {1134.656494, 196.030563},
        {1134.656494, 194.198502},
        {1134.656494, 194.198502}
    },
    {
        {1116.335938, 432.366394},
        {1133.435059, 423.816742},
        {1147.480835, 398.778564},
        {1163.358765, 363.358734},
        {1179.847290, 323.053406},
        {1198.167969, 283.358765},
        {1216.488525, 248.549591},
        {1231.755737, 218.625931},
        {1243.358765, 195.419876},
        {1250.076294, 181.374023},
        {1253.129761, 174.045822},
        {1253.129761, 171.603043},
        {1253.129761, 172.213730},
        {1253.129761, 172.213730}
    }
};



std::vector<std::vector<Vec2>> X_GESTURE_3 = {
    {
        {1313.587769, 451.908356},
        {1267.786255, 407.938904},
        {1244.580078, 383.511444},
        {1225.038086, 359.083954},
        {1190.839600, 315.725159},
        {1164.580078, 279.083954},
        {1146.259521, 250.381653},
        {1137.099243, 233.893112},
        {1130.381714, 219.236618},
        {1124.274780, 204.580139},
        {1120.610718, 191.755722},
        {1119.389282, 188.702316},
        {1119.389282, 188.702316}
    },
    {
        {1092.519043, 404.885498},
        {1100.458008, 400.000000},
        {1113.893066, 393.282410},
        {1123.053467, 385.954193},
        {1132.824463, 374.961792},
        {1154.198486, 349.923676},
        {1174.961792, 321.832031},
        {1195.114502, 294.351135},
        {1214.045776, 268.091583},
        {1229.312988, 244.885468},
        {1241.526611, 224.122147},
        {1248.854858, 208.244263},
        {1251.908447, 199.083939},
        {1252.519043, 196.030563},
        {1252.519043, 195.419876},
        {1252.519043, 195.419876}
    }
};

std::vector<std::vector<Vec2>> X_GESTURE_4 = {
    {
        {1116.335938, 459.236572},
        {1098.625977, 441.526672},
        {1078.473267, 424.427429},
        {1049.160278, 395.725189},
        {1027.175537, 371.297699},
        {1007.633545, 346.870178},
        {988.091553, 322.442719},
        {969.770935, 298.625977},
        {953.282410, 277.251892},
        {941.068665, 261.374023},
        {934.961792, 252.824432},
        {933.129761, 250.381653},
        {935.572449, 252.824432},
        {937.404541, 253.435089},
        {937.404541, 253.435089}
    },
    {
        {961.221313, 455.572479},
        {974.045776, 444.580109},
        {1005.190796, 408.549591},
        {1024.732788, 386.564880},
        {1043.053467, 361.526672},
        {1061.374023, 337.099213},
        {1079.694580, 313.893097},
        {1095.572510, 294.351135},
        {1108.396973, 278.473267},
        {1117.557251, 264.427460},
        {1124.274780, 254.045776},
        {1127.938965, 246.717545},
        {1129.160278, 243.664093},
        {1129.160278, 243.053406},
        {1129.160278, 243.053406}
    }
};


std::vector<std::vector<Vec2>> HEAL_GESTURE = {
    {
        {799.082031, 316.132843},
        {799.679688, 316.734406},
        {807.484375, 323.906281},
        {826.242188, 341.160187},
        {852.984375, 365.937531},
        {894.757812, 408.417969},
        {919.265625, 435.593781},
        {939.195312, 460.546906},
        {950.011719, 474.015686},
        {955.183594, 480.449249},
        {960.617188, 486.992218},
        {964.894531, 492.007874},
        {966.800781, 494.242249},
        {967.921875, 495.601593},
        {968.675781, 496.359436},
        {969.109375, 496.761780},
        {969.109375, 496.761780}
    },
    {
        {795.699219, 403.976593},
        {796.460938, 405.593781},
        {801.906250, 412.023468},
        {808.433594, 419.136749},
        {835.414062, 447.140686},
        {848.859375, 459.750000},
        {860.589844, 470.492249},
        {872.089844, 480.949249},
        {882.832031, 490.027405},
        {892.128906, 497.687531},
        {899.187500, 503.429749},
        {904.375000, 507.878937},
        {910.617188, 513.406311},
        {912.457031, 515.308655},
        {913.503906, 516.074280},
        {913.761719, 516.335938},
        {913.761719, 516.335938}
    },
    {
        {784.847656, 499.066437},
        {784.847656, 498.152405},
        {784.847656, 494.964905},
        {784.847656, 491.468811},
        {784.847656, 485.953186},
        {784.847656, 478.257874},
        {784.847656, 466.500061},
        {784.847656, 452.058655},
        {784.847656, 447.300842},
        {784.847656, 428.250031},
        {784.847656, 413.656281},
        {784.847656, 397.332062},
        {784.847656, 378.417999},
        {785.070312, 361.250031},
        {785.917969, 344.394562},
        {787.148438, 328.058624},
        {788.351562, 312.664093},
        {789.531250, 298.175812},
        {790.679688, 284.394562},
        {791.925781, 264.355499},
        {792.179688, 259.273468},
        {792.761719, 246.390640},
        {792.761719, 237.949234},
        {792.761719, 232.621124},
        {792.761719, 226.812531},
        {792.761719, 220.750031},
        {792.761719, 215.339859},
        {792.761719, 211.187515},
        {792.761719, 205.660172},
        {792.761719, 199.609390},
        {792.761719, 193.523453},
        {792.761719, 187.125015},
        {792.761719, 181.449219},
        {792.761719, 176.203140},
        {792.761719, 170.984406},
        {792.761719, 166.433609},
        {793.097656, 160.644547},
        {793.097656, 158.019547},
        {793.097656, 157.082047},
        {793.410156, 154.222656},
        {793.410156, 152.675797},
        {793.691406, 151.292999},
        {793.691406, 150.128922},
        {793.691406, 149.406265},
        {793.691406, 148.375015},
        {793.691406, 147.199234},
        {793.691406, 146.132812},
        {793.691406, 145.390625},
        {793.691406, 144.863281},
        {793.691406, 144.335938},
        {793.328125, 144.417984}
    }
};

std::vector<std::vector<Vec2>> HEAL_GESTURE_2 = {
    {
        {1052.213745, 230.228973},
        {1058.931274, 237.557220},
        {1077.862549, 254.656494},
        {1099.847412, 277.251892},
        {1124.885498, 306.564850},
        {1151.755615, 337.709900},
        {1177.404541, 364.580139},
        {1198.778564, 385.954193},
        {1212.824463, 396.946503},
        {1225.648804, 401.221344},
        {1229.312988, 403.053406},
        {1229.312988, 403.053406},
    },
    {
        {1033.282349, 327.938904},
        {1043.053467, 329.160278},
        {1055.267090, 340.152649},
        {1076.641235, 359.083954},
        {1102.290039, 382.290039},
        {1124.274780, 401.221344},
        {1138.931274, 412.213684},
        {1147.480835, 417.709900},
        {1154.809204, 421.374023},
        {1160.305298, 424.427429},
        {1167.022949, 427.480865},
        {1173.129761, 429.923615},
        {1181.679321, 432.366394},
        {1181.679321, 432.366394}
    },
    {
        {1027.175537, 436.641174},
        {1033.893066, 426.870178},
        {1033.282349, 422.595398},
        {1033.282349, 404.885498},
        {1033.282349, 370.076324},
        {1033.282349, 319.999939},
        {1034.503784, 271.755737},
        {1038.167969, 229.618301},
        {1041.832031, 194.198502},
        {1044.885498, 167.938934},
        {1046.717529, 149.007660},
        {1047.938965, 130.687042},
        {1047.938965, 131.908417},
        {1047.938965, 134.961823},
        {1048.549561, 137.404572},
        {1048.549561, 137.404572},
    }
};




// The screen is divided into four zones: Left and Right
//
//   |---------------|
//   |       |       |
//   | L     |     R |
//   |       |       |
//   |---------------|
//
// The meaning of any touch depends on the zone it begins in.

/** The portion of the screen used for the left zone */
#define LEFT_ZONE       0.5f
/** The portion of the screen used for the right zone */
#define RIGHT_ZONE      0.5f

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
InputController::InputController() :
_active(false),
_currDown(false),
_prevDown(false),
_mouseDown(false),
_touchDown(false),
_touchID(-1),
_mouseKey(0),
_joystick(false),
_pickupPressed(false),
_dropPressed(false),
_depositPressed(false),
_contextActionPressed(false),
_inventoryTogglePressed(false),
_inventoryPanelPressed(false),
_inventoryPanelOpen(false),
_lastUITap(Vec2::ZERO),
_resetPressed(false),
_lastGesture(SpellType::NONE),
_isAiming(false),
_releaseSpell(false),
_timeSinceCast(0),
_spellAngle(Vec2::ZERO)
{
}


/**
 * Initializes the control to support mouse or touch.
 *
 * This method attaches all of the listeners. It tests which
 * platform we are on (mobile or desktop) to pick the right
 * listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @return true if the initialization was successful
 */
bool InputController::init(std::shared_ptr<JsonValue> values, const Rect bounds,
                            std::function<cugl::Vec2(const cugl::Vec2&)> screenToWorld) {
    _screenToWorld = screenToWorld;
    recognizer = cugl::PGestureRecognizer::alloc();
    recognizer -> addGesture("V",  X_GESTURE );
    recognizer -> addGesture("Z", HEAL_GESTURE);
    recognizer -> addGesture("Z", HEAL_GESTURE_2);
    recognizer -> addGesture("V", X_GESTURE_2);
    recognizer -> addGesture("V", X_GESTURE_3);

    recognizer->set_accuracy(0.80f);

    // Update the gesturemap with more spells later on
    _gesturemap = {
        {"V", SpellType::FIREBALL},
        {"C", SpellType::FREEZE},
        { "Z", SpellType::HEAL }
    };

    _joyradius = values->get("joystick")->getInt("radius");
    _deadzone = values->get("joystick")->getInt("dead_zone");

    
    //
#ifndef CU_TOUCH_SCREEN
    _sbounds = bounds;
    //
    Size display = Application::get()->getDisplaySize();
    _tbounds = Rect(0,0,display.width,display.height);
        
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
#else
    Size display = Application::get()->getDrawableSize();
    _tbounds = Rect(0,0,display.width,display.height);
    _sbounds = _tbounds;
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
    
#endif


    return true;
}



/**
 * Disposes this input controller, deactivating all listeners.
 *
 * As the listeners are deactived, the user will not be able to
 * monitor input until the controller is reinitialized with the
 * {@link #init} method.
 */
void InputController::dispose() {
#ifndef CU_TOUCH_SCREEN
    Input::deactivate<Keyboard>();
    Mouse* mouse = Input::get<Mouse>();
    mouse->removePressListener(LISTENER_KEY);
    mouse->removeReleaseListener(LISTENER_KEY);
    mouse->removeDragListener(LISTENER_KEY);
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    touch->removeBeginListener(LISTENER_KEY);
    touch->removeEndListener(LISTENER_KEY);
#endif
    _active = false;
}


/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 * 
 * @param dt the time (in seconds) since this was last called
 */
void InputController::update(float dt) {
    readInput();

    // Update _isAiming
    _timeSinceCast += dt;
    if (_timeSinceCast > 10000 && _isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        currentStroke.clear();
        //CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
    }
}

/**
 * Reads the input for this player and converts the result into game logic.
 *
 * This is an example of polling input.  Instead of registering a listener,
 * we ask the controller about its current state.  When the game is running,
 * it is typically best to poll input instead of using listeners.  Listeners
 * are more appropriate for menus and buttons (like the loading screen).
 */
void InputController::readInput(){
#ifndef CU_TOUCH_SCREEN
    _pickupPressed = false;
    _dropPressed = false;
    _depositPressed = false;
    _contextActionPressed = false;
    _inventoryTogglePressed = false;
    _inventoryPanelPressed = false;
    _resetPressed = false;

    Keyboard* keys = Input::get<Keyboard>();
    if (keys) {
        // Keyboard input for movement
        bool up = keys->keyDown(KeyCode::ARROW_UP) || keys->keyDown(KeyCode::W);
        bool down = keys->keyDown(KeyCode::ARROW_DOWN) || keys->keyDown(KeyCode::S);
        bool left = keys->keyDown(KeyCode::ARROW_LEFT) || keys->keyDown(KeyCode::A);
        bool right = keys->keyDown(KeyCode::ARROW_RIGHT) || keys->keyDown(KeyCode::D);
        static bool upPressed = false;
        static bool downPressed = false;
        static bool leftPressed = false;
        static bool rightPressed = false;

        if (up && !upPressed) {
            _move.y = 1;
            upPressed = true;
        }
        if (down && !downPressed) {
            _move.y = -1;
            downPressed = true;
        }
        if (!up && !down) {
            _move.y = 0;
        }
        upPressed = up;
        downPressed = down;

        if (left && !leftPressed) {
            _move.x = -1;
            leftPressed = true;
        }
        if (right && !rightPressed) {
            _move.x = 1;
            rightPressed = true;
        }
        if (!left && !right) {
            _move.x = 0;
        }
        leftPressed = left;
        rightPressed = right;

        _move.normalize();


        // Keyboard input for item pickup
        bool pickup = keys->keyDown(KeyCode::E);
        static bool pickupDown = false;

        _pickupPressed = pickup && !pickupDown;
        pickupDown = pickup;

        // Keyboard input for dropping and depositing while we prototype items
        // on the networking branch. These are scene-level requests; the scene
        // decides whether the action is locally allowed this frame.
        bool drop = keys->keyDown(KeyCode::Q);
        bool deposit = keys->keyDown(KeyCode::F);
        bool inventoryToggle = keys->keyDown(KeyCode::I);
        static bool dropDown = false;
        static bool depositDown = false;
        static bool inventoryToggleDown = false;

        _dropPressed = drop && !dropDown;
        _depositPressed = deposit && !depositDown;
        _inventoryTogglePressed = inventoryToggle && !inventoryToggleDown;

        dropDown = drop;
        depositDown = deposit;
        inventoryToggleDown = inventoryToggle;


        // Keyboard inputs for spells
        bool space = keys->keyDown(KeyCode::SPACE);
        bool shift = keys->keyDown(KeyCode::LEFT_SHIFT);
        static bool spacePressed = false;
        static bool shiftPressed = false;
        if (space && !spacePressed) {
            _lastGesture = SpellType::FIREBALL;
            _spellAngle.setZero();
        }
        spacePressed = space;

        if (shift && !shiftPressed) {
            _lastGesture = SpellType::FREEZE;
            _spellAngle.setZero();
        }
        shiftPressed = shift;


        // Keyboard input for reset
        bool r = keys->keyDown(KeyCode::R);
        static bool rPressed = false;

        _resetPressed = r && !rPressed;
        rPressed = r;
    }

#endif // !CU_TOUCH_SCREEN
};

#pragma mark Touch Callbacks / Controls

/**
 * Defines the zone boundaries, so we can quickly categorize touches.
 */
void InputController::createZones() {
    _lzone = _sbounds;
    _lzone.size.width *= LEFT_ZONE;
    _rzone = _sbounds;
    _rzone.size.width *= RIGHT_ZONE;
    _rzone.origin.x = _sbounds.origin.x+_sbounds.size.width-_rzone.size.width;
}

/**
 * Populates the initial values of the input TouchInstance
 */
void InputController::clearTouchInstance(TouchInstance& touchInstance) {
    touchInstance.touchids.clear();
    touchInstance.position = Vec2::ZERO;
}

/**
 * Returns the correct zone for the given position.
 *
 * See the comments above for a description of how zones work.
 *
 * @param  pos  a position in screen coordinates
 *
 * @return the correct zone for the given position.
 */
InputController::Zone InputController::getZone(const Vec2 pos) const {
    if (_lzone.contains(pos)) {
        return Zone::LEFT;
    } else if (_rzone.contains(pos)) {
        return Zone::RIGHT;
    } else if (_sbounds.contains(pos)) {
        return Zone::MAIN;
    }
    return Zone::UNDEFINED;
}

/**
 * Returns the scene location of a touch
 *
 * Touch coordinates are inverted, with y origin in the top-left
 * corner. This method corrects for this and scales the screen
 * coordinates down on to the scene graph size.
 *
 * @return the scene location of a touch
 */
Vec2 InputController::touch2Screen(const Vec2 pos) const {
    return _screenToWorld(pos);
}

Vec2 InputController::mouse2Screen(const Vec2 pos) const {
    return _screenToWorld(pos);
}

/**
 * Processes movement for the floating joystick.
 *
 * This will register movement as left or right (or neither).  It
 * will also move the joystick anchor if the touch position moves
 * too far.
 *
 * @param  pos  the current joystick position
 */
void InputController::processJoystick(const cugl::Vec2 pos) {
    _joystickpos = pos;
    Vec2 diff = pos - _joycenter;
    float dist = diff.length();

    // Clamp to joystick radius
    if (dist > _joyradius) {
        Vec2 excess = diff;
        diff.normalize();
        diff *= _joyradius;

        excess -= diff;
        // Move joystick in excess direction
        _joycenter += excess;
    }

    if (dist > _deadzone) {
        _joystick = true;

        // Scale to [-1,1] range without normalizing
        float scale = 1.0f / _joyradius;

        float x = diff.x * scale;
        float y = diff.y * scale;

        // Clamp just in case
        x = clampf(x, -1.0f, 1.0f);
        y = clampf(y, -1.0f, 1.0f);

        _move = Vec2(x, y);
    } else {
        _joystick = false;
        _move = Vec2(0,0);
    }
}

void InputController::processGestureEnd(const cugl::Vec2 pos) {
    if (_isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        _spellAngle = pos - _touchStart;
        //CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
        return;
    }

    _spellAngle.setZero();
    currentStroke.push_back(pos);
    if (currentStroke.size() > 3) {
        try {
            if((currentStroke.size() > 3) & !secondStroke.empty() & !thirdStroke.empty()){
                std::string gesture = recognizer->match({ currentStroke });
                std::string gesture2 = recognizer -> match({ currentStroke, secondStroke });
                std::string gesture3 = recognizer -> match({ currentStroke, secondStroke, thirdStroke });
                if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture2, {currentStroke, secondStroke})){
                    if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture3, {currentStroke, secondStroke, thirdStroke})){
                        _lastGesture = _gesturemap[gesture];
                    }
                    _lastGesture = _gesturemap[gesture3];
                }else{
                    if(recognizer -> similarity(gesture2,{ currentStroke, secondStroke }) > recognizer -> similarity(gesture3, {currentStroke, secondStroke, thirdStroke})){
                        _lastGesture = _gesturemap[gesture2];
                    }
                    _lastGesture = _gesturemap[gesture3];
                }
            }else if((currentStroke.size() > 3) & !secondStroke.empty()){
                std::string gesture = recognizer->match({ currentStroke });
                std::string gesture2 = recognizer -> match({ currentStroke, secondStroke });
                
                if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture2, {currentStroke, secondStroke})){
                    _lastGesture = _gesturemap[gesture];
                }else{
                    _lastGesture = _gesturemap[gesture2];
                }
            }else if (currentStroke.size() > 3){
                std::string gesture = recognizer->match({ currentStroke });
                _lastGesture = _gesturemap[gesture];
            }
            _timeSinceCast = 0;
        }
        catch (std::out_of_range) {
            // Not a recognized gesture... probably
            _lastGesture = SpellType::NONE;
        }

        if (_lastGesture != SpellType::NONE) {
            _isAiming = true;
        }

        //CULog("%s", gesture.c_str());
    }
    else {
        // Taps in the gesture zone no longer imply pickup now that mobile item
        // actions live behind explicit HUD buttons.
        _lastGesture = SpellType::NONE;
    }
    CULog("First Stroke points:");
        for (const auto& p : currentStroke) {
            CULog("{%f, %f},", p.x, p.y);
        }
    CULog("Second Stroke points:");
        for (const auto& p : secondStroke) {
            CULog("{%f, %f},", p.x, p.y);
        }
    CULog("Third Stroke points:");
        for (const auto& p : thirdStroke) {
            CULog("{%f, %f},", p.x, p.y);
        }

    // Clear points
    thirdStroke = secondStroke;
    secondStroke = currentStroke;
    currentStroke.clear();
}

void InputController::processDrag(const cugl::Vec2 pos) {
    if (_isAiming) {
        _spellAngle = pos - _touchStart;
    }
    else {
        currentStroke.push_back(pos);
    }
}

/**
 * Callback for the beginning of a touch event
 *
 * @param event The associated event
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchBeganCB(const TouchEvent& event, bool focus) {
    _touchDown = true;
    //CULog("Touch began %lld", event.touch);

    _touchDown = true;
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    pos = touch2Screen(pos);
    
    CULog("Touch pos world: (%.2f, %.2f), zone: %d", pos.x, pos.y, (int)zone);
    CULog("lzone: (%.2f,%.2f,%.2f,%.2f)", _lzone.origin.x, _lzone.origin.y, _lzone.size.width, _lzone.size.height);
    CULog("rzone: (%.2f,%.2f,%.2f,%.2f)", _rzone.origin.x, _rzone.origin.y, _rzone.size.width, _rzone.size.height);
    switch (zone){
        case Zone::LEFT:
            // Only process if no touch in zone
            if (_ltouch.touchids.empty()) {
                // Left is the floating joystick
                _ltouch.position = event.position;
                _ltouch.timestamp.mark();
                _ltouch.touchids.insert(event.touch);

                _joycenter = pos;
                //_joycenter.y += JSTICK_OFFSET;
            }
            break;
        case Zone::RIGHT:
            // For now if pressed on right side attempt pickup, need to change to button
            _touchDown = true;
            _touchStart = pos;

            currentStroke.clear();
            currentStroke.push_back(pos);
            _rtouch.touchids.insert(event.touch);
            break;
        case Zone::MAIN:
            //not possible hopefully lol
            break;
        default:
            break;
    }
    
}

/**
 * Callback for the end of a touch event
 *
 * @param event The associated event
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchEndCB(const TouchEvent& event, bool focus) {
    _touchDown = false;
    // Reset all keys that might have been set
    Vec2 pos = touch2Screen(event.position);
    Zone zone = getZone(pos);

    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        _joystick = false;
        _ltouch.touchids.clear();
        _move = Vec2::ZERO;
    } else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    } else if (zone == Zone::MAIN) {
    }
}

/**
 * Callback for a touch moved event.
 *
 * @param event The associated event
 * @param previous The previous position of the touch
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchMotionCB(const TouchEvent& event, const Vec2& previous, bool focus) {
    Vec2 pos = touch2Screen(event.position);
    // Left zone is all that matters rn. 
    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        processJoystick(pos);
    }
    else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        processDrag(pos);
    }
}

void InputController::buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    _touchDown = true;
    //CULog("Mouse press");

    Vec2 pos = mouse2Screen(event.position);
    Zone zone = getZone(pos);

    switch (zone) {
    case Zone::LEFT:
        // Left is the floating joystick
        _mouseDownZone = Zone::LEFT;

        _joycenter = pos;
        break;
    case Zone::RIGHT:
        // For now if pressed on right side attempt pickup, need to change to button
        _touchDown = true;
        _touchStart = pos;

        currentStroke.clear();
        currentStroke.push_back(pos);
        //_rtouch.touchids.insert(event.touch);
        _mouseDownZone = Zone::RIGHT;
        break;
    case Zone::MAIN:
        //not possible hopefully lol
        _mouseDownZone = Zone::MAIN;

        break;
    default:
        _mouseDownZone = Zone::UNDEFINED;
        break;
    }
}

void InputController::buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    _touchDown = false;
    // Reset all keys that might have been set
    Vec2 pos = mouse2Screen(event.position);
    Zone zone = getZone(pos);

    if (_mouseDownZone == Zone::LEFT) {
        //CULog("(%.2f, %.2f) (%.2f, %.2f)", _joycenter.x, _joycenter.y, pos.x, pos.y);
        _ltouch.touchids.clear();
        _move = Vec2::ZERO;
        _joystick = false;
    }
    else if (_mouseDownZone == Zone::RIGHT) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    }
    else if (zone == Zone::MAIN) {
    }
    _mouseDownZone = Zone::UNDEFINED;
}

void InputController::motionCB(const cugl::MouseEvent& event, const cugl::Vec2 previous, bool focus) {
    Vec2 pos = mouse2Screen(event.position);

    if (_mouseDownZone == Zone::LEFT) {
        processJoystick(pos);
    }
    else if (_mouseDownZone == Zone::RIGHT) {
        processDrag(pos);
    }
}


void InputController::activate() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        touch->addBeginListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
            this->touchBeganCB(event, focus);
        });
        touch->addEndListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
            this->touchEndCB(event, focus);
        });
        touch->addMotionListener(LISTENER_KEY, [this](const cugl::TouchEvent& event,const Vec2 previous, bool focus) {
            this->touchMotionCB(event,previous, focus);
        });
        _active = true;
    } else {
        _active = false;
    }
#else
    _active = Input::activate<Keyboard>();
    Mouse* mouse = Input::get<Mouse>();
    if (_active && mouse) {
        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _mouseKey = mouse->acquireKey();

        mouse->addPressListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonDownCB(event, clicks, focus);
        });
        mouse->addReleaseListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonUpCB(event, clicks, focus);
        });
        mouse->addDragListener(_mouseKey, [=, this](const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
            this->motionCB(event, previous, focus);
        });
        _active = true;

        CULog("Added listeners!");
    }
    else {
        _active = false;
    }
#endif
}

void InputController::deactivate() {
#ifndef CU_TOUCH_SCREEN
    Mouse* mouse = Input::get<Mouse>();
    mouse->removePressListener(LISTENER_KEY);
    mouse->removeReleaseListener(LISTENER_KEY);
    mouse->removeDragListener(LISTENER_KEY);
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    touch->removeBeginListener(LISTENER_KEY);
    touch->removeEndListener(LISTENER_KEY);
#endif
    _active = false;
}

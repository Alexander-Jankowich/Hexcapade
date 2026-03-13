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

/** This defines the joystick "deadzone" (how far we must move) */
#define JSTICK_DEADZONE  15
/** This defines the joystick radial size (for reseting the anchor) */
//#define JSTICK_RADIUS    25
/** How far we must swipe up for a jump gesture */
#define SWIPE_LENGTH    50
/** How fast a double click must be in milliseconds */
#define DOUBLE_CLICK    400

std::vector<cugl::Vec2> V_GESTURE = {
    {600.0f, 87.666656f}, {607.0f, 91.333336f}, {612.0f, 96.333321f},
    {618.666687f, 107.333336f}, {625.0f, 123.666656f}, {630.999939f, 142.333328f},
    {636.666687f, 162.666656f}, {642.333313f, 182.666672f}, {647.666687f, 203.0f},
    {653.666626f, 220.666641f}, {658.666626f, 235.666641f}, {663.333313f, 248.333328f},
    {667.999939f, 259.0f}, {672.333313f, 269.333313f}, {677.666687f, 280.333344f},
    {683.0f, 290.333344f}, {688.333313f, 300.333344f}, {693.333313f, 310.0f},
    {698.333313f, 319.0f}, {701.666687f, 325.666656f}, {703.333313f, 327.333344f},
    {703.333313f, 324.666656f}, {703.333313f, 321.0f}, {703.333313f, 318.333344f},
    {703.333313f, 315.0f}, {704.0f, 309.666656f}, {709.333313f, 292.333344f},
    {716.666687f, 259.0f}, {726.0f, 215.0f}, {738.333313f, 174.333328f},
    {747.333313f, 151.0f}, {752.0f, 144.333328f}, {760.0f, 133.333328f},
    {771.0f, 119.333336f}, {780.0f, 108.666656f}, {786.0f, 102.666656f},
    {789.333313f, 99.0f}, {791.0f, 96.333321f}, {792.333313f, 94.0f},
    {793.333313f, 90.666656f}, {794.666687f, 87.333321f}, {796.333313f, 84.0f},
    {798.666687f, 80.333328f}, {801.666687f, 77.333328f}, {802.333313f, 76.333328f},
    {802.333313f, 76.333328f}
};

std::vector<cugl::Vec2> STAR_GESTURE = {
    {582.666687, 296.666656},
    {581.000000, 290.000000},
    {581.000000, 284.666656},
    {583.666687, 273.000000},
    {594.333313, 241.333328},
    {611.333313, 195.666672},
    {627.000000, 157.000000},
    {637.999939, 130.333328},
    {641.666687, 120.333336},
    {642.333313, 118.333321},
    {644.333313, 118.000000},
    {648.000000, 118.000000},
    {660.333313, 118.000000},
    {678.000000, 123.333321},
    {697.000000, 142.999985},
    {716.000000, 173.000000},
    {732.000000, 201.666641},
    {746.000000, 223.333328},
    {757.000000, 240.666672},
    {764.333313, 254.666641},
    {768.000000, 264.333344},
    {768.000000, 269.666656},
    {765.000000, 271.000000},
    {757.000000, 271.000000},
    {749.000000, 267.000000},
    {738.333313, 258.666656},
    {716.666687, 240.666672},
    {675.666687, 211.666641},
    {633.333313, 192.333328},
    {600.333313, 183.333313},
    {576.666687, 181.000000},
    {561.666626, 180.333328},
    {553.000000, 179.333328},
    {546.666687, 178.666672},
    {542.999939, 178.333328},
    {541.666687, 176.666641},
    {541.666687, 174.333328},
    {548.333313, 172.333328},
    {564.333313, 168.666656},
    {602.333313, 155.000000},
    {650.333313, 146.000000},
    {695.333313, 142.999985},
    {734.000000, 142.999985},
    {759.000000, 142.999985},
    {769.000000, 142.999985},
    {771.333313, 143.666656},
    {771.333313, 145.666656},
    {771.000000, 148.000000},
    {757.000000, 154.666656},
    {709.000000, 180.000000},
    {678.000000, 203.333328},
    {657.333313, 221.000000},
    {637.999939, 240.333328},
    {621.333313, 258.000000},
    {603.000061, 279.000000},
    {583.000000, 302.666656},
    {570.000000, 317.000000},
    {566.000061, 320.333313},
    {564.666687, 320.333313},
    {564.666687, 320.333313},
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
_joyradius(100),
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
bool InputController::init(const Rect bounds) {
    recognizer = cugl::GestureRecognizer::alloc();
    recognizer->addGesture("V", V_GESTURE);
    recognizer->addGesture("STAR", STAR_GESTURE);
    recognizer->setSimilarityThreshold(0.6f);
    // Update the gesturemap with more spells later on
    _gesturemap = {
        {"V", SpellType::FIREBALL},
        {"STAR", SpellType::FREEZE}
    };

    _sbounds = bounds;
    Size display = Application::get()->getDisplaySize();
    Size drawable = Application::get()->getDrawableSize();
    if (drawable.width > 0 && drawable.height > 0) {
        _tbounds = Rect(0, 0, drawable.width, drawable.height);
    } else {
        _tbounds = Rect(0, 0, display.width, display.height);
    }
    
    CULog("tbounds x: %.2f, y: %.2f", _tbounds.getMaxX(), _tbounds.getMaxY());
    CULog("sbounds x: %.2f, y: %.2f", _sbounds.getMaxX(), _sbounds.getMaxY());

    
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);

    // The HUD touch bounds mirror the item/inventory overlay drawn in GameScene
    // so mobile taps hit the same rectangles the player sees on screen.
    float margin = 16.0f;
    float barW = 320.0f;
    float barH = 50.0f;
    float panelGap = 8.0f;
    float panelRowH = 36.0f;
    float panelRows = 4.0f;

    _actionButtonBounds = Rect(_sbounds.getMaxX() - barW - margin, margin, barW, barH);
    _inventoryHeaderBounds = Rect(margin, _sbounds.getMaxY() - barH - margin, barW, barH);
    _inventoryPanelBounds = Rect(
        _inventoryHeaderBounds.origin.x,
        _inventoryHeaderBounds.origin.y - panelGap - panelRowH * panelRows,
        _inventoryHeaderBounds.size.width,
        panelRowH * panelRows
    );
//#ifdef CU_TOUCH_SCREEN
//    Touchscreen* touch = Input::get<Touchscreen>();
//    if (touch) {
//        touch->addBeginListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
//            this->touchBeganCB(event, focus);
//        });
//        touch->addEndListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
//            this->touchEndCB(event, focus);
//        });
//        touch->addMotionListener(LISTENER_KEY, [this](const cugl::TouchEvent& event,const Vec2 previous, bool focus) {
//            this->touchMotionCB(event,previous, focus);
//        });
//        _active = true;
//    } else {
//        _active = false;
//    }
//#else
//    _active = Input::activate<Keyboard>();
//    Mouse* mouse = Input::get<Mouse>();
//    if (_active && mouse) {
//        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
//        _mouseKey = mouse->acquireKey();
//
//        mouse->addPressListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
//            this->buttonDownCB(event, clicks, focus);
//        });
//        mouse->addReleaseListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
//            this->buttonUpCB(event, clicks, focus);
//        });
//        mouse->addDragListener(_mouseKey, [=, this](const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
//            this->motionCB(event, previous, focus);
//        });
//        _active = true;
//
//        CULog("Added listeners!");
//    }
//    else {
//        _active = false;
//    }
//#endif
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
    if (_timeSinceCast > 1 && _isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        currentStroke.clear();
        CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
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
    float px = pos.x * _sbounds.getMaxX() / _tbounds.getMaxX();
    float py = pos.y * _sbounds.getMaxY() / _tbounds.getMaxY();

    Vec2 result;
    result.x = px;
    result.y = _sbounds.getMaxY() - py;
    return result;
//    return Display::get()->getTouchWindowPosition(pos.x, pos.y);
}

bool InputController::processMobileHUDTap(const Vec2 pos) {
    struct TapTarget {
        const char* tag;
        const Rect* bounds;
        bool active;
    };

    TapTarget targets[] = {
        { "context_action", &_actionButtonBounds, true },
        { "inventory_header", &_inventoryHeaderBounds, true },
        { "inventory_panel", &_inventoryPanelBounds, _inventoryPanelOpen }
    };

    for (int ii = 0; ii < 3; ++ii) {
        if (!targets[ii].active || !targets[ii].bounds->contains(pos)) {
            continue;
        }

        _lastUITap = pos;
        if (ii == 0) {
            _contextActionPressed = true;
        } else if (ii == 1) {
            _inventoryTogglePressed = true;
        } else {
            _inventoryPanelPressed = true;
        }

        CULog("HUD tap consumed target=%s pos=(%.1f, %.1f)", targets[ii].tag, pos.x, pos.y);
        return true;
    }

    return false;
}

Vec2 InputController::mouse2Screen(const Vec2 pos) const {
    float px = pos.x;
    float py = pos.y;

    Vec2 result;
    result.x = px;
    result.y = _sbounds.getMaxY() - py;
    return result;
//    return Display::get()->getMouseWindowPosition(pos.x, pos.y);
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

    if (dist > JSTICK_DEADZONE) {
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
    
    //_joycenter = touch2Screen(_ltouch.position);
    
    //_joycenter.y += JSTICK_OFFSET;
}

void InputController::processGestureEnd(const cugl::Vec2 pos) {
    if (_isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        _spellAngle = pos - _touchStart;
        CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
        return;
    }

    _spellAngle.setZero();
    currentStroke.push_back(pos);
    if (currentStroke.size() > 3) {
        std::string gesture = recognizer->match(currentStroke);

        try {
            _lastGesture = _gesturemap[gesture];
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
    //CULog("Stroke points:");
     //   for (const auto& p : currentStroke) {
     //       CULog("{%f, %f},", p.x, p.y);
     //   }

    // Clear points
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

    Vec2 pos = touch2Screen(event.position);
    if (processMobileHUDTap(pos)) {
        return;
    }

    Zone zone = getZone(pos);
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
    CULog("Mouse press");

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

//
//  GLInputController.h
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
#ifndef __GL_INPUT_CONTROLLER_H__
#define __GL_INPUT_CONTROLLER_H__

/**
 * Device-independent input manager.
 *
 * This class supports drag controls. This means pressing at a location,
 * dragging while pressed, and then releasing. However, the implementation
 * for this approach varies according to whether it is a mouse or mobile
 * touch controls. This interface hides the details.
 */
class InputController {
// Stylistically, I like common stuff protected, device specific private
    
public:
    /** Add more spells to this list later */
    enum class Gesture {
        NONE,
        FREEZE,
        FIREBALL
    };

    Gesture lastGesture;
protected:
    /** Whether the input device was successfully initialized */
    bool _active;
    /** The current touch/mouse position */
    cugl::Vec2 _currPos;
    /** The previous touch/mouse position */
    cugl::Vec2 _prevPos;
    /** Whether there is an active button/touch press */
    bool _currDown;
    /** Whether there was an active button/touch press last frame*/
    bool _prevDown;
   

protected:
	/** The key for the mouse listeners */
	Uint32 _mouseKey;
    /** The mouse position (for mice-based interfaces) */
    cugl::Vec2 _mousePos;
    /** Whether the (left) mouse button is down */
    bool _mouseDown;
    /** Whether a finger is down*/
    bool _touchDown;
    /** Where the finger touched*/
    cugl::Vec2 _touchPos;
    /**Current touchID**/
    cugl::TouchID _touchID;

    /**Are we going up*/
    float _up;
    /**Are we going down?*/
    float _down;
    /**Are we going to the right**/
    float _right;
    /**Are we going to the left**/
    float _left;
    /**Did we press the keyboard*/
    bool _pressed;
    
    /**is e held right now*/
    bool _pickupHeld;
    /**did e go down this frame*/
    bool _pickupPressed;
    
    
    
    /**Movement*/
    cugl::Vec2 move;
    /**A vector of points representing the gesture**/
    std::vector<cugl::Vec2> currentStroke;
    /**The gesture recognizer*/
    cugl::GestureRecognizer recognizer;
    
	// ADD NEW ATTRIBUTES HERE FOR EXTRA CREDIT
    
    /** Information representing a single "touch" (possibly multi-finger) */
    struct TouchInstance {
        /** The anchor touch position (on start) */
        cugl::Vec2 position;
        /** The current touch time */
        cugl::Timestamp timestamp;
        /** The touch id(s) for future reference */
        std::unordered_set<Uint64> touchids;
    };

    /** Enumeration identifying a zone for the current touch */
    enum class Zone {
        /** The touch was not inside the screen bounds */
        UNDEFINED,
        /** The touch was in the left zone (as shown above) */
        LEFT,
        /** The touch was in the right zone (as shown above) */
        RIGHT,
        /** The touch was in the main zone (as shown above) */
        MAIN
    };

    /** The bounds of the entire game screen (in touch coordinates) */
    cugl::Rect _tbounds;
    /** The bounds of the entire game screen (in scene coordinates) */
    cugl::Rect _sbounds;
    /** The bounds of the left touch zone */
    cugl::Rect _lzone;
    /** The bounds of the right touch zone */
    cugl::Rect _rzone;

    // Each zone can have only one touch
    /** The current touch location for the left zone */
    TouchInstance _ltouch;
    /** The current touch location for the right zone */
    TouchInstance _rtouch;
    /** The current touch location for the bottom zone */
    TouchInstance _mtouch;
    
    /** Whether the virtual joystick is active */
    bool _joystick;
    /** The position of the virtual joystick */
    cugl::Vec2 _joycenter;
    /** Whether or not we have processed a jump for this swipe yet */
    bool _hasJumped;
    /** The timestamp for a double tap on the right */
    cugl::Timestamp _rtime;
    /** The timestamp for a double tap in the middle */
    cugl::Timestamp _mtime;

    /** Mapping of gesture strings to the correct Gesture */
    std::unordered_map<std::string, Gesture> _gesturemap;

    /**
     * Defines the zone boundaries, so we can quickly categorize touches.
     */
    void createZones();
  
    /**
     * Populates the initial values of the TouchInstances
     */
    void clearTouchInstance(TouchInstance& touchInstance);

    /**
     * Returns the correct zone for the given position.
     *
     * See the comments above for a description of how zones work.
     *
     * @param  pos  a position in screen coordinates
     *
     * @return the correct zone for the given position.
     */
    Zone getZone(const cugl::Vec2 pos) const;
    
    /**
     * Returns the scene location of a touch
     *
     * Touch coordinates are inverted, with y origin in the top-left
     * corner. This method corrects for this and scales the screen
     * coordinates down on to the scene graph size.
     *
     * @return the scene location of a touch
     */
    cugl::Vec2 touch2Screen(const cugl::Vec2 pos) const;

    /**
     * Processes movement for the floating joystick.
     *
     * This will register movement as left or right (or neither).  It
     * will also move the joystick anchor if the touch position moves
     * too far.
     *
     * @param  pos  the current joystick position
     */
    void processJoystick(const cugl::Vec2 pos);
    
    
	
#pragma mark Input Control
public:
    /**
     * Creates a new input controller.
     *
     * This constructor DOES NOT attach any listeners, as we are not
     * ready to do so until the scene is created. You should call
     * the {@link #init} method to initialize the scene.
     */
    InputController();

    /**
     * Deletes this input controller, releasing all resources.
     */
    ~InputController() { dispose(); }
    
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
    bool init(const cugl::Rect bounds);
        
    /**
     * Disposes this input controller, deactivating all listeners.
     *
     * As the listeners are deactived, the user will not be able to
     * monitor input until the controller is reinitialized with the
     * {@link #init} method.
     */
    void dispose();
    
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
     */
    void update();
    
    void resolvedGesture() {lastGesture = Gesture::NONE;}

#pragma mark Attributes
    /**
     * Returns true if this control is active.
     *
     * An active control is one where all of the listeners are attached
     * and it is actively monitoring input. An input controller is only
     * active if {@link #init} is called, and if {@link #dispose} is not.
     *
     * @return true if this control is active.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the current mouse/touch position
     *
     * @return the current mouse/touch position
     */
    const cugl::Vec2& getPosition() const {
        return _currPos;
    }

    /**
     * Returns the previous mouse/touch position
     *
     * @return the previous mouse/touch position
     */
    const cugl::Vec2& getPrevious() const {
        return _prevPos;
    }
    
    /**
     * Return true if the user initiated a press this frame.
     *
     * A press means that the user is pressing (button/finger) this
     * animation frame, but was not pressing during the last frame.
     *
     * @return true if the user initiated a press this frame.
     */
    bool didPress() const {
        return (!_prevDown && _currDown) || _touchDown;
    }

    /**
     * Return true if the user initiated a release this frame.
     *
     * A release means that the user was pressing (button/finger) last
     * animation frame, but is not pressing during this frame.
     *
     * @return true if the user initiated a release this frame.
     */
    bool didRelease() const {
        return !_currDown && _prevDown;
    }

    /**
     * Return true if the user is actively pressing this frame.
     *
     * This method only checks that a press is active or ongoing.
     * It does not care when the press was initiated.
     *
     * @return true if the user is actively pressing this frame.
     */
    bool isDown() const {
        return _currDown;
    }
    
    bool getUp() const{
        return _up;
    }
    
    bool getDown() const{
        return _down;
    }
    
    bool getLeft() const{
        return _left;
    }
    
    bool getRight() const{
        return _right;
    }
    
    bool getPressed() const{
        return _pressed;
    }
    
    bool didPickupPress() const {
        return _pickupPressed;
    }
    
    cugl::Vec2 getMove() const{
        return move;
    }
    
    /** Returns normalized joystick vector in [-1,1] per axis (Vec2::ZERO if inactive) */
    cugl::Vec2 getJoystickVector() const;
    
    /**
     * Reads the input for this player and converts the result into game logic.
     *
     * This is an example of polling input.  Instead of registering a listener,
     * we ask the controller about its current state.  When the game is running,
     * it is typically best to poll input instead of using listeners.  Listeners
     * are more appropriate for menus and buttons (like the loading screen).
     */
    void readInput();

#pragma mark Mouse Callbacks
private:
    /**
     * Call back to execute when a mouse button is first pressed.
     *
     * This function will record a press only if the left button is pressed.
     *
     * @param event     The event with the mouse information
     * @param clicks    The number of clicks (for double clicking)
     * @param focus     Whether this device has focus (UNUSED)
     */
    void buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

    /**
     * Call back to execute when a mouse button is first released.
     *
     * This function will record a release for the left mouse button.
     *
     * @param event     The event with the mouse information
     * @param clicks    The number of clicks (for double clicking)
     * @param focus     Whether this device has focus (UNUSED)
     */
    void buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

    /**
     * Call back to execute when the mouse moves.
     *
     * This input controller sets the pointer awareness only to monitor a mouse
     * when it is dragged (moved with button down), not when it is moved. This
     * cuts down on spurious inputs. In addition, this method only pays attention
     * to drags initiated with the left mouse button.
     *
     * @param event     The event with the mouse information
     * @param previous  The previously reported mouse location
     * @param focus     Whether this device has focus (UNUSED)
     */
    void motionCB(const cugl::MouseEvent& event, const cugl::Vec2 previous, bool focus);
    
	// ADD THE TOUCH CALLBACKS FOR EXTRA CREDIT
    
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
    
    void touchEndCB(const cugl::TouchEvent& event, bool focus);
    
    void touchMotionCB(const cugl::TouchEvent& event, const cugl::Vec2& previous, bool focus);
    
    
};

#endif /* __GL_INPUT_CONTROLLER_H__ */


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
#include "../Models/SpellSet.h"

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
#pragma mark Input Control
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
    bool init(std::shared_ptr<cugl::JsonValue> values, const cugl::Rect bounds);

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
     * 
     * @param dt the time (in seconds) since this was last called
     */
    void update(float dt);

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
    * Returns the last gesture the user has done.
    * 
    * Should call {@link #resolvedGesture} once the gesture has been
    * resolved to clear the gesture.
    * 
    * @return Last gesture user has done
    */
    SpellType getGesture() {
        return _lastGesture;
    }

    /**
    * Call this function after resolving a gesture
    */
    void resolvedGesture() { _lastGesture = SpellType::NONE; }

    /**
    * Returns a vector containing the current stroke
    * @return current stroke
    */
    std::vector<cugl::Vec2> getCurStroke() { return currentStroke; }

    /**
    * Returns the user input for movement
    * @return movement input
    */
    cugl::Vec2 getMove() const {
        return _move;
    }

    /**
    * Returns whether the user has attempted to pickup an item
    * 
    * Should call {@link #resolvedPickup} once the pickup has been
    * resolved to clear this input.
    */
    bool pressedPickup() {
        return _pickupPressed;
    }

    /**
    * Call this function after resolving item pickup
    */
    void resolvedPickup() {
        _pickupPressed = false;
    }

    /**
    * Returns whether the player requested a quick-drop action this frame.
    *
    * Desktop uses this for the `Q` key and mobile uses inventory-panel taps to
    * choose a slot, which later resolve into a drop request.
    */
    bool pressedDrop() {
        return _dropPressed;
    }

    /** Clears the one-frame quick-drop input flag after the scene handles it. */
    void resolvedDrop() {
        _dropPressed = false;
    }

    /**
    * Returns whether the player requested a deposit action this frame.
    *
    * Desktop uses this for the `F` key and mobile routes it through the single
    * context button when the player is inside the deposit zone.
    */
    bool pressedDeposit() {
        return _depositPressed;
    }

    /** Clears the one-frame deposit input flag after the scene handles it. */
    void resolvedDeposit() {
        _depositPressed = false;
    }

    /**
    * Returns whether the mobile context button was tapped this frame.
    */
    bool pressedContextAction() {
        return _contextActionPressed;
    }

    /** Clears the one-frame mobile context button flag. */
    void resolvedContextAction() {
        _contextActionPressed = false;
    }

    /**
    * Returns whether the inventory header toggle was tapped this frame.
    */
    bool pressedInventoryToggle() {
        return _inventoryTogglePressed;
    }

    /** Clears the one-frame inventory toggle flag. */
    void resolvedInventoryToggle() {
        _inventoryTogglePressed = false;
    }

    /**
    * Returns whether a tap landed inside the expanded inventory panel.
    */
    bool pressedInventoryPanel() {
        return _inventoryPanelPressed;
    }

    /** Clears the one-frame inventory panel tap flag. */
    void resolvedInventoryPanel() {
        _inventoryPanelPressed = false;
    }

    /**
    * Returns the most recent UI tap location in scene coordinates.
    *
    * The GameScene uses this to convert panel taps into inventory slot indices.
    */
    cugl::Vec2 getLastUITap() const {
        return _lastUITap;
    }

    /**
    * Lets the GameScene tell the input controller whether the panel is open.
    *
    * This keeps HUD hit testing and panel visibility synchronized.
    */
    void setInventoryPanelOpen(bool open) {
        _inventoryPanelOpen = open;
    }

    /**
    * Returns whether the user attempted to reset the game
    * 
    * Should call {@link #resolvedReset} once the reset has been
    * resolved to clear this input.
    */
    bool pressedReset() {
        return _resetPressed;
    }

    /**
    * Call this function after resolving game reset
    */
    void resolvedReset() {
        _resetPressed = false;
    }

    /**
     * Reads the input for this player and converts the result into game logic.
     *
     * This is an example of polling input.  Instead of registering a listener,
     * we ask the controller about its current state.  When the game is running,
     * it is typically best to poll input instead of using listeners.  Listeners
     * are more appropriate for menus and buttons (like the loading screen).
     */
    void readInput();

#pragma mark Joystick Attributes

    /**
    * Returns the radius of the joystick
    * @return joystick radius
    */
    int getJoystickRadius() { return _joyradius; }

    /**
    * Returns whether the joystick is being used/should be shown
    * @return true if the joystick should be shown
    */
    bool showJoystick() { return _joystick; }

    /**
    * Returns the position of the center of the joystick
    * @return center of joystick position
    */
    cugl::Vec2 getJoystickCenter() { return _joycenter; }

    /**
    * Returns the position of the joystick
    * @return joystick position
    */
    cugl::Vec2 getJoystickPos() { return _joystickpos; }

    void clearSpell() { _lastGesture = SpellType::NONE; }

    bool shouldReleaseSpell() { return _releaseSpell; }

    void resolveSpell() { _releaseSpell = false; _lastGesture = SpellType::NONE; _isAiming = false; }

    bool showAim() { return _isAiming; }

    cugl::Vec2 getSpellAngle() { return _spellAngle; }

    /** Returns the bottom-right context button bounds in scene space. */
    const cugl::Rect& getActionButtonBounds() const { return _actionButtonBounds; }
    /** Returns the top-left inventory header bounds in scene space. */
    const cugl::Rect& getInventoryHeaderBounds() const { return _inventoryHeaderBounds; }
    /** Returns the expanded inventory panel bounds in scene space. */
    const cugl::Rect& getInventoryPanelBounds() const { return _inventoryPanelBounds; }

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

    /** The last gesture done */
    SpellType _lastGesture;

	/** The key for the mouse listeners */
	Uint32 _mouseKey;
    /** The mouse position (for mice-based interfaces) */
    cugl::Vec2 _mousePos;
    /** Whether the (left) mouse button is down */
    bool _mouseDown;
    /** Whether a finger is down*/
    bool _touchDown;
    /** Where the finger touch began*/
    cugl::Vec2 _touchStart;
    /**Current touchID**/
    cugl::TouchID _touchID;
    
    /** Did player attempt to pickup item */
    bool _pickupPressed;
    /** Did player request a quick-drop this frame. */
    bool _dropPressed;
    /** Did player request a deposit this frame. */
    bool _depositPressed;
    /** Did player tap the context-sensitive mobile action button. */
    bool _contextActionPressed;
    /** Did player tap the inventory header toggle. */
    bool _inventoryTogglePressed;
    /** Did player tap inside the expanded inventory panel. */
    bool _inventoryPanelPressed;
    /** Whether inventory panel hit-testing should be active. */
    bool _inventoryPanelOpen;
    /** Most recent UI tap position in scene coordinates. */
    cugl::Vec2 _lastUITap;

    /** Did player press reset */
    bool _resetPressed;
    
    /**Movement*/
    cugl::Vec2 _move;

    /**A vector of points representing the gesture**/
    std::vector<cugl::Vec2> currentStroke;

    /**The gesture recognizer*/
    std::shared_ptr<cugl::GestureRecognizer> recognizer;

    /** Mapping of gesture strings to the correct Gesture */
    std::unordered_map<std::string, SpellType> _gesturemap;

    bool _isAiming;
    float _timeSinceCast;
    bool _releaseSpell;
    cugl::Vec2 _spellAngle;
    /** Bottom-right mobile context button bounds shared with HUD rendering. */
    cugl::Rect _actionButtonBounds;
    /** Top-left inventory header bounds shared with HUD rendering. */
    cugl::Rect _inventoryHeaderBounds;
    /** Expanded inventory panel bounds shared with HUD rendering. */
    cugl::Rect _inventoryPanelBounds;
    
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
    /** The position of the virtual joystick center*/
    cugl::Vec2 _joycenter;
    /** The position of the virtual joystick */
    cugl::Vec2 _joystickpos;
    /** Radius of joystick */
    int _joyradius;
    int _deadzone;

    /** The timestamp for a double tap on the right */
    cugl::Timestamp _rtime;
    /** The timestamp for a double tap in the middle */
    cugl::Timestamp _mtime;


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
     * Returns the scene location of the mouse
     *
     * Touch coordinates are inverted, with y origin in the top-left
     * corner. This method corrects for this.
     *
     * @return the scene location of the mouse
     */
    cugl::Vec2 mouse2Screen(const cugl::Vec2 pos) const;

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

    /**
    * Processes a drag movement on the right side of the screen.
    * 
    * @param pos the position of the touch
    */
    void processDrag(const cugl::Vec2 pos);

    /**
     * Processes a gesture. Should be called when a touch ends.
     *
     * This will update \ref _lastGesture to the correct gesture that
     * was just done, or have no effect if the gesture is not recognized.
     *
     * @param pos the position of the touch end
     */
    void processGestureEnd(const cugl::Vec2 pos);

    /**
     * Consumes taps on mobile HUD controls before they reach joystick/gesture zones.
     *
     * @param pos  The tap position in scene coordinates.
     *
     * @return true if the HUD consumed this touch.
     */
    bool processMobileHUDTap(const cugl::Vec2 pos);

private:
#pragma mark Mouse Callbacks
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

    /** The zone the mouse drag started in */
    Zone _mouseDownZone = Zone::UNDEFINED;

#pragma mark Touch Callbacks
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
    
    void touchEndCB(const cugl::TouchEvent& event, bool focus);
    
    void touchMotionCB(const cugl::TouchEvent& event, const cugl::Vec2& previous, bool focus);
        
public:
    void deactivate();
    void activate();
    
};

#endif /* __GL_INPUT_CONTROLLER_H__ */

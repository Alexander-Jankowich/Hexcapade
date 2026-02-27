//
//  GLApp.cpp
//  Geometry Lab
//
//  This is the root class for your game. You need to use CU_ROOTCLASS in your
//  .cpp file to register this as the root with CUGL. Otherwise, it will not
//  know where to start. All root classes are subclasses of Application.
//
//  With that said, root classes often do not do all that much stuff. They
//  often delegate their work to individual scenes and just handle the scene
//  switching.
//
//  Author: Walker White
//  Version: 1/20/26
//
#include "GLApp.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;

// 16x9,  Android phones, PC Gaming
#define GAME_WIDTH  1280
#define GAME_HEIGHT 720

CU_ROOTCLASS(GeometryApp)

/**
 * Creates, but does not initialize, a new application.
 *
 * This constructor is where you set all your configuration values such
 * as the game name, the FPS, and so on. Many of these need to be set
 * before the backend is initialized.
 *
 * With that said, it is unsafe to do anything in this constuctor other than
 * initialize attributes. That is because this constructor is called before
 * the backend is initialized, and so much CUGL API calls will fail. Any
 * initialization that requires access to CUGL must happen in onStartup().
 */
GeometryApp::GeometryApp() : Application(), 
_loaded(false) {
    // Pre-launch configuration. Nothing here can be reassigned later.
    setName("Geometry Lab");
    setOrganization("GDIAC");
    setAppId("edu.cornell.gdiac.code2");
    
    setHighDPI(true);
    setResizable(false);
    setVSync(true);
    
    // This one can MAYBE reassigned after launch
    setDisplaySize(GAME_WIDTH, GAME_HEIGHT);
    
    // This one can be reset at any time
    setFPS(120.0f);
    
    // Get rid of the cornflower blue
    setClearColor(Color4(192,192,192));
}
    
/**
 * The method called after the backend is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place. You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void GeometryApp::onStartup() {
    _assets = AssetManager::alloc();
    _batch  = SpriteBatch::alloc();
    auto cam = OrthographicCamera::alloc(getDisplaySize());
    
#ifdef CU_MOBILE
    // Start-up basic input for loading screen (MOBILE ONLY)
    Input::activate<Touchscreen>();
#else
    // Start-up basic input for loading screen (DESKTOP ONLY)
    Input::activate<Mouse>();
    Input::activate<Keyboard>();
#endif
    
    // Bare bones asset loading (textures only)
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    // Needed for loading screen
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    _assets->loadDirectory("json/loading.json");

    // Create a "loading" screen
    _loaded = false;
    _loading.init(_assets,"json/assets.json");
    _loading.setSpriteBatch(_batch);
    
    // Queue up the other assets
    _loading.start();   
    Application::onStartup(); // YOU MUST END with call to parent
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line. This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void GeometryApp::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _endScene.dispose();
    _assets = nullptr;
    _batch = nullptr;

    // Shutdown input
    Input::deactivate<Mouse>();
    Input::deactivate<Keyboard>();

    Application::onShutdown();  // YOU MUST END with call to parent
}

/**
 * The method called when the application is suspended and put in the background.
 *
 * When this method is called, you should store any state that you do not
 * want to be lost.  There is no guarantee that an application will return
 * from the background; it may be terminated instead.
 *
 * If you are using audio, it is critical that you pause it on suspension.
 * Otherwise, the audio thread may persist while the application is in
 * the background.
 */
void GeometryApp::onSuspend() {
}

/**
 * The method called when the application resumes and put in the foreground.
 *
 * If you saved any state before going into the background, now is the time
 * to restore it. This guarantees that the application looks the same as
 * when it was suspended.
 *
 * If you are using audio, you should use this method to resume any audio
 * paused before app suspension.
 */
void GeometryApp::onResume() {
}

/**
 * The method called to update the application data.
 *
 * This is part of your core loop and should be replaced with your custom
 * implementation. This method should contain any code that is not a
 * graphics API call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param dt    The amount of time (in seconds) since the last frame
 */
void GeometryApp::update(float dt) {
    if (!_loaded && _loading.isActive()) {
        _loading.update(dt);
    }
    else if (!_loaded) {
        _loading.dispose(); // releases its mouse listeners first
        _gameplay.init(_assets);
        _gameplay.setSpriteBatch(_batch);
        _endScene.init(_assets);         // safe to register listeners now
        _endScene.setSpriteBatch(_batch);
        _loaded = true;
        setDeterministic(true);
    }
}

/**
 * The method called to indicate the start of a deterministic loop.
 *
 * This method is used instead of {@link #update} if {@link #setDeterministic}
 * is set to true. It marks the beginning of the core application loop,
 * which is concluded with a call to {@link #postUpdate}.
 *
 * This method should be used to process any events that happen early in
 * the application loop, such as user input or events created by the
 * {@link schedule} method. In particular, no new user input will be
 * recorded between the time this method is called and {@link #postUpdate}
 * is invoked.
 *
 * Note that the time passed as a parameter is the time measured from the
 * start of the previous frame to the start of the current frame. It is
 * measured before any input or callbacks are processed. It agrees with
 * the value sent to {@link #postUpdate} this animation frame.
 *
 * @param dt    The amount of time (in seconds) since the last frame
 */
void GeometryApp::preUpdate(float dt) {
    if (_endScene.isVisible()) {
        _endScene.update(dt);
        if (_endScene.didPressReplay()) {
            _endScene.hide();
            _gameplay.reset();
        }
        return;
    }

    _gameplay.preUpdate(dt);

    if (_gameplay.hasWon()) {
        _endScene.show(true);
    }
    else if (_gameplay.hasLost()) {
        _endScene.show(false);
    }
}

/**
 * The method called to provide a deterministic application loop.
 *
 * This method provides an application loop that runs at a guaranteed fixed
 * timestep. This method is (logically) invoked every {@link getFixedStep}
 * microseconds. By that we mean if the method {@link draw} is called at
 * time T, then this method is guaranteed to have been called exactly
 * floor(T/s) times this session, where s is the fixed time step.
 *
 * This method is always invoked in-between a call to {@link #preUpdate}
 * and {@link #postUpdate}. However, to guarantee determinism, it is
 * possible that this method is called multiple times between those two
 * calls. Depending on the value of {@link #getFixedStep}, it can also
 * (periodically) be called zero times, particularly if {@link #getFPS}
 * is much faster.
 *
 * As such, this method should only be used for portions of the application
 * that must be deterministic, such as the physics simulation. It should
 * not be used to process user input (as no user input is recorded between
 * {@link #preUpdate} and {@link #postUpdate}) or to animate models.
 *
 * @param step    The fixed timestep in microseconds
 */
void GeometryApp::fixedUpdate(Uint64 step) {
    _gameplay.fixedUpdate(step);
}

/**
 * The method called to indicate the end of a deterministic loop.
 *
 * This method is used instead of {@link #update} if {@link #setDeterministic}
 * is set to true. It marks the end of the core application loop, which was
 * begun with a call to {@link #preUpdate}.
 *
 * This method is the final portion of the update loop called before any
 * drawing occurs. As such, it should be used to implement any final
 * animation in response to the simulation provided by {@link #fixedUpdate}.
 * In particular, it should be used to interpolate any visual differences
 * between the the simulation timestep and the FPS.
 *
 * This method should not be used to process user input, as no new input
 * will have been recorded since {@link #preUpdate} was called.
 *
 * Note that the time passed as a parameter is the time measured from the
 * start of the previous frame to the start of the current frame. It is
 * measured before any input or callbacks are processed. It agrees with
 * the value sent to {@link #preUpdate} this animation frame.
 *
 * @param dt    The amount of time (in seconds) since the last frame
 */
void GeometryApp::postUpdate(float dt) {
	// Compute time to report to game scene version of postUpdate
    float time = getFixedRemainder()/1000000.0f;
    _gameplay.postUpdate(time);
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom
 * implementation. This method should contain graphics API calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void GeometryApp::draw() {
    if (!_loaded) {
        _loading.render();
    }
    else if (_endScene.isVisible()) {
        _endScene.render();
    }
    else {
        _gameplay.render();
    }
}

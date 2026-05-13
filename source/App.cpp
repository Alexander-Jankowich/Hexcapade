//
// App.cpp
// Root Class for the game contains all scene switching logic and updating of core gameplay loops
// Author: Serena Duncan
#include "App.h"


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
    _loaded(false), _scene(State::LOAD) {
    // Pre-launch configuration. Nothing here can be reassigned later.
    setName("Hexcapade");
    setOrganization("GDIAC");
    setAppId("edu.cornell.gdiac.code2");
    setHighDPI(true);
    setResizable(false);
    setVSync(true);

    // This one can MAYBE reassigned after launch
    setDisplaySize(GAME_WIDTH, GAME_HEIGHT);

    // This one can be reset at any time
    setFPS(60.0f);

    // Get rid of the cornflower blue
    setClearColor(Color4(50,25,70));
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
    _batch = SpriteBatch::alloc();
    _rooms = std::make_shared<std::vector<std::string>>();
    auto cam = OrthographicCamera::alloc(getDisplaySize());
    Input::activate<TextInput>();
    Input::activate<Keyboard>();

#ifdef CU_MOBILE
    // Start-up basic input for loading screen (MOBILE ONLY)
    Input::activate<Touchscreen>();

#else
    // Start-up basic input for loading screen (DESKTOP ONLY)
    Input::activate<Mouse>();

#endif

    // Bare bones asset loading (textures only)
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());
    // Needed for loading screen
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    _assets->attach<LevelLoader>(GenericLoader<LevelLoader>::alloc("level", 4)->getHook());
    _assets->attach<StaticLevelLoader>(GenericLoader<StaticLevelLoader>::alloc("tutorial", 5)->getHook());

    _assets->loadDirectory("json/loading.json");
    setFixedStep(_fixstep / 2);

    // Create a "loading" screen
    _loaded = false;
    _loading.init(_assets, "json/assets.json");
    _loading.setSpriteBatch(_batch);

    // Queue up the other assets TODO: make this a forloop lol
    _assets->loadAsync<LevelLoader>("room1","json/room1.json",nullptr);
    _assets->loadAsync<LevelLoader>("room2","json/room2.json",nullptr);
    _assets->loadAsync<LevelLoader>("room3","json/room3.json",nullptr);
    _assets->loadAsync<LevelLoader>("room4","json/room4.json",nullptr);
    _assets->loadAsync<LevelLoader>("room5","json/room5.json",nullptr);
    _assets->loadAsync<LevelLoader>("room6","json/room6.json",nullptr);
    _assets->loadAsync<LevelLoader>("room7","json/room7.json",nullptr);
    _assets->loadAsync<StaticLevelLoader>("tutorial", "json/tutorial.json", nullptr);


    _rooms->push_back("room1");
    _rooms->push_back("room2");
    _rooms->push_back("room3");
    _rooms->push_back("room4");
    _rooms->push_back("room5");
    _rooms->push_back("room6");
    _rooms->push_back("room7");


    _loading.start();

    cugl::netcode::NetworkLayer::start(cugl::netcode::NetworkLayer::Log::DEVELOPER);

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
    _mainmenu.dispose();
    _hostScene.dispose();
    _clientScene.dispose();
    _loading.dispose();
    _gameplay.dispose();
    _endScene.dispose();
    _selectScene.dispose();
    _tutorialScene.dispose();
    _assets = nullptr;
    _batch = nullptr;
    // Shutdown input
    Input::deactivate<Mouse>();
    Input::deactivate<Keyboard>();
    Input::deactivate<TextInput>();

    cugl::netcode::NetworkLayer::stop();

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
    // NOTE: In deterministic mode (after loading), this method is NOT called.
    // All scene updates are handled in preUpdate() instead.
    if (_scene == State::LOAD) updateLoadingScene(dt);
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
    // Handle non-game scenes here (update() is not called in deterministic mode)
    switch (_scene) {
    case State::LOAD:   updateLoadingScene(dt); return;
    case State::MENU:   updateMenuScene(dt);    return;
    case State::HOST:   updateHostScene(dt);    return;
    case State::CLIENT: updateClientScene(dt);  return;
    case State::SELECT: updateSelectScene(dt); return;
    case State::END: updateEndScene(dt); return;
    case State::TUTORIAL: updateTutorialScene(dt); return;
    default: break;
    }

    if(_gameplay.isActive()){
        _gameplay.preUpdate(dt);
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
    if (_scene==State::TUTORIAL) {
            _tutorialScene.fixedUpdate(step);
            return;
        }
    if (_scene != State::GAME) return;
    _gameplay.fixedUpdate(step);
    // need to disconnect
    if (_gameplay.getNetwork()->getStatus()==NetworkController::Status::NETERROR) {
        _gameplay.setActive(false);
        _gameplay.getNetwork()->disconnect();
        _gameplay.reset();
        _gameplay.release();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        return;
    }
    // endgame
    if (_gameplay.gameEnded()) {
        _endScene.setNetwork(_gameplay.getNetwork());
        CULog("app: displaying end game scene");
        _endScene.show(_gameplay.hasWon());
        _gameplay.reset();
        _gameplay.release();
        _endScene.setActive(true);
        _gameplay.setActive(false);
        _scene = State::END;

    }
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
    if (_scene != State::GAME) return;
    // Compute time to report to game scene version of postUpdate
    float time = getFixedRemainder() / 1000000.0f;
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
    switch (_scene) {
    case State::LOAD:
        _loading.render();
        break;
    case State::MENU:
        _mainmenu.render();
        break;
    case State::HOST:
        _hostScene.render();
        break;
    case State::CLIENT:
        _clientScene.render();
        break;
    case State::SELECT:
        _selectScene.render();
        break;
    case State::GAME:
        _gameplay.render();
        break;
    case State::END:
        _endScene.render();
        break;
    case State::TUTORIAL:
        _tutorialScene.render();
        break;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Scene Transitions
// ──────────────────────────────────────────────────────────────────────────────

void GeometryApp::updateLoadingScene(float dt) {
    if (_loading.isActive()) {
        _loading.update(dt);
        return;
    }

    // Loading finished — initialize all scenes once
    _loading.dispose();

    _mainmenu.init(_assets);
    _mainmenu.setSpriteBatch(_batch);
    
    _hostScene.init(_assets);
    _hostScene.setSpriteBatch(_batch);
    
    _clientScene.init(_assets);
    _clientScene.setSpriteBatch(_batch);
    
    _selectScene.init(_assets);
    _selectScene.setSpriteBatch(_batch);
    
    _gameplay.init(_assets, _rooms);
    _gameplay.setSpriteBatch(_batch);

    _endScene.init(_assets);
    _endScene.setSpriteBatch(_batch);
    
    _tutorialScene.init(_assets);
    _tutorialScene.setSpriteBatch(_batch);

    _loaded = true;
    setDeterministic(true);
    _scene = State::MENU;
    _mainmenu.setActive(true);
}

void GeometryApp::updateMenuScene(float dt) {
    _mainmenu.update(dt);

    switch (_mainmenu.getChoice()) {
    case MenuScene::Choice::HOST:
        //_mainmenu.resetChoice();
        _mainmenu.setActive(false);
        _hostScene.setActive(true);
        _scene = State::HOST;
        break;
    case MenuScene::Choice::JOIN:
        //_mainmenu.resetChoice();
        _mainmenu.setActive(false);
        _clientScene.setActive(true);
        _scene = State::CLIENT;
        break;
    case MenuScene::Choice::TUTORIAL:
        _mainmenu.setActive(false);
        _tutorialScene.setActive(true);
        _scene=State::TUTORIAL;
        break;
    case MenuScene::Choice::NONE:
        break;
    }
}

void GeometryApp::updateHostScene(float dt) {
    _hostScene.update(dt);
    if (_hostScene.didBackOut()) {
        _hostScene.setActive(false);
        _hostScene.reset();
        _hostScene.resetStatus();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        return;
    }
    switch (_hostScene.getStatus()) {
    case NetworkController::Status::NETERROR:
        //TODO: Display some sort of network popup?
        _hostScene.setActive(false);
        _hostScene.reset();
        _hostScene.resetStatus();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        break;
    case NetworkController::Status::CONNECTED:
        _gameplay.initNetwork(_hostScene.getNetwork()); 
        break;
    case NetworkController::Status::HANDSHAKE:
    case NetworkController::Status::READY:
  //      CULog("status host handshake/ready %d", _hostScene.getStatus());
        break;
    case NetworkController::Status::INGAME:
        _hostScene.setActive(false);
        // Transfer connection ownership
        _selectScene.obtainNetwork(_hostScene.getNetwork());
        _gameplay.initPhysics();
        //_hostScene.release();
        _selectScene.setActive(true);
        _scene = State::SELECT;

        break;
    default: // should not happen but whatever
        break;
    }
}

void GeometryApp::updateSelectScene(float dt) {
    _selectScene.update(dt);
    switch (_selectScene.getStatus()) {
        case SelectScene::SSStatus::SELECT:
            break;
        case SelectScene::SSStatus::HANDSHAKE:
            if (_selectScene.canInitMap()) {
                _gameplay.initMap();
            }
            break;
        case SelectScene::SSStatus::START:
            CULog("in start");
            _selectScene.setActive(false);
            _gameplay.spawnPlayers();
            //_selectScene.release();
            _gameplay.setActive(true); // alr has connection from host/client scene
            _scene = State::GAME;
            break;
    }
    // init gameplay w same network earlier, so j use it for checks lol
    if (_gameplay.getNetwork()->getStatus()==NetworkController::Status::NETERROR) {
        _selectScene.setActive(false);
        _gameplay.getNetwork()->disconnect();
        _gameplay.getNetwork()->resetStatus();
        //_selectScene.release();
        //_gameplay.release();
        
        _gameplay.reset();
        _mainmenu.setActive(true);
        _scene = State::MENU;
    }
}

void GeometryApp::updateClientScene(float dt) {
    _clientScene.update(dt);
    if (_clientScene.didBackOut()) {
        _clientScene.setActive(false);
        _clientScene.reset();
        _clientScene.resetStatus();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        return;
    }

    switch (_clientScene.getStatus()) {
    case NetworkController::Status::NETERROR:
        // Reinitialize so a fresh connection is made next time
        _clientScene.setActive(false);
        _clientScene.reset();
        _clientScene.resetStatus();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        break;
    case NetworkController::Status::CONNECTED:
        _gameplay.initNetwork(_clientScene.getNetwork());
        break;
    case NetworkController::Status::HANDSHAKE:
//            CULog("client entered handshake");
            break;
    case NetworkController::Status::READY :
            CULog("ready freddy");
            _gameplay.initPhysics();
            break;
    case NetworkController::Status::INGAME:
            _clientScene.setActive(false);
            // Transfer connection ownership
            _selectScene.obtainNetwork(_clientScene.getNetwork());
            //_clientScene.release();
            _selectScene.setActive(true);
            _scene = State::SELECT;
        break;
    default:
        break;
    }
}

void GeometryApp::updateGameScene(float dt) {
    // Placeholder — gameplay logic lives in preUpdate/fixedUpdate/postUpdate
}

void GeometryApp::updateEndScene(float dt) {
    _endScene.update(dt);
    // EXIT BUTTON
    if (_endScene.didPressExit()) {
        _endScene.setActive(false);
        _endScene.getNetwork()->disconnect();
        _endScene.getNetwork()->resetStatus();
       // _endScene.release();
        _mainmenu.setActive(true);
        _scene = State::MENU;
        return;
    }
    if (_endScene.didPressReplay()) {
        CULog("replay was pressed");
        _endScene.setActive(false);
        //_selectScene.obtainNetwork(_endScene.getNetwork()); // hand off network,
        //_gameplay.initNetwork(_endScene.getNetwork());
        //_endScene.release();
        _selectScene.setActive(true);
        _scene = State::SELECT;
        return;
            
    }
    
    if (_endScene.getNetwork()->getStatus()==NetworkController::Status::NETERROR) {
        _endScene.setActive(false);
        _endScene.getNetwork()->disconnect();
        _endScene.getNetwork()->resetStatus();
        //_endScene.reset();
        //_endScene.release();
        _mainmenu.setActive(true);
        _scene = State::MENU;
    }
    return;
}

void GeometryApp::updateTutorialScene(float dt) {
    _tutorialScene.preUpdate(dt);
    
    // check for if it was exited and switch state back to correct thing
    if(_tutorialScene.getEnded()){
        _tutorialScene.setActive(false);
        _mainmenu.setActive(true);
        _scene = State::MENU;
        _tutorialScene.setEnded(false);
    }
}

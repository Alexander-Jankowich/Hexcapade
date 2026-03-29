//
//  GLGameScene.h
//  Geometry Lab
//
//  This is the primary class file for running the game.  You should study this 
//  file for ideas on how to structure your primary game scene. This class 
//  illustrates how to use the geometry tools in CUGL, and how to combine
//  them with the physics engine.
//
//  Note that this time we do not have any additional model classes.
//
//  Author: Walker White
//  Version: 1/29/24
//
#ifndef __GL_GAME_SCENE_H__
#define __GL_GAME_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <cstddef> 
#include "../Controllers/GLInputController.h"
#include "../Controllers/NetworkController.h"
#include "../Controllers/RoomController.h"
#include "../Controllers/CollisionController.h"

#include "../Models/GameplayConfig.h"
#include "../Models/Map.h"
#include "../Models/Items.h"
#include "../Models/Players.h"
#include "../Models/Spells.h"
#include "../Models/Enemies.h"
#include "../Models/Events.h"

#include "../Models/GameplayConfigLoader.h"

#include "InputDisplay.h"

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public cugl::scene2::Scene2 {
protected:
    // CONTROLLERS are attached directly to the scene (no pointers)
    std::shared_ptr<NetworkController> _networkCtrl;
    bool _ishost;
    /** uuid to players most recent update*/
    //std::unordered_map<std::string, ClientInput> _clientInputs;
    /** The controller to manage the ship */
    InputController _input;
    std::shared_ptr<InputDisplay> _inputDisplay;
    std::shared_ptr<cugl::Random> _random;
    /** Reference to the physics root of the scene graph */
    std::shared_ptr<cugl::scene2::SceneNode> _worldnode;

    /** The Box2D world */
    std::shared_ptr<cugl::physics2::distrib::NetWorld> _world;

    /** Factories */
    std::shared_ptr<SpellFactory> _spellFact;
    Uint32 _spellFactId;
    std::shared_ptr<PlayerFactory> _playerFact;
    Uint32 _playerFactId;
    std::shared_ptr<EnemyFactory> _enemyFact;
    Uint32 _enemyFactId;
    std::shared_ptr<RectFactory> _rectFact;
    Uint32 _rectFactId;
    std::shared_ptr<ItemFactory> _itemFact;
    Uint32 _itemFactId;

    std::shared_ptr<PlayerObstacle> _player;
    
    std::shared_ptr<Map> _map;
    RoomController _roomController;
    RunBalanceConfig _runConfig;
    cugl::Vec2 _cameraBottomLeft;
    int _runRevision;
    int _appliedRunRevision;
    unsigned int _mapSeed;
    cugl::Size _mapSceneSize;
    float lastEnemySpawn;

    // PUT OTHER ATTRIBUTES HERE AS NECESSARY
    CollisionController _collisions;
    std::vector<std::pair<ItemObstacle*, std::shared_ptr<cugl::scene2::SceneNode>>> _itemNodes;
    /** GA: itemnet legacy scene-local inventory cache kept during the migration. */
    int _inventoryValue;
    /** The total value already deposited into the shared quota. */
    int _depositedValue;
    /** The value the player needs to reach to win */
    int _quota;
    /** Whether the player has won/lost - only valid when _gameEnded is true */
    bool _playerWon;
    /** Whether thegame has ended */
    bool _gameEnded;
    /**Timer for end of game**/
    float _timer = 3600;

    std::shared_ptr<cugl::AssetManager> _assets;
    /** Font for rendering HUD text */
    std::shared_ptr<cugl::graphics::Font> _font;

    /** The constants json */
    std::shared_ptr<cugl::JsonValue> _constants;
    /** The gestures json */
    std::shared_ptr<cugl::JsonValue> _gestures;

    bool input_down = false;
    /** this tracks whether the player is currently inside the deposit zone */
    bool _canDeposit;
    /** this stores the prototype origin/deposit interaction position */
    cugl::Vec2 _depositPosition;
    /** this stores the prototype origin/deposit interaction radius */
    float _depositRadius;
    /** this tracks local-only inventory panel visibility */
    bool _inventoryPanelOpen;
    /** id of item player is on, if they are on an item */
    int _currOverlappedItem = -1;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::scene2::Scene2(), _cameraBottomLeft(cugl::Vec2::ZERO), _runRevision(0), _appliedRunRevision(-1), _mapSeed(0), _mapSceneSize(cugl::Size::ZERO), _quota(100), _playerWon(false), _gameEnded(false), _inventoryValue(0), _depositedValue(0), _canDeposit(false), _depositPosition(cugl::Vec2::ZERO), _depositRadius(42.0f), _inventoryPanelOpen(false) {}

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    void reset();
    
    bool gameEnded() {return _gameEnded;}

    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    std::shared_ptr<NetworkController> getNetwork() {return _networkCtrl;}

    void addItem(int value);
    int getInventoryValue() const;
    int getDepositedValue() const { return _depositedValue; }
    int getQuota() const { return _quota; }
    bool hasWon() const { return _playerWon; }

    /** initializes the network and physics world */
    void initNetwork(std::shared_ptr<NetworkController> n);
    /** initialize map once network is populated with random seed for this round */
    void initMap();
    /**
     * Returns true if the player is host.
     *
     * We may need to have gameplay specific code for host.
     *
     * @return true if the player is host.
     */
    bool isHost() const { return _ishost; }




#pragma mark -
#pragma mark Gameplay Handling
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
    void preUpdate(float dt);

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
     * The time passed to this method is NOT the same as the one passed to
     * {@link #preUpdate}. It will always be exactly the same value.
     *
     * @param step    The fixed timestep in microseconds
     */
    void fixedUpdate(Uint64 step);

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
     * last call to {@link #fixedUpdate}. That is because this method is used
     * to interpolate object position for animation.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
     */
    void postUpdate(float remain);

    /**
     * Draws all this scene to its associated SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch. By overriding it, you can do custom drawing
     * in its place. The expectation is that you will use the associated
     * SpriteBatch for drawing, but this is not required.
     */
    void render() override;

    void setActive(bool value) override;
    
    void initHost(unsigned int randSeed);

private:
    void processComboEvent(const std::shared_ptr<CastSpellEvent>& e);
    void processEndEvent(const std::shared_ptr<EndGameEvent>& e);

    // this maps local HUD input into explicit pickup/drop/deposit requests
    void handleMobileHUDInput(bool& pickupRequested, bool& depositRequested, int& dropIndex);
    // this maps the last panel tap to an inventory row index for the upcoming item flow
    int getInventoryPanelIndexAt(const cugl::Vec2& tapPos) const;
    /**
     * on recieving a pickup event from player, checks if it is valid.
     */
    bool hostCheckItemPickupForPlayer(const std::string& pid);
    // this removes the world node for an item after the item obstacle is deleted
    void removeItemNode(const std::shared_ptr<ItemObstacle>& item);
    // host-only: this spawns physics-backed items at map spawn points
    void spawnItemsFromMap();
    // this loads map/item quota tuning from json without changing the networking path
    void loadGameplayConfig();
    // this rebuilds the current map from host-owned run metadata
    void rebuildMapForRun();
    // this keeps the world camera centered on the local player inside map bounds
    void updateCameraFollow();
    // this resolves the local player move against generated room/hall walls
    cugl::Vec2 resolvePlayerMapMove(const cugl::Vec2& startPos, const cugl::Vec2& velocity) const;
    // this returns the camera offset used to keep hud elements screen-anchored
    cugl::Vec2 getHUDOffset() const;

    // this draws the prototype deposit zone so desktop/mobile tests can target it
    void drawDepositZone();

    void renderHUD();

    /**
     * This method links a scene node to the obstacle.
     *
     * This method adds a listener so that the sceneNode will move along with the obstacle.
     */
    void linkSceneToObs(const std::shared_ptr<cugl::physics2::Obstacle>& obj,
        const std::shared_ptr<cugl::scene2::SceneNode>& node);

    /**
     * Adds the physics object to the physics world and loosely couples it to the scene graph
     *
     * There are two ways to link a physics object to a scene graph node on the
     * screen.  One way is to make a subclass of a physics object.
     * The other is to use callback functions to loosely couple
     * the two.  This function is an example of the latter.
     * the two.  This function is an example of the latter.
     *
     * param obj    The physics object to add
     * param node   The scene graph node to attach it to
     */
    void addInitObstacle(const std::shared_ptr<cugl::physics2::Obstacle>& obj,
        const std::shared_ptr<cugl::scene2::SceneNode>& node);
    
    void beginContact(b2Contact* contact);

    void beforeSolve(b2Contact* contact, const b2Manifold* oldManifold);
};

#endif // !__GL_GAME_SCENE_H__


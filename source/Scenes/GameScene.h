//  GameScene.h
//
//  This is the primary class file for running the game.  It contains all core gameplay loop logic as well as
// calls to the network controller for processing networked events.
// Authors: Serena Duncan, Alexander Jankowich, Kaz Tam, Frank Xie, Galiba Anjum
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__
#include <cugl/core/math/polygon/CUPathFactory.h> // Bug that this is not in <cugl/cugl.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <box2d/b2_draw.h>
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include <cugl/cugl.h>
#include <iomanip>
#include "../Controllers/InputController.h"
#include "../Controllers/NetworkController.h"
#include "../Controllers/RoomController.h"
#include "../Controllers/Pathfinder.h"

#include "../Models/Map/Map.h"
#include "../Models/Items/ItemFactory.h"
#include "../Models/Players/PlayerFactory.h"
#include "../Models/Spells/SpellFactory.h"
#include "../Models/Enemies/EnemyFactory.h"
#include "../Models/Events.h"

#include "../Models/Animations.h"

#include "InputDisplay.h"
#define ROUND_TIMER 10800
// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** The ratio between the physics world and the screen. (40 px = 1 meter, 1 tile is 1m x 1m or 40px x 40px) */
#define PHYSICS_SCALE 50

#define COMBO_RADIUS 4

#define ITEM_PICKUP_RADIUS 2f

using namespace cugl;
using namespace std;


/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public scene2::Scene2 {
protected:
    // CONTROLLERS are attached directly to the scene (no pointers)
    shared_ptr<NetworkController> _networkCtrl;
    /** uuid to players most recent update*/
    //unordered_map<string, ClientInput> _clientInputs;
    /** The controller to manage the ship */
    InputController _input;
    ItemTypeClass _itemInfo;
    shared_ptr<InputDisplay> _inputDisplay;
    shared_ptr<Random> _random;
    bool _addedLWalls;
    /** Reference to the physics root of the scene graph */
    shared_ptr<scene2::OrderedNode> _worldnode;
    
    /** Node for the currently open inventory card cente */
    shared_ptr<scene2::SceneNode> _inventoryCardNode;
    /** Node for the currently open tutorial page */
    shared_ptr<scene2::SceneNode> _tutorialNode;

    /** The Box2D world */
    shared_ptr<physics2::distrib::NetWorld> _world;

    /** Factories */
    shared_ptr<SpellFactory> _spellFact;
    Uint32 _spellFactId;
    shared_ptr<PlayerFactory> _playerFact;
    Uint32 _playerFactId;
    shared_ptr<EnemyFactory> _enemyFact;
    Uint32 _enemyFactId;
    shared_ptr<ItemFactory> _itemFact;
    Uint32 _itemFactId;

    shared_ptr<PlayerObstacle> _player;
    
    //Just if we have a weak spell, I am doing it this way
    bool _weakSpell = false;

    /** Action Timeline for animations */
    shared_ptr<Animator> _animations;
    
    shared_ptr<Map> _map;
    RoomController _roomController;
    Vec2 _cameraBottomLeft;
    float _cameraZoom;
    unsigned int _mapSeed;
    Size _mapSceneSize;
    float lastEnemySpawn;

    // PUT OTHER ATTRIBUTES HERE AS NECESSARY
    Pathfinder _pathfinder;

    vector<pair<shared_ptr<ItemObstacle>, shared_ptr<scene2::SceneNode>>> _itemNodes;
    /** GA: itemnet legacy scene-local inventory cache kept during the migration. */
    int _inventoryValue;
    /** The total value already deposited into the shared quota. */
    int _depositedValue;
    /** The value the player needs to reach to win */
    int _quota;
    /** Whether the player has won/lost - only valid when _gameEnded is true */
    bool _playerWon;
    /** Whether thegame has ended */
    bool  _gameEnded;
    /**Timer for end of game**/
    float _timer = 0;

    shared_ptr<AssetManager> _assets;
    /** list of rooms */
    shared_ptr<vector<string>> _rooms;
    /** Font for rendering HUD text */
    shared_ptr<graphics::Font> _font;
    shared_ptr<graphics::Font> _nameFont;
    shared_ptr<graphics::Font> _uiFont;


    /** The constants json */
    shared_ptr<JsonValue> _constants;
    /** The gestures json */
    shared_ptr<JsonValue> _gestures;

    /** A map of a pair of spells to their combination */
    map<pair<SpellType, SpellType>, SpellType> _spellCombos;

    /** this tracks whether the player is currently inside the deposit zone */
    bool _canDeposit;
    /** this stores the prototype origin/deposit interaction position */
    Vec2 _depositPosition;
    /** this stores the prototype origin/deposit interaction radius */
    float _depositRadius;
    /** id of item player is on, if they are on an item */
    int _currOverlappedItem = -1;
    int _pendingInventoryTapIndex = -1;
    Timestamp _lastInventoryTapTime;
    /** Which tutorial page is currently open, or 0 if closed */
    int _tutorialPage = 0;

    int _glowItemId = -1;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : scene2::Scene2(),
        _cameraBottomLeft(Vec2::ZERO),
        _cameraZoom(1.5f),
        _mapSeed(0),
        _mapSceneSize(Size::ZERO),
        _quota(100),
        _playerWon(false),
        _gameEnded(false),
        _inventoryValue(0),
        _depositedValue(0),
        _canDeposit(false),
        _depositPosition(Vec2::ZERO),
        _depositRadius(42.0f),
        lastEnemySpawn(0) {}


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
    void reset() override;
    
    bool gameEnded() { return _networkCtrl->gameState()==NetworkController::NCGameState::WON || _networkCtrl->gameState()==NetworkController::NCGameState::LOST; }

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
    bool init(const shared_ptr<AssetManager>& assets, const shared_ptr<vector<string>>& rooms, const std::shared_ptr<NetworkController>& network);
    
    shared_ptr<NetworkController> getNetwork() {return _networkCtrl;}
    void release() {_networkCtrl = nullptr;}

    int getDepositedValue() const { return _depositedValue; }
    int getQuota() const { return _quota; }
    bool hasWon() const { return _networkCtrl->gameState()==NetworkController::NCGameState::WON; }

    /** Initialize networked physics before map built */
    void initPhysics();
    /** initialize map once network is populated with random seed for this round */
    void initMap();
    void spawnPlayers();
    /**
     * Returns true if the player is host.
     *
     * We may need to have gameplay specific code for host.
     *
     * @return true if the player is host.
     */
    bool isHost() const { return _networkCtrl->isHost(); }




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
    
private:

#pragma mark Inventory Code
    // PLEASE MOVE THIS OUT OF MAIN.
    Rect getInventorySlotBounds(size_t slotIndex) const;
    Rect getHelpButtonBounds() const;
    // this maps the last hud tap to an inventory slot index for the upcoming item flow
    int getInventorySlotIndexAt(const Vec2& tapPos) const;
    void showInventoryCard(size_t slotIndex);
    void hideInventoryCard();
    void showTutorialPage(int page);
    void hideTutorial();
    /**
     * on recieving a pickup event from player, checks if it is valid.
     */
    bool hostCheckItemPickupForPlayer(const string& pid);
    // this removes the world node for an item after the item obstacle is deleted
    void removeItemNode(const shared_ptr<ItemObstacle>& item);
    // host-only: this spawns physics-backed items at map spawn points
    void spawnItemsFromMap();

#pragma mark Map things
    // this rebuilds the current map from host-owned run metadata
    void rebuildMapForRun();

#pragma mark Camera/HUD
    // this returns the orthographic zoom
    float getActiveCameraZoom() const;

    // this keeps the world camera centered on the local player inside map bounds
    void updateCameraFollow();

    // this returns the camera offset used to keep hud elements screen-anchored
    Vec2 getHUDOffset() const;

    // this draws the prototype deposit zone so desktop/mobile tests can target it
    void drawDepositZone();

    void renderHUD();

#pragma mark Physics functions
    /**
    * This method links a scene node to the obstacle.
    *
    * This method adds a listener so that the sceneNode will move along with the obstacle.
    */
    void linkSceneToObs(const shared_ptr<physics2::Obstacle>& obj,
        const shared_ptr<scene2::SceneNode>& node);
    
    void beginContact(b2Contact* contact);

    void beforeSolve(b2Contact* contact, const b2Manifold* oldManifold);

#pragma mark Spell helper functions
    /**
    * Adds all spell combos to the map.
    * Only adds one version of each combo. The getCombo function will check both ordering of the spells.
    */
    void initCombos();

    /**
    * Given the caster id and spell type, returns the combo spell type (SpellType::NONE
    * if no combos available) and the player id of the player that was combo-ed with
    */
    pair<SpellType, string> getCombo();
};

#endif // !__GAME_SCENE_H__

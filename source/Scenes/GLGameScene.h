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
#include "../Controllers/GLInputController.h"
#include "../Controllers/NetworkController.h"
#include "../Models/InputDisplay.h"
#include <cstddef> 
#include "../Models/EnemySet.h"
#include "../Models/SpellSet.h"
#include "../Models/PlayerSet.h"
#include "../Models/ItemSet.h"
#include "../Controllers/CollisionController.h"
#include "../Models/Player.h"

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
    std::unordered_map<std::string, ClientInput> _clientInputs;
    /** The controller to manage the ship */
    InputController _input;
    std::shared_ptr<InputDisplay> _inputDisplay;

    // MODELS should be shared pointers or a data structure of shared pointers
    /** The spline for the exterior circle */
    cugl::Spline2 _spline;
    /** The physics world to animate the falling */
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
    /** The outside "circle" */
    std::shared_ptr<cugl::physics2::PolygonObstacle> _center;
    /** The falling star */

    std::shared_ptr<Player> _player;
    
    std::shared_ptr<PlayerSet> _players;

    std::shared_ptr<EnemySet> _enemy;
    float lastEnemySpawn;

    // PUT OTHER ATTRIBUTES HERE AS NECESSARY
    CollisionController _collisions;
    /** GA: itemnet legacy scene-local inventory cache kept during the migration. */
    int _inventoryValue;
    /** The total value already deposited into the shared quota. */
    int _depositedValue;
    /** The value the player needs to reach to win */
    int _quota;
    /** Whether the player has won */
    bool _playerWon;
    /** Whether the player has lost */
    bool _playerLost;
    std::shared_ptr<cugl::AssetManager> _assets;
    /** Font for rendering HUD text */
    std::shared_ptr<cugl::graphics::Font> _font;
    cugl::Poly2 capsule;
    cugl::Path2 path;
    cugl::Poly2 circle;
    std::vector<cugl::Poly2> handles;
    std::vector<cugl::Poly2> knobs;
    cugl::Path2 star_path;
    cugl::Poly2 poly_star;
    bool input_down = false;

    SpellSet spells;

    /** Architecture-spec world-item owner for pickups, drops, and quota seeding. */
    ItemSet _itemSet;

    // GA: itemnet old scene-local item prototype kept for reference while
    // ItemSet becomes the gameplay source of truth on this branch.
#if 0
    struct ItemData {
        enum class ItemType {
            ARTIFACT,
            SPELL
        };

        ItemType type;
        std::string name;
        int value;
        cugl::Vec2 position;
        float radius;
        cugl::Color4 color;
    };

    /** this stores all items currently on the map */
    std::vector<ItemData> _items;
#endif
    /** this tracks if player is close enough to pickup an item */
    bool _canPickupItem;
    /** this stores the index for the current pickup target */
    int _pickupIndex;
    /** this stores the current prompt text for pickup */
    std::string _pickupPrompt;
    /** this tracks whether the player is currently inside the deposit zone */
    bool _canDeposit;
    /** this stores the prototype origin/deposit interaction position */
    cugl::Vec2 _depositPosition;
    /** this stores the prototype origin/deposit interaction radius */
    float _depositRadius;
    /** this tracks local-only inventory panel visibility */
    bool _inventoryPanelOpen;



    // Index of the currently selected tangent handle -1 if none
    size_t _selectedTangent = (size_t)-1;


    // Whether a control point is currently selected/being dragged can be inferred from _selectedTangent

    /**
     * Rebuilds the geometry.
     *
     * This method should recreate all the polygons for the spline, the handles
     * and the falling star. It should also recreate all physics objects.
     *
     * However, to cut down on performance overhead, this method should NOT add
     * those physics objects to the world inside this method (as this method is
     * called repeatedly while the user moves a handle). Instead, those objects
     * should not be activated until the state is "stable".
     */
    void buildGeometry();

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::scene2::Scene2(), _inventoryValue(0), _depositedValue(0), _quota(100), _playerWon(false), _playerLost(false), _canPickupItem(false), _pickupIndex(-1), _canDeposit(false), _depositPosition(cugl::Vec2::ZERO), _depositRadius(42.0f), _inventoryPanelOpen(false) {}

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

    void addItem(int value);
    int getInventoryValue() const;
    int getDepositedValue() const { return _depositedValue; }
    int getQuota() const { return _quota; }
    bool hasWon() const { return _playerWon; }
    bool hasLost() const { return _playerLost; }

    /**
     * Returns the network connection (as made by this scene)
     *
     * This value will be reset every time the scene is made active.
     *
     * @return the network connection (as made by this scene)
     */
    void setConnection(std::shared_ptr<NetworkController> network, bool isHost) {
        _networkCtrl = network;
        _ishost = isHost;
    }

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

    void setActive(bool value);

private:
    // this maps local HUD input into explicit pickup/drop/deposit requests
    void handleMobileHUDInput(bool& pickupRequested, bool& depositRequested, int& dropIndex);
    // this maps the last panel tap to an inventory row index for the upcoming item flow
    int getInventoryPanelIndexAt(const cugl::Vec2& tapPos) const;
    // this executes pickup against ItemSet and the local player's Inventory
    bool pickupNearbyItem();
    // this deposits all currently carried items into the shared quota
    bool depositAtDropoff();
    // this drops a specific inventory slot back into world space
    bool dropInventoryIndex(size_t index);
    // host-only helper that validates and applies a pickup for the given player
    bool hostPickupForPlayer(const std::shared_ptr<Player>& player);
    // host-only helper that validates and applies a drop for the given player
    bool hostDropInventoryIndex(const std::shared_ptr<Player>& player, size_t index);
    // host-only helper that validates and applies a deposit for the given player
    bool hostDepositForPlayer(const std::shared_ptr<Player>& player);

    // GA: itemnet old scene-local item prototype kept for reference while
    // ItemSet/Inventory replace the temporary vector-based path.
#if 0
    ItemData createRandomItem(bool forceArtifact, bool forceSpell);
    void spawnInitialItems();
    void spawnReplacementItem();
#endif
    // this updates nearest item + pickup prompt state
    void updateItemPickupState();
    // this updates whether the player can currently use the prototype deposit zone
    void updateDepositState();
    // this handles desktop e key pickup action
    void tryPickupItem();
    // this reserves the keyboard/mobile drop path for the inventory-backed item port
    void tryDropItem();
    // this reserves the keyboard/mobile deposit path for the inventory-backed item port
    void tryDepositItems();
    // this draws all active items
    void drawItems();
    // this draws the prototype deposit zone so desktop/mobile tests can target it
    void drawDepositZone();


    void renderHUD();
};

#endif // !__GL_GAME_SCENE_H__

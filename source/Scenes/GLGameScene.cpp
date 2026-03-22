//
//  GLGameScene.cpp
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
#include <cugl/cugl.h>
#include <cugl/core/math/polygon/CUPathFactory.h> // Bug that this is not in <cugl/cugl.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <random> //itemssss
#include <box2d/b2_draw.h>
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>

#include "GLGameScene.h"
#include "../Models/Artifact.h"
#include "../Models/GameplayConfigLoader.h"
#include "../Models/Inventory.h"
#include "../Models/SpellBook.h"

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** The ratio between the physics world and the screen. */
#define PHYSICS_SCALE 100

#define COMBO_RADIUS 25

#define PI 3.14159265358979323


using namespace cugl;
using namespace std;
using namespace cugl::graphics;

namespace {
    constexpr size_t kPrototypeInventoryCapacity = 4;
    constexpr int kPrototypeQuota = 500;
    constexpr int kGuaranteedArtifacts = 1;
    constexpr int kGuaranteedSpellbooks = 1;
    constexpr float kTargetWorldValueMultiplier = 1.35f;
    constexpr int kMaxAdditionalPrototypeItems = 40;
    constexpr float kEnemySpawnDelaySeconds = 15.0f;

//    unsigned int chooseNonZeroSeed() {
//        unsigned int seed = (unsigned int)std::random_device{}();
//        return seed == 0 ? 1u : seed;
//    }

    std::shared_ptr<Item> makePrototypeItem(const std::string& id, int kind,
                                            const std::string& name, int value) {
        if (kind == 0) {
            return std::make_shared<Artifact>(
                id,
                name,
                value,
                Artifact::Effect::ARTIFACT_EFFECT
            );
        }

        return std::make_shared<SpellBook>(
            id,
            name,
            value,
            SpellBook::Spell::SPELL_EFFECT
        );
    }

    // GA: itemnet old delta-path helper retained only as reference while the
    // first networking item pass uses full host snapshots instead.
    std::shared_ptr<Item> makePrototypeNetItem(const std::string& itemName, int itemId) {
        if (itemName == "artifact") {
            return makePrototypeItem(
                "net_artifact_" + std::to_string(itemId), 0, "artifact1", 60
            );
        }
        return makePrototypeItem(
            "net_spellbook_" + std::to_string(itemId), 1, "spellbook1", 25
        );
    }

    // item ids and texture stuff
    std::string getItemTextureKey(const std::shared_ptr<Item>& item) {
        if (!item) {
            return "";
        }

        const std::string& itemId = item->getId();
        if (itemId.rfind("artifact_scroll", 0) == 0) {
            return "artifact_scroll";
        }
        if (itemId.rfind("artifact_gem", 0) == 0) {
            return "artifact_gem";
        }
        if (itemId.rfind("spellbook_fire", 0) == 0) {
            return "spellbook_fire";
        }
        if (itemId.rfind("spellbook_freeze", 0) == 0) {
            return "spellbook_freeze";
        }
        if (itemId.rfind("spellbook_heal", 0) == 0) {
            return "spellbook_heal";
        }

        // keep one tiny fallback so older ids still show something readable.
        if (item->getKind() == Item::Kind::ARTIFACT) {
            return "artifact_scroll";
        }
        return "spellbook_fire";
    }
}

#pragma mark -
#pragma mark Level Layout


/**
 * Processes the start of a collision
 *
 * This method is called when we first get a collision between two objects.
 * We use this method to test if it is the "right" kind of collision.  In
 * particular, we use it to test if we make it to the win door.
 *
 * @param  contact  The two bodies that collided
 */
void GameScene::beginContact(b2Contact* contact){
    //CULog("made contact");
}

#pragma mark -
#pragma mark Constructors
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    // Initialize the scene to a locked width
    if (assets == nullptr)
    {
        return false;
    }
    else if (!cugl::scene2::Scene2::initWithHint(Size(0, SCENE_HEIGHT)))
    {
        return false;
    }

    _assets = assets;

    // Get constants
    _constants = assets->get<JsonValue>("constants");
    _gestures = assets->get<JsonValue>("gestures");
    _random = cugl::Random::alloc();

    // Start up the input handler
    // this keeps main's json-driven input setup while preserving the
    // item hud hooks already used on this branch
    _input.init(_constants->get("input"), cugl::Rect(0, 0, this->getSize().width, this->getSize().height));
    _input.setInventoryPanelOpen(false);
    _inputDisplay = make_shared<InputDisplay>(&_input, _constants->get("input"));

    lastEnemySpawn = 0;

    _map = std::make_shared<Map>();
    _map->init(); // initialize with tiled data
    _roomController.setMap(_map);
    _runRevision = 0;
    _appliedRunRevision = -1;
    _mapSeed = 0;
    _mapSceneSize = this->getSize();
    loadGameplayConfig();

    // Initialize inventory system
    _depositedValue = 0;
    _quota = std::max(1, _runConfig.quota);
    _playerWon = false;
    _playerLost = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    // this resets pickup prompt state
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";
    _canDeposit = false;
    _depositRadius = 42.0f;
    _depositPosition = Vec2::ZERO;
    _inventoryPanelOpen = false;

/*
    // ga: procnet old screen-space bootstrap kept for reference while the map
    // becomes the shared run layout source.
    _itemSet.reset();
    if (kGuaranteedArtifacts > 0 || kGuaranteedSpellbooks > 0) {
        _itemSet.spawnInitialPrototypeItems();
    }
    _itemSet.spawnUntilTotalValueAtLeast(
        (int)std::ceil((float)_quota * kTargetWorldValueMultiplier),
        kMaxAdditionalPrototypeItems
    );
*/

    _itemSet.reset();
    
    _mapSceneSize = this->getSize();
    //_mapSeed = chooseNonZeroSeed();
    _runRevision = 1;
    
    

    _spellFact = SpellFactory::alloc(_assets);
    _playerFact = PlayerFactory::alloc(_assets);
    _enemyFact = EnemyFactory::alloc(_assets);
    _rectFact = RectFactory::alloc(_assets, _map->_tiledRoomPrefab);
    CULog("did tiled thing load: %d", _map->_tiledRoomPrefab==nullptr);

    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    Size dimen = getSize();
    //_scale = dimen.width == SCENE_WIDTH ? dimen.width / rect.size.width : dimen.height / rect.size.height;
    //Vec2 offset((dimen.width - SCENE_WIDTH) / 2.0f, (dimen.height - SCENE_HEIGHT) / 2.0f);
    Vec2 offset = Vec2::ZERO;

    // Create the scene graph
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);
    

    addChild(_worldnode);

    _world = physics2::distrib::NetWorld::alloc(Rect(0, 0, dimen.width * 1000 , SCENE_HEIGHT * 1000), Vec2::ZERO);
    _world -> activateCollisionCallbacks(true);
    _world -> activateFilterCallbacks(false);
    _world -> activateDestructionCallbacks(true);
    //_world->onBeginContact = [this](b2Contact* contact) {
    //        beginContact(contact);
    //    };
    _world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact, oldManifold);
        };
    _world->update(1);
    
    
    
    _networkCtrl = NetworkController::alloc(_assets);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    // NOTHING TO DO THIS TIME
    _inputDisplay->dispose();
    _map = nullptr;
    _assets->dispose();
    _assets = nullptr;
}

void GameScene::reset()
{
    loadGameplayConfig();
    _playerLost = false;
    _playerWon = false;
    _depositedValue = 0;
    _quota = std::max(1, _runConfig.quota);
    // Match startup behavior on replay: reset clears active enemies and restarts
    // the host spawn timer instead of forcing one immediate spawn.
    lastEnemySpawn = 0;
    // TODO unbreak this
    // for (const std::shared_ptr<Player>& player : _players->current) {
    //     if (player->getInventory()) {
    //         player->getInventory()->clear();
    //     }
    // }
    _inventoryValue = 0;

    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt.clear();
    _canDeposit = false;
    _inventoryPanelOpen = false;
    _input.setInventoryPanelOpen(false);

    _mapSceneSize = this->getSize();
    _runRevision = std::max(1, _runRevision + 1);
    rebuildMapForRun();
    if (_ishost) {
        _networkCtrl->startGame();
    }
}

#pragma mark -
#pragma mark Inventory

void GameScene::addItem(int value) {
    // GA: itemnet old scene-local carry-value path kept only as a migration
    // bridge for any remaining call sites while Inventory becomes authoritative.
    _inventoryValue += value;
}

int GameScene::getInventoryValue() const {
    //if (!_player || !_player->getInventory()) {
    //    return _inventoryValue;
    //}
    //return _player->getInventory()->getCarriedValue();
    return 1;
}

void GameScene::loadGameplayConfig() {
    _runConfig = hexcapade::gameplayconfig::loadRunBalanceConfigFromAssets();
    _itemSet.setSpawnConfig(_runConfig.item);
}

void GameScene::rebuildMapForRun() {
    if (!_map) {
        _map = std::make_shared<Map>();
    }

    Size buildSize = _mapSceneSize;
    if (buildSize.width <= 0 || buildSize.height <= 0) {
        buildSize = this->getSize();
    }

    if (!_map->generate(buildSize, _runConfig.map, _mapSeed)) {
        CULog("map generation failed for run revision %d", _runRevision);
        return;
    }

    _roomController.setMap(_map);
    _appliedRunRevision = _runRevision;
    if(_networkCtrl -> isHost()){
        _map->populateObtacles(_networkCtrl, _rectFact, _rectFactId);
    }
    
    _quota = std::max(1, _runConfig.quota);
    _depositPosition = _map->getPlayerSpawn();

/*
    // ga: procnet old random item seeding kept as reference while room spawn
    // points are the current source for initial item placement.
    _itemSet.reset();
    _itemSet.setSpawnBounds(_map->getWorldBounds());
    if (kGuaranteedArtifacts > 0 || kGuaranteedSpellbooks > 0) {
        _itemSet.spawnInitialPrototypeItems();
    }
    _itemSet.spawnUntilTotalValueAtLeast(
        (int)std::ceil((float)_quota * kTargetWorldValueMultiplier),
        kMaxAdditionalPrototypeItems
    );
*/
    _itemSet.reset();
    _itemSet.setSpawnConfig(_runConfig.item);
    _itemSet.setSpawnBounds(_map->getWorldBounds());
    const std::vector<Vec2>& itemSpawnPoints = _roomController.getInitialItemSpawnPoints();
    _itemSet.spawnFromRoomPositions(itemSpawnPoints);
    _itemSet.spawnUntilTotalValueAtLeast(
        itemSpawnPoints,
        (int)std::ceil((float)_quota * kTargetWorldValueMultiplier),
        kMaxAdditionalPrototypeItems
    );

    //updateItemPickupState();
    //updateDepositState();
    if(_player){
        updateCameraFollow();
    }
    
}

void GameScene::updateCameraFollow() {
    std::shared_ptr<Camera> camera = this->getCamera();
    if (!camera) {
        _cameraBottomLeft = Vec2::ZERO;
        return;
    }

    if (!_map) {
        _cameraBottomLeft = Vec2::ZERO;
        Vec3 cameraPos = camera->getPosition();
        cameraPos.x = this->getSize().width * 0.5f;
        cameraPos.y = this->getSize().height * 0.5f;
        camera->setPosition(cameraPos);
        camera->update();
        return;
    }

    const Rect& world = _map->getWorldBounds();
    Size view = this->getSize();
    float halfW = view.width * 0.5f;
    float halfH = view.height * 0.5f;

    float minCenterX = world.origin.x + halfW;
    float maxCenterX = world.getMaxX() - halfW;
    float minCenterY = world.origin.y + halfH;
    float maxCenterY = world.getMaxY() - halfH;

    if (maxCenterX < minCenterX) {
        minCenterX = maxCenterX = world.getMidX();
    }
    if (maxCenterY < minCenterY) {
        minCenterY = maxCenterY = world.getMidY();
    }
    float cx = std::clamp(_player->getPosition().x * PHYSICS_SCALE, minCenterX, maxCenterX);
    float cy = std::clamp(_player->getPosition().y * PHYSICS_SCALE, minCenterY, maxCenterY);
    _cameraBottomLeft = Vec2(cx - halfW, cy - halfH);

    Vec3 cameraPos = camera->getPosition();
    
    cameraPos.x = cx;
    cameraPos.y = cy;
    camera->setPosition(cameraPos);
    camera->update();
}

Vec2 GameScene::resolvePlayerMapMove(const Vec2& startPos, const Vec2& velocity) const {
    if (!_map) {
        return startPos + velocity;
    }
    return _map->resolveSlideMove(startPos, velocity, 14.0f);
}

Vec2 GameScene::getHUDOffset() const {
    return _cameraBottomLeft;
}

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
void GameScene::preUpdate(float dt)
{
    _input.update(dt);
    
    if (_input.pressedReset()) {
        reset();
        _input.resolvedReset();
        return;
    }

    if (_playerLost || _playerWon) {
        return;
    }

    // Player movement
    cugl::Vec2 input = _input.getMove();
    cugl::Vec2 velocity = input * _player->getSpeed();
    _player->setVelocity(velocity);
    Vec2 pos = _player -> getPosition();
    //CULog("body active: %d", _player->isAwake());
    //CULog("actual vel: %f %f", _player->getLinearVelocity().x, _player->getLinearVelocity().y);
    _player->setAniState(_input.showAim() ? PlayerObstacle::AnimationState::CASTING : PlayerObstacle::AnimationState::IDLE);
    cugl::Vec2 playerScreenPos = _player->getPosition() * PHYSICS_SCALE;
    //CULog("Pos: %.2f %.2f", playerScreenPos.x, playerScreenPos.y);

    //updateItemPickupState();
    //updateDepositState();
    _input.setInventoryPanelOpen(_inventoryPanelOpen);

    bool pickupRequested = false;
    bool depositRequested = false;
    int dropIndex = -1;

    if (_input.pressedPickup()) {
        pickupRequested = true;
        _input.resolvedPickup();
    }
    //if (_input.pressedDrop()) {
    //    std::shared_ptr<Inventory> inventory = _player->getInventory();
     //   if (inventory && !inventory->isEmpty()) {
     //       dropIndex = (int)inventory->getCount() - 1;
     //   }
      //  _input.resolvedDrop();
   // }
    if (_input.pressedDeposit()) {
        depositRequested = true;
        _input.resolvedDeposit();
    }
    handleMobileHUDInput(pickupRequested, depositRequested, dropIndex);

    //bool hasNetwork = _networkCtrl && _networkCtrl->hasNetwork();
   // if (!hasNetwork || _ishost) {
//        if (pickupRequested) {
//            pickupNearbyItem();
//        }
//        if (dropIndex >= 0) {
//            dropInventoryIndex((size_t)dropIndex);
//        }
//        if (depositRequested) {
//            depositAtDropoff();
//        }
   // }

    // Spellcasting
    if (_input.getGesture() != SpellType::NONE) {
        _player->setSpell(_input.getGesture());
        //CULog("Set spell! %d", _player->getSpell());
    }

    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero()) {
        angle = _player->getDirection();
    }
    if (_input.shouldReleaseSpell()) {
        CULog("Release spell--------------------------------------------------------");
        // CULog("Pos: %.2f %.2f", playerScreenPos.x, playerScreenPos.y);
        //spells.spawnSpell(_player->getSpellId(), _player->getSpell(), playerScreenPos, angle);
        // Spawn spell, make host owner of object
        
        CULog("Spell type: %d", _player->getSpell());
        SpellType t = SpellType::NONE;

        // check for possible combos - serena implemented
        for (auto p : _playerFact->current) {
            if (_player->getPosition().distance(p->getPosition()) < COMBO_RADIUS && p->getSpell() != SpellType::NONE && p->getPID() != _player->getPID()) {
                // calc combo spell + send combo event
                if (_player->getSpell() == SpellType::FIREBALL) {
                    if (p->getSpell() == SpellType::FIREBALL) {
                        t = SpellType::COMBO_FIREBALL;
                    }
                }
                else if (_player->getSpell() == SpellType::FREEZE) {
                    if (p->getSpell() == SpellType::FREEZE) {
                        t = SpellType::COMBO_FREEZE;
                    }
                }

                // Spell is combo-ed!
                if (t != SpellType::NONE) {
                    _networkCtrl->pushOutEvent(CastSpellEvent::allocCastSpellEvent(_player->getPID(), p->getPID(), t));
                    break;
                }
            }
        }

        // Not a combo, use base spell
        if (t == SpellType::NONE) {
            t = _player->getSpell();
        }

        CULog("Casting spell type: %d", t);
        auto params = _spellFact->serializeParams(playerScreenPos/PHYSICS_SCALE, angle*2, t, _networkCtrl->getShortUID());
        auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
        _networkCtrl->getPhysController()->releaseObs(pair.first);

        _input.resolveSpell();
        _player->resolveSpell();
    }

    if (_ishost) {
        // spawn an enemy if host and need to spawn
        lastEnemySpawn += dt;
        if (lastEnemySpawn > 8) {
            lastEnemySpawn = 0;
            //_enemy->spawnEnemy(getSize() / 2, 4);
            
            
            auto spawns = _map -> getEnemySpawnPoints();
            std::uint64_t rand = _random -> getClosedUint64(0, spawns.size());
            if (!spawns.empty()) {
                std::uint64_t index = rand % spawns.size();
                Vec2 pos = spawns[index];

                auto params = _enemyFact->serializeParams(pos / PHYSICS_SCALE, Vec2::ZERO, 4);
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
            }
            
            
        }

        // Change to be all player positions
        std::vector<Vec2> player_pos;
        for (auto p : _playerFact->current) {
            player_pos.push_back(p->getPosition());
        }
        auto toRemove = _enemyFact->update(player_pos);
        for (auto en : toRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(en);
        }

        // Remove spells that have gone out of range
        auto spellRemove = _spellFact->update();
        for (auto spell : spellRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(spell);
            CULog("Remove spell: %d", spell->isRemoved());
        }
    }

    playerScreenPos = _player->getPosition();
    updateCameraFollow();
    _inputDisplay->setUIOffset(getHUDOffset());
    _inputDisplay->update(playerScreenPos);

    if (_player->getHealth() <= 0) {
        _playerLost = true;
    }
    if ((_playerLost || _playerWon) && _ishost) {
        _networkCtrl->endGame(_playerWon); // RN DOES NOTHING
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
 * possible that this method be called multiple times between those two
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
void GameScene::fixedUpdate(Uint64 step)
{
    // First process incoming events
    _networkCtrl->updateNet();
    
    // handle cast spell event
    if(_networkCtrl->isInAvailable()){
        auto e = _networkCtrl->popInEvent();
        if(auto cse = std::dynamic_pointer_cast<CastSpellEvent>(e)){
            CULog("COMBO SPELL GOT");
            processComboEvent(cse);
        }
//        else if(auto cse = std::dynamic_pointer_cast<GameInitEvent>(e)){
//            CULog("INIT EVENT GOT");
//            processInitEvent(cse);
//        }
    }

    _world->update(step / 1000000.0f);
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
 * last call to {@link #fixedUpdate}. That is because this method is used
 * to interpolate object position for animation.
 *
 * @param remain    The amount of time (in seconds) last fixedUpdate
 */
void GameScene::postUpdate(float remain)
{
}

/**
 * Draws all this scene to its associated SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch. By overriding it, you can do custom drawing
 * in its place. The expectation is that you will use the associated
 * SpriteBatch for drawing, but this is not required.
 */
void GameScene::render()
{
    

    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();
//    if (_map) {
//        _map->draw(_batch);
//    }
    drawDepositZone();
    drawItems();
    
    Color4 original = _batch->getColor();
    renderHUD();
    _inputDisplay->draw(_batch);
    _batch->setColor(original);
    Scene2::render();

}

void GameScene::renderHUD()
{
    Size screenSize = this->getSize();
    Vec2 hudOffset = getHUDOffset();

    float barW = 320;
    float barH = 50;
    float margin = 16;

    // Helper lambda to draw a solid colored box using Poly2
    auto drawBox = [&](float x, float y, float w, float h, Color4 color)
    {
        std::vector<Vec2> verts = {
            Vec2(x, y),
            Vec2(x + w, y),
            Vec2(x + w, y + h),
            Vec2(x, y + h)};
        std::vector<Uint32> idx = {0, 1, 2, 0, 2, 3};
        Poly2 box(verts, idx);
        _batch->setColor(color);
        _batch->fill(box);
    };

    _batch->setTexture(nullptr);

    Color4 boxColor(30, 30, 30, 255);

    Rect inventoryHeader = _input.getInventoryHeaderBounds();
    drawBox(inventoryHeader.origin.x + hudOffset.x, inventoryHeader.origin.y + hudOffset.y,
            inventoryHeader.size.width, inventoryHeader.size.height, boxColor);

    // Quota box - top right
    float quotaX = screenSize.width - barW - margin;
    float quotaY = screenSize.height - barH - margin;
    drawBox(quotaX + hudOffset.x, quotaY + hudOffset.y, barW, barH, boxColor);

    // Health box - top center
    float healthW = 200;
    float healthX = screenSize.width / 2 - healthW / 2;
    float healthY = screenSize.height - barH - margin;
    drawBox(healthX + hudOffset.x, healthY + hudOffset.y, healthW, barH, boxColor);

    Rect actionButton = _input.getActionButtonBounds();
    drawBox(actionButton.origin.x + hudOffset.x, actionButton.origin.y + hudOffset.y,
            actionButton.size.width, actionButton.size.height, boxColor);

    if (_font)
    {
        _batch->setColor(Color4::WHITE);

        //std::shared_ptr<Inventory> inventory = _player ? _player->getInventory() : nullptr;
        //size_t itemCount = inventory ? inventory->getCount() : 0;
        //size_t inventoryCapacity = inventory ? inventory->getCapacity() : kPrototypeInventoryCapacity;
        //std::shared_ptr<TextLayout> invLabel = TextLayout::allocWithText(
        //    "inventory: " + std::to_string(itemCount) + "/" + std::to_string(inventoryCapacity), _font);
        //invLabel->layout();
       // Affine2 invTrans;
        //invTrans.translate(inventoryHeader.origin.x + 10 + hudOffset.x, inventoryHeader.origin.y + 12 + //hudOffset.y);
       // _batch->drawText(invLabel, invTrans);

        std::shared_ptr<TextLayout> quotaLabel = TextLayout::allocWithText(
            "Quota: $" + std::to_string(_depositedValue) + " / $" + std::to_string(_quota), _font);
        quotaLabel->layout();
        Affine2 quotaTrans;
        quotaTrans.translate(quotaX + 10 + hudOffset.x, quotaY + 12 + hudOffset.y);
        _batch->drawText(quotaLabel, quotaTrans);

        std::shared_ptr<TextLayout> healthLabel = TextLayout::allocWithText(
            "Health: " + std::to_string(_player->getHealth()), _font);
        healthLabel->layout();
        Affine2 healthTrans;
        healthTrans.translate(healthX + 10 + hudOffset.x, healthY + 12 + hudOffset.y);
        _batch->drawText(healthLabel, healthTrans);

        std::string pickupText = "move close to item";
#ifdef CU_TOUCH_SCREEN
//        if (_canPickupItem) {
//            if (inventory && !inventory->canAddItem()) {
//                pickupText = "inventory full";
//            } else {
//                pickupText = "pickup item";
//            }
//        } else if (_canDeposit && inventory && !inventory->isEmpty()) {
//            pickupText = "deposit at origin";
//        }
#else
       // if (_canPickupItem) {
        //    pickupText = _pickupPrompt;
      //  } else if (_canDeposit && inventory && !inventory->isEmpty()) {
       //     pickupText = "press f to deposit carried items";
       // } else if (inventory && !inventory->isEmpty()) {
        //    pickupText = "e pickup  q drop  f deposit";
       // }
#endif
        std::shared_ptr<TextLayout> pickupLabel = TextLayout::allocWithText(pickupText, _font);
        pickupLabel->layout();
        Affine2 pickupTrans;
        pickupTrans.translate(actionButton.origin.x + 10 + hudOffset.x, actionButton.origin.y + 12 + hudOffset.y);
        _batch->drawText(pickupLabel, pickupTrans);

#ifdef CU_TOUCH_SCREEN
//        if (_inventoryPanelOpen && inventory) {
//            Rect panelBounds = _input.getInventoryPanelBounds();
//            drawBox(panelBounds.origin.x + hudOffset.x, panelBounds.origin.y + hudOffset.y,
//                    panelBounds.size.width, panelBounds.size.height,
//                    Color4(20, 20, 20, 230));
//
//            float rowH = panelBounds.size.height / 4.0f;
//            for (int ii = 0; ii < 4; ++ii) {
//                float rowY = panelBounds.getMaxY() - rowH * (ii + 1);
//                drawBox(panelBounds.origin.x + 2.0f + hudOffset.x, rowY + 1.0f + hudOffset.y,
//                        panelBounds.size.width - 4.0f, rowH - 2.0f,
//                        Color4(40, 40, 40, 220));
//
//                std::string rowText = "-";
//                if ((size_t)ii < inventory->getCount()) {
//                    rowText = inventory->getItems()[(size_t)ii]->getName();
//                }
//
//                std::shared_ptr<TextLayout> rowLabel = TextLayout::allocWithText(rowText, _font);
//                rowLabel->layout();
//                Affine2 rowTrans;
//                rowTrans.translate(panelBounds.origin.x + 10.0f + hudOffset.x, rowY + 8.0f + hudOffset.y);
//                _batch->drawText(rowLabel, rowTrans);
//            }
//        }
#endif // CU_TOUCH_SCREEN
    }
}

void GameScene::handleMobileHUDInput(bool& pickupRequested, bool& depositRequested, int& dropIndex) {
#ifdef CU_TOUCH_SCREEN
//    if (_input.pressedInventoryToggle()) {
//        _inventoryPanelOpen = !_inventoryPanelOpen;
//        _input.setInventoryPanelOpen(_inventoryPanelOpen);
//        _input.resolvedInventoryToggle();
//        CULog("inventory panel toggled: %d", _inventoryPanelOpen ? 1 : 0);
//    }
//
//    if (_input.pressedInventoryPanel()) {
//        if (_inventoryPanelOpen) {
//            int index = getInventoryPanelIndexAt(_input.getLastUITap());
//            if (index >= 0) {
//                dropIndex = index;
//            }
//        }
//        _input.resolvedInventoryPanel();
//    }
//
//    if (_input.pressedContextAction()) {
//        _input.resolvedContextAction();
//        if (_canPickupItem) {
//            pickupRequested = true;
//        } else if (_canDeposit) {
//            depositRequested = true;
//        }
//    }
#else
    if (_input.pressedInventoryToggle()) {
        _inventoryPanelOpen = !_inventoryPanelOpen;
        _input.resolvedInventoryToggle();
    }
#endif
}

//bool GameScene::pickupNearbyItem() {
   // return hostPickupForPlayer(_player);
//}

//bool GameScene::depositAtDropoff() {
   // return hostDepositForPlayer(_player);
//}

//bool GameScene::dropInventoryIndex(size_t index) {
   // return hostDropInventoryIndex(_player, index);
//}

//bool GameScene::hostPickupForPlayer(const std::shared_ptr<PlayerObstacle>& player) {
    // Host-authoritative item pickup. The host validates overlap and capacity
    // before moving an item from ItemSet into the player's Inventory.
    //if (!player) {
    //    return false;
    //}

    //int pickupIndex = _itemSet.findNearestPickupIndex(player->getPosition(), player->getSize());
    //if (pickupIndex < 0) {
    //    return false;
  //  }

   // std::shared_ptr<Inventory> inventory = player->getInventory();
    //if (!inventory || !inventory->canAddItem()) {
     //   if (player == _player) {
#ifdef CU_TOUCH_SCREEN
           // _pickupPrompt = "inventory full";
#else
        //    _pickupPrompt = "inventory full, press q to drop";
#endif
      //  }
      //  return false;
    //}

   // std::shared_ptr<Item> item = _itemSet.takeAt((size_t)pickupIndex);
   // if (!item) {
   //     return false;
   // }

   // if (!inventory->addItem(item)) {
   //     _itemSet.addDroppedItem(item, player->getPosition());
    ///    return false;
    //}

   // if (player == _player) {
   //     _inventoryValue = inventory->getCarriedValue();
    //    updateItemPickupState();
    //    updateDepositState();
   // }
   // return true;
//}

//bool GameScene::hostDropInventoryIndex(const std::shared_ptr<Player>& player, size_t index) {
//    // Host-authoritative drop. The host verifies slot ownership before putting
//    // the item back into ItemSet near the player.
//    if (!player) {
//        return false;
//    }
//
//    std::shared_ptr<Inventory> inventory = player->getInventory();
//    if (!inventory) {
//        return false;
//    }
//
//    std::shared_ptr<Item> dropped = inventory->dropItemAt(index);
//    if (!dropped) {
//        return false;
//    }
//
//    Vec2 dropDir = player->getDirection();
//    if (dropDir.isZero()) {
//        dropDir = Vec2(1, 0);
//    }
//
//    Vec2 dropPos = player->getPosition() + dropDir.getNormalization() * 26.0f;
//    _itemSet.addDroppedItem(dropped, dropPos);
//
//    if (player == _player) {
//        _inventoryValue = inventory->getCarriedValue();
//        updateItemPickupState();
//        updateDepositState();
//    }
//    return true;
//}
//
//bool GameScene::hostDepositForPlayer(const std::shared_ptr<Player>& player) {
//    // Host-authoritative deposit. The host checks deposit-zone overlap and then
//    // moves carried value into the shared team quota total.
//    if (!player) {
//        return false;
//    }
//
//    if (!_collisions.resolvePlayerItem(player, _depositPosition, _depositRadius)) {
//        return false;
//    }
//
//    std::shared_ptr<Inventory> inventory = player->getInventory();
//    if (!inventory || inventory->isEmpty()) {
//        return false;
//    }
//
//    _depositedValue += inventory->depositAllItems();
//    if (_depositedValue >= _quota) {
//        _playerWon = true;
//    }
//
//    if (player == _player) {
//        _inventoryValue = inventory->getCarriedValue();
//        updateDepositState();
//    }
//    return true;
//}
//
//void GameScene::applyInventorySnapshotToPlayer(const std::shared_ptr<Player>& player,
//                                               const std::vector<HostInventoryItemData>& items) {
//    if (!player) {
//        return;
//    }
//
//    if (!player->getInventory()) {
//        player->setInventory(std::make_shared<Inventory>(kPrototypeInventoryCapacity));
//    }
//
//    std::shared_ptr<Inventory> inventory = player->getInventory();
//    inventory->clear();
//    for (const HostInventoryItemData& item : items) {
//        inventory->addItem(makePrototypeItem(item.id, item.kind, item.name, item.value));
//    }
//}

//void GameScene::applyWorldItemSnapshot(const std::vector<HostWorldItemData>& items) {
//    _itemSet.reset();
///*
//    // ga: procnet old screen-space item bounds kept as reference during the
//    // move to map-owned world bounds.
//    _itemSet.setSpawnBounds(Rect(60, 80, this->getSize().width - 120, this->getSize().height - 170));
//*/
//    if (_map) {
//        _itemSet.setSpawnBounds(_map->getWorldBounds());
//    } else {
//        _itemSet.setSpawnBounds(Rect(60, 80, this->getSize().width - 120, this->getSize().height - 170));
//    }
//    for (const HostWorldItemData& item : items) {
//        _itemSet.addDroppedItem(
//            makePrototypeItem(item.id, item.kind, item.name, item.value),
//            Vec2(item.posX, item.posY),
//            item.radius
//        );
//    }
//}
//
//int GameScene::getInventoryPanelIndexAt(const Vec2& tapPos) const {
//    Rect panel = _input.getInventoryPanelBounds();
//    if (!panel.contains(tapPos)) {
//        return -1;
//    }
//
//    std::shared_ptr<Inventory> inventory = _player ? _player->getInventory() : nullptr;
//    if (!inventory || inventory->isEmpty()) {
//        return -1;
//    }
//
//    float rowH = panel.size.height / 4.0f;
//    float fromTop = panel.getMaxY() - tapPos.y;
//    int index = (int)(fromTop / rowH);
//    if (index < 0 || index >= (int)inventory->getCount()) {
//        return -1;
//    }
//    return index;
//}
//
//void GameScene::updateDepositState() {
//    _canDeposit = _collisions.resolvePlayerItem(_player, _depositPosition, _depositRadius);
//}
//
//void GameScene::tryDropItem() {
//    if (!_input.pressedDrop()) {
//        return;
//    }
//    _input.resolvedDrop();
//
//    std::shared_ptr<Inventory> inventory = _player->getInventory();
//    if (!inventory || inventory->isEmpty()) {
//        return;
//    }
//
//    dropInventoryIndex(inventory->getCount() - 1);
//}

//void GameScene::tryDepositItems() {
//    if (!_input.pressedDeposit()) {
//        return;
//    }
//    _input.resolvedDeposit();
//    depositAtDropoff();
//}

void GameScene::drawDepositZone() {
    if (!_map || _appliedRunRevision <= 0) {
        return;
    }

    _batch->setTexture(nullptr);

    Color4 color = _canDeposit ? Color4(70, 180, 95, 220) : Color4(60, 120, 70, 200);
    Rect zone(_depositPosition.x - _depositRadius, _depositPosition.y - _depositRadius,
              _depositRadius * 2.0f, _depositRadius * 2.0f);

    _batch->setColor(color);
    _batch->fill(zone);
}

// GA: itemnet old scene-local item prototype kept for reference while ItemSet
// and Inventory replace it as the gameplay source of truth.
/*
GameScene::ItemData GameScene::createRandomItem(bool forceArtifact, bool forceSpell) {
    ...
}

void GameScene::spawnInitialItems() {
    ...
}

void GameScene::spawnReplacementItem() {
    ...
}
*/

//void GameScene::updateItemPickupState()
//{
//    _canPickupItem = false;
//    _pickupIndex = -1;
//    _pickupPrompt = "";
//
//    _pickupIndex = _itemSet.findNearestPickupIndex(_player->getPosition(), _player->getSize());
//    if (_pickupIndex < 0) {
//        return;
//    }
//
//    const std::vector<ItemSet::WorldItem>& items = _itemSet.getItems();
//    if (_pickupIndex >= (int)items.size()) {
//        _pickupIndex = -1;
//        return;
//    }
//
//    _canPickupItem = true;
//    std::shared_ptr<Inventory> inventory = _player->getInventory();
//    if (inventory && !inventory->canAddItem()) {
//#ifdef CU_TOUCH_SCREEN
//        _pickupPrompt = "inventory full";
//#else
//        _pickupPrompt = "inventory full, press q to drop";
//#endif
//    } else {
//#ifdef CU_TOUCH_SCREEN
//        _pickupPrompt = "pickup item";
//#else
//        _pickupPrompt = "press e to pickup " + items[(size_t)_pickupIndex].item->getName();
//#endif
//    }
//}
//
//void GameScene::tryPickupItem() {
//    if (!_input.pressedPickup()) {
//        return;
//    }
//    _input.resolvedPickup();
//    pickupNearbyItem();
//}

void GameScene::drawItems()
{
    for (const ItemSet::WorldItem& world : _itemSet.getItems()) {
        Rect body(world.position.x - world.radius, world.position.y - world.radius,
                  world.radius * 2.0f, world.radius * 2.0f);

        std::string textureKey = getItemTextureKey(world.item);
        std::shared_ptr<Texture> itemTexture = textureKey.empty() ? nullptr : _assets->get<Texture>(textureKey);
        if (itemTexture) {
            // textured pickups are back. we draw them in the same world rect the
            // debug circles used before so networking/item positions stay unchanged.
            _batch->draw(itemTexture, Color4::WHITE, body);
            continue;
        }

        // keep the old debug fill as a last resort if a texture asset is missing.
        _batch->setTexture(nullptr);
        _batch->setColor(world.item->getDebugColor());
        _batch->fill(body);
    }
}

void GameScene::setActive(bool value)
{
    if (isActive() != value)
    {
        Scene2::setActive(value);
        if (value)
        {
            _input.activate();
            _mapSeed = _networkCtrl->getRandSeed();
            CULog("set map seed to : %d", _mapSeed);
            // init network and world once we start the game
            //Make a std::function reference of the linkSceneToObs function in game scene for network controller
            
            std::function<void(const std::shared_ptr<physics2::Obstacle>&, const std::shared_ptr<scene2::SceneNode>&)> linkSceneToObsFunc = [=, this](const std::shared_ptr<physics2::Obstacle>& obs, const std::shared_ptr<scene2::SceneNode>& node) {
                this->linkSceneToObs(obs, node);
                };
            
            _networkCtrl->enablePhysics(_world, linkSceneToObsFunc);
            _spellFactId = _networkCtrl->getPhysController()->attachFactory(_spellFact);
            _playerFactId = _networkCtrl->getPhysController()->attachFactory(_playerFact);
            _enemyFactId = _networkCtrl->getPhysController()->attachFactory(_enemyFact);
            _rectFactId = _networkCtrl->getPhysController()->attachFactory(_rectFact);
            
            if (_mapSeed != 0) {
                rebuildMapForRun();
            }
            
            auto params = _playerFact->serializeParams(_map -> getPlayerSpawn() / PHYSICS_SCALE, _networkCtrl->getPID());
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_playerFactId, params);

            // set _player to the correct object
            _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
            _player = _playerFact->getPlayerByPID(_networkCtrl->getPID());
            
            CULog("GameScene setActive: hasNetwork=%d isHost=%d",
                  _networkCtrl->hasNetwork(), _networkCtrl->isHost());
        }
        else
        {
            _input.deactivate();
        }
    }
}



void GameScene::addInitObstacle(const std::shared_ptr<physics2::Obstacle>& obj,
    const std::shared_ptr<scene2::SceneNode>& node) {
    _world->initObstacle(obj);
    if (_networkCtrl->isHost()) {
        _world->getOwnedObstacles().insert({ obj,0 });
    }
    linkSceneToObs(obj, node);
}

void GameScene::linkSceneToObs(const std::shared_ptr<physics2::Obstacle>& obj,
    const std::shared_ptr<scene2::SceneNode>& node) {

    node->setPosition(obj->getPosition() * 1);
    _worldnode->addChild(node);

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        
        // i do not know how else to check if it is a player...
        const shared_ptr<const PlayerObstacle> p = std::dynamic_pointer_cast<const PlayerObstacle>(obj);

        if (p != nullptr) {
            std::shared_ptr<Player> p_node = std::dynamic_pointer_cast<Player>(node);
            obj->setListener([=, this](physics2::Obstacle* obs) {
                PlayerObstacle* p = dynamic_cast<PlayerObstacle*>(obs);
                p_node->updateAnimationState(p->getState());
                float leftover = Application::get()->getFixedRemainder() / 1000000.f;
                Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
                float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
                p_node->setPosition(pos * PHYSICS_SCALE);
                p_node->setAngle(angle);
            });
            
        } else {
            obj->setListener([=, this](physics2::Obstacle* obs) {
                float leftover = Application::get()->getFixedRemainder() / 1000000.f;
                Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
                float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
                weak->setPosition(pos * PHYSICS_SCALE);
                weak->setAngle(angle);
            });
        }
    }
}

void GameScene::processComboEvent(const std::shared_ptr<CastSpellEvent>& e) {
    if (e->getP2() == _player->getPID()) {
        CULog("recieved spell combo event for our spell");
        _input.resolveSpell();
        _player->resolveSpell();
    }
}

//void GameScene::processInitEvent(const std::shared_ptr<GameInitEvent>& e) {
//    _mapSeed = e->getSeed();
//    rebuildMapForRun();
//}

/**
 * Handles any modifications necessary before collision resolution
 *
 * This method is called just before Box2D resolves a collision.  We use this method
 * to implement sound on contact, using the algorithms outlined in Ian Parberry's
 * "Introduction to Game Physics with Box2D".
 *
 * @param  contact  	The two bodies that collided
 * @param  oldManfold  	The collision manifold before contact
 */
void GameScene::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    if (!_ishost) return;
    
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();

    physics2::Obstacle* obs1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* obs2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    if (obs1->isRemoved() || obs2->isRemoved()) {
        return;
    }

    SpellObstacle* spell = nullptr;
    PlayerObstacle* player = nullptr;
    EnemyObstacle* enemy = nullptr;

    // Get types of the 2 obstacles; only care about collisions between different types of obstacles
    if (typeid(*obs1) == typeid(SpellObstacle)) {
        spell = reinterpret_cast<SpellObstacle*> (obs1);
    }
    else if (typeid(*obs1) == typeid(PlayerObstacle)) {
        player = reinterpret_cast<PlayerObstacle*> (obs1);
    }
    else if (typeid(*obs1) == typeid(EnemyObstacle)) {
        enemy = reinterpret_cast<EnemyObstacle*> (obs1);
    }

    if (typeid(*obs2) == typeid(SpellObstacle)) {
        spell = reinterpret_cast<SpellObstacle*> (obs2);
    }
    else if (typeid(*obs2) == typeid(PlayerObstacle)) {
        player = reinterpret_cast<PlayerObstacle*> (obs2);
    }
    else if (typeid(*obs2) == typeid(EnemyObstacle)) {
        enemy = reinterpret_cast<EnemyObstacle*> (obs2);
    }

    // Checking for collisions
    if (player && spell) {
        if (player->getPID()==_networkCtrl->getPID() && spell->getCasterUID()==_networkCtrl->getShortUID()) return;
        if (spell->getHealed()) {
            int nh = player->getHealth() + HEAL_AMT;
            player->setHealth(nh);
            spell->markRemoved(true);
        }
        return;
    }
    if (player && enemy) {
        CULog("enemy and player colliison");
        // TODO make this actually work?
        player->getBody()->ApplyLinearImpulseToCenter(b2Vec2(20 * enemy->getLinearVelocity().x, 20 * enemy->getLinearVelocity().y), true);
        enemy->getBody()->ApplyLinearImpulseToCenter(b2Vec2(-20 * enemy->getLinearVelocity().x, -20 * enemy->getLinearVelocity().y), true);
        // TODO - get enemy to pause after attack
       
        return;
    }
    if (enemy && spell) {
        enemy->setHealth(enemy->getHealth() - spell->getDamage());
        // TODO implement freezing
        spell->markRemoved(true);
        if (enemy->getHealth()==0) enemy->markRemoved(true);
        CULog("spell and enemy collision: %d", enemy->getHealth());
        return;
    }
}




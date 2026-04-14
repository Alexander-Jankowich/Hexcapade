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


// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** The ratio between the physics world and the screen. (40 px = 1 meter, 1 tile is 1m x 1m or 40px x 40px) */
#define PHYSICS_SCALE 40

#define COMBO_RADIUS 25

#define ITEM_PICKUP_RADIUS 0.5f


using namespace cugl;
using namespace std;
using namespace cugl::graphics;

namespace {
    constexpr size_t kPrototypeInventoryCapacity = 5;
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

//std::shared_ptr<Item> makePrototypeItem(const std::string& id, int kind,
//                                        const std::string& name, int value) {
//    if (kind == 0) {
//        return std::make_shared<Artifact>(
//                                          id,
//                                          name,
//                                          value,
//                                          Artifact::Effect::ARTIFACT_EFFECT
//                                          );
//    }
//    
//    return std::make_shared<SpellBook>(
//                                       id,
//                                       name,
//                                       value,
//                                       SpellBook::Spell::SPELL_EFFECT
//                                       );
//}


// item ids and texture stuff
//std::string getItemTextureKey(const std::shared_ptr<Item>& item) {
//    if (!item) {
//        return "";
//    }
//    
//    const std::string& itemId = item->getId();
//    if (itemId.rfind("artifact_scroll", 0) == 0) {
//        return "artifact_scroll";
//    }
//    if (itemId.rfind("artifact_gem", 0) == 0) {
//        return "artifact_gem";
//    }
//    if (itemId.rfind("spellbook_fire", 0) == 0) {
//        return "spellbook_fire";
//    }
//    if (itemId.rfind("spellbook_freeze", 0) == 0) {
//        return "spellbook_freeze";
//    }
//    if (itemId.rfind("spellbook_heal", 0) == 0) {
//        return "spellbook_heal";
//    }
//    
//    // keep one tiny fallback so older ids still show something readable.
//    if (item->getKind() == Item::Kind::ARTIFACT) {
//        return "artifact_scroll";
//    }
//    return "spellbook_fire";
//}

const ItemCatalogEntryConfig* chooseWeight( const std::vector<ItemCatalogEntryConfig>& entries, const std::shared_ptr<cugl::Random>& random) {
    
    float totalWeight = 0.0f;
    for (const auto& item : entries) {
        totalWeight += item.weight;
    }
    
    float roll = random->getClosedFloat(0.0f, totalWeight);
    
    for (const auto& item : entries) {
        if ((roll -= item.weight) <= 0.0f) {
            return &item;
        }
    }
    
    return &entries.back();
}

float getTypicalItemValue(const std::vector<ItemCatalogEntryConfig>& entries) {
    // Estimating the average item value from the weighted itemd so spawn count can scale with quota
    float totalWeight = 0.0f;
    float totalValue = 0.0f;
    for (const auto& item : entries) {
        totalWeight += item.weight;
        totalValue += item.weight * item.value;
    }

    return totalValue / totalWeight;
}

// Goal: enough item value to cover quota, but never spawn more than
// the authored spawn points we actually have on the map
size_t chooseSpawnCount(size_t totalSpawn, size_t roomCount, int quota,
                            float typicalItemValue, float quotaCoverageRatio) {
    int valueNeeded = (int)std::ceil((float)quota * quotaCoverageRatio);
    size_t itemsNeeded = (size_t)std::ceil((float)valueNeeded / typicalItemValue);
    return std::min(totalSpawn, std::max(roomCount, itemsNeeded));
}

std::string getCarriedItemName(const std::vector<ItemCatalogEntryConfig>& entries,
                               const PlayerObstacle::CarriedItem& item) {
    for (const auto& itemEntry : entries) {
        if (itemEntry.textureAsset == item.itemType || itemEntry.id == item.itemType) {
            return itemEntry.name;
        }
    }
    return item.itemType;
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets, const std::shared_ptr<std::vector<std::string>>& rooms)
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
    _rooms = rooms;

    // Get constants
    _constants = assets->get<JsonValue>("constants");
    _gestures = assets->get<JsonValue>("gestures");
    _random = cugl::Random::alloc();

    // Start up the input handler
    // This is the size of scene! We want to screenToWorld and worldToScreen
    // Factor out conversion with lambda gamescene -> input
    cugl::Size display = cugl::Application::get()->getDisplaySize();
    _input.init(
        _constants->get("input"),
        cugl::Rect(0, 0, display.width, display.height),
        [this](const cugl::Vec2& screenCoords) -> cugl::Vec2 {
            cugl::Vec3 world = screenToWorldCoords(screenCoords);
            return cugl::Vec2(world.x, world.y);
        }
    );
    _inputDisplay = make_shared<InputDisplay>(&_input, _constants->get("input"));

    lastEnemySpawn = 0;

    _map = std::make_shared<Map>();
    _roomController.setMap(_map);
    _runRevision = 0;
    _appliedRunRevision = -1;
    _mapSeed = 0;
    loadGameplayConfig();

    // Initialize inventory system
    _depositedValue = 0;
    _quota = std::max(1, _runConfig.quota);
    _playerWon = false;
    _gameEnded = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    // this resets pickup prompt state
    _canDeposit = false;
    _depositRadius = 42.0f;
    _depositPosition = Vec2::ZERO;
    
    //_mapSeed = chooseNonZeroSeed();
    _runRevision = 1;
    
    // Create all the factories
    _spellFact = SpellFactory::alloc(_assets);
    _playerFact = PlayerFactory::alloc(_assets);
    _enemyFact = EnemyFactory::alloc(_assets);
    _itemFact = ItemFactory::alloc(_assets);

    // Create other controllers
    _pathfinder = Pathfinder();

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
    
    _networkCtrl = nullptr;
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
    CULog("reset game");
    loadGameplayConfig();
    _gameEnded = false;
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
    for (auto& playerObs : _playerFact->current) {
        if (playerObs) {
            playerObs->clearInventory();
        }
    }
    // clear any tracked item visuals so the next run can relink fresh world items
    _itemNodes.clear();
    _inventoryValue = 0;
    _canDeposit = false;

    _runRevision = std::max(1, _runRevision + 1);
    rebuildMapForRun();
    if (_networkCtrl->isHost()) {
        _networkCtrl->startGame();
    }
}

#pragma mark -
#pragma mark Inventory

void GameScene::removeItemNode(const std::shared_ptr<ItemObstacle>& item) {
    if (!item) {
        return;
    }

    for (auto it = _itemNodes.begin(); it != _itemNodes.end(); ++it) {
        if (it->first != item.get()) {
            continue;
        }

        if (it->second) {
            it->second->removeFromParent();
        }
        _itemNodes.erase(it);
        return;
    }
}

void GameScene::loadGameplayConfig() {
    _runConfig = hexcapade::gameplayconfig::loadRunBalanceConfigFromAssets();
}

void GameScene::rebuildMapForRun() {
    if (!_map) {
        _map = std::make_shared<Map>();
    }

    if (!_map->generate(_runConfig.map, _mapSeed, _assets, _rooms)) {
        CULog("map generation failed for run revision %d", _runRevision);
        return;
    }
    
    auto n = _map -> generate(_runConfig.map, _mapSeed, _assets, _rooms);
    n->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    n->setPosition(Vec2::ZERO);
    _worldnode -> addChild(n);
    
    
    _roomController.setMap(_map);
    _appliedRunRevision = _runRevision;
    
    _quota = std::max(1, _runConfig.quota);
    _depositPosition = _map->getPlayerSpawn();

    if (_networkCtrl && _networkCtrl->isHost()) {
        spawnItemsFromMap();
    }
    //updateItemPickupState();
    //updateDepositState();
    if(_player){
        updateCameraFollow();
    }
}

void GameScene::spawnItemsFromMap() {
    const std::vector<Vec2>& spawnPoints = _map->getItemSpawnPoints();
    if (spawnPoints.empty()) {
        CULog("ItemFactory: no item spawn points found on map");
        return;
    }

    const std::vector<ItemCatalogEntryConfig>& entries = _runConfig.item.entries;

    int roomCount = _map->getNumRooms();
    float typicalItemValue = getTypicalItemValue(entries);
    size_t spawnCount = chooseSpawnCount(
        spawnPoints.size(), roomCount, _quota,
        typicalItemValue, _runConfig.item.quotaCoverageRatio
    );

    // Start with every spawn point, then randomly* remove from this
    // list as we choose which points to actually use for run
    std::vector<size_t> spawnIndices(spawnPoints.size());
    for (size_t spawnIndex = 0; spawnIndex < spawnPoints.size(); ++spawnIndex) {
        spawnIndices[spawnIndex] = spawnIndex;
    }

    for (size_t spawned = 0; spawned < spawnCount && !spawnIndices.empty(); ++spawned) {
        size_t pickIndex = 0;
        if (spawnIndices.size() > 1) {
            pickIndex = (size_t)(_random->getUint32() % spawnIndices.size());
        }

        const Vec2& worldPt = spawnPoints[spawnIndices[pickIndex]];
        spawnIndices[pickIndex] = spawnIndices.back();
        spawnIndices.pop_back();

        Vec2 physPos = worldPt / PHYSICS_SCALE;
        const ItemCatalogEntryConfig* item = chooseWeight(entries, _random);
        int itemValue = item->value;
        std::string itemType = item->textureAsset.empty() ? item->id : item->textureAsset;

        auto params = _itemFact->serializeParams(physPos, itemValue, itemType);
        _networkCtrl->getPhysController()->addSharedObstacle(_itemFactId, params);
    }
    CULog("ItemFactory: spawned %zu items", spawnPoints.size());
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

    const Rect& world = Rect(0, 0, MAP_WIDTH, MAP_HEIGHT); //TODO: is this right??
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
    if (!_player) {
        // Try to find our player now that replication may have run
        _player = _playerFact->getPlayerByPID(_networkCtrl->getPID());
        //Have we set our player yet / received signal
        if (!_player) return;
    }
    //We have to acquire ourselves before we attempt to move
    //it is a little weird to do this every frame but this allows host to acquire themselves
    _networkCtrl -> getPhysController() -> acquireObs(_player, 0);

    if (_gameEnded) {
        return;
    }
    
    if(_timer > 0){
        _timer -= 1;
    }
    if (_timer==0) {
        _gameEnded=true;
        bool won = _quota <= _depositedValue;
        _playerWon = won;
        _networkCtrl->pushOutEvent(EndGameEvent::allocEndGameEvent(won));
        return;
    }
    
    _input.update(dt);
    
    if (_input.pressedReset()) {
        reset();
        _input.resolvedReset();
        return;
    }
    
    if (_networkCtrl->isHost()) {
        // spawn an enemy if host and need to spawn
        lastEnemySpawn += dt;
        if (lastEnemySpawn > 8) {
            lastEnemySpawn = 0;

            auto spawns = _map -> getEnemySpawnPoints();
            std::uint64_t rand = _random -> getClosedUint64(0, spawns.size());
            if (!spawns.empty()) {
                std::uint64_t index = rand % spawns.size();
                Vec2 pos = spawns[index];

                auto params = _enemyFact->serializeParams(EnemyType::GHOST, pos / PHYSICS_SCALE);
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
                //If we create it we MUST acquire it
                // Kaz: shouldn't need to, when obstacles are created they automatically get assigned to the host
                _networkCtrl -> getPhysController() -> acquireObs(pair.first, 0);
            }
        }

        auto toRemove = _enemyFact->getRemoved();
        for (auto en : toRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(en);
        }

        // Update enemy movement
        _pathfinder.updateEnemies(_enemyFact, _playerFact);
        
        auto itRemove = _itemFact->update();
        for (auto item : itRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(item);
        }

        // Remove spells that have gone out of range
        auto spellRemove = _spellFact->update();
        for (auto spell : spellRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(spell);
            //CULog("Remove spell: %d", spell->isRemoved());
        }
        
    }
    
    if (_player->getHealth()==0) return; // if dead ignore updates

    // Player movement
    cugl::Vec2 input = _input.getMove();
    cugl::Vec2 velocity = input * _player->getSpeed();
    _player->setVelocity(velocity);
    
    _player->setAniState(_input.showAim() ? PlayerObstacle::AnimationState::CASTING : PlayerObstacle::AnimationState::IDLE);
    cugl::Vec2 playerScreenPos = _player->getPosition() * PHYSICS_SCALE;

    updateCameraFollow();
    Vec2 offset = getHUDOffset();
    _input.setCameraOffset(offset);
    _inputDisplay->setUIOffset(offset);
    _inputDisplay->update(playerScreenPos);


    bool depositRequested = false;
    int depositIndex = -1;
    _canDeposit = playerScreenPos.distance(_depositPosition) <= _depositRadius;

    if (_input.pressedDeposit()) {
        depositRequested = true;
        _input.resolvedDeposit();
    }
    if (_input.pressedInventory()) {
        depositIndex = getInventorySlotIndexAt(_input.getInventoryTap());
        _input.resolvedInventory();
    }
    if (_input.getInventoryHotkeyIndex() >= 0) {
        depositIndex = _input.getInventoryHotkeyIndex();
        _input.resolvedInventoryHotkeyIndex();
    }

    // MOBILE DEPOSIT: TAP FOR ONE SLOT
    if (depositIndex >= 0 && _canDeposit) {
        if (_player && _player->hasInventory()) {
            if (_networkCtrl->isHost()) {
                int deposited = _player->depositItem((size_t)depositIndex);
                if (deposited > 0) {
                    _inventoryValue = _player->getInventoryVal();
                    _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), deposited, true, depositIndex));
                }
            } else {
                _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), 0, false, depositIndex));
                CULog("CLIENT deposit request");
            }
        }
    }

    // MOBILE DEPOSIT: TAP FOR ALL!
    // if (depositIndex >= 0 && _canDeposit) {
    //     if (_player && _player->hasInventory()) {
    //         if (_networkCtrl->isHost()) {
    //             int deposited = _player->depositItems();
    //             if (deposited > 0) {
    //                 _inventoryValue = _player->getInventoryVal();
    //                 _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), deposited, true));
    //             }
    //         } else {
    //             _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID()));
    //             CULog("CLIENT deposit request");
    //         }
    //     }
    // }

    // DESKTOP DEPOSIT: F FOR ALL!
    if (depositRequested && _canDeposit) {
        if (_player && _player->hasInventory()) {
            if (_networkCtrl->isHost()) {
                int deposited = _player->depositItems();
                if (deposited > 0) {
                    _inventoryValue = _player->getInventoryVal();
                    _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), deposited, true));
                }
            } else {
                // Clients only send a request here. The host checks the
                // position and inventory before sending back the result
                _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID()));
                CULog("CLIENT deposit request");
            }
        }
    }

    // Spellcasting
    if (_input.getGesture() != SpellType::NONE) {
        SpellType newSpell = _input.getGesture();
        if (newSpell != _player->getSpell() && _player->hasSpell(newSpell)) {
            _player->setSpell(newSpell);
            _networkCtrl->pushOutEvent(
                SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), newSpell));
        }
    }

    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero()) {
        angle = _player->getDirection();
    }
    if (_input.shouldReleaseSpell()) {
        //CULog("Release spell--------------------------------------------------------");
        
        //CULog("Spell type: %d", _player->getSpell());
        if (_player->getSpell() == SpellType::NONE) {
            _input.resolveSpell();
            _player->resolveSpell();
        }
        else {
            SpellType t = SpellType::NONE;

            // check for possible combos - serena implemented
            for (auto p : _playerFact->current) {
                CULog("pLayer fact size: %zu", _playerFact->current.size());
                CULog("This is the spell %d", p->getSpell());
                if (_player->getPosition().distance(p->getPosition()) < COMBO_RADIUS && p->getSpell() != SpellType::NONE && p->getPID() != _player->getPID()) {
                    // calc combo spell + send combo event
                    CULog("combo potential");
                    if (_player->getSpell() == SpellType::FIREBALL) {
                        if (p->getSpell() == SpellType::FIREBALL) {
                            t = SpellType::COMBO_FIREBALL;
                            p->resolveSpell();
                            _networkCtrl->pushOutEvent(
                                SetSpellEvent::allocSetSpellEvent(p->getPID(), SpellType::NONE));
                            CULog("combo fireball");
                        }
                    }
                    else if (_player->getSpell() == SpellType::FREEZE) {
                        if (p->getSpell() == SpellType::FREEZE) {
                            t = SpellType::COMBO_FREEZE;
                            p->resolveSpell();
                            _networkCtrl->pushOutEvent(
                                SetSpellEvent::allocSetSpellEvent(p->getPID(), SpellType::NONE));
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

            //CULog("Casting spell type: %d", t);
            if (t == SpellType::DEATH) {
                for (int i = 0; i < 32; i++) {
                    angle = angle.rotate((3.141592653 / 16) * i);
                    auto params = _spellFact->serializeParams((playerScreenPos / PHYSICS_SCALE), _assets->get<graphics::Texture>("fireball")->getSize().height / 4.f, angle, t, _networkCtrl->getPID());
                    auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                    _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                }
            }
            else {
                auto params = _spellFact->serializeParams(playerScreenPos / PHYSICS_SCALE, _assets->get<graphics::Texture>("fireball")->getSize().height / 4.f, angle, t, _networkCtrl->getPID());
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
            }

            //We must acquire an object if we create it if we fail to do so the network
            //will consistently try to reset its position.
            _input.resolveSpell();
            _player->resolveSpell();
            _networkCtrl->pushOutEvent(
                SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), SpellType::NONE));
        }
    }

    if (_input.pressedPickup() && _currOverlappedItem >= 0) {
        auto item = _itemFact->getItem(_currOverlappedItem);
        // Checks if player can pick up anything so inventory doesn't overflow
        if (_player && _player->canAddItem() && item && !item->isRemoved()) {
            _networkCtrl->pushOutEvent(
                PickupEvent::allocPickupEvent(_player->getPID(), _currOverlappedItem)
            );
        }
        _input.resolvedPickup();
        _currOverlappedItem = -1;
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
    
    std::tuple<int,int> values = _networkCtrl->customEvents(_itemFact, _playerFact,
                                 _player, _input, &_playerWon, &_gameEnded, _depositPosition, _depositRadius);
    _inventoryValue = std::get<0>(values);
    _depositedValue += std::get<1>(values);
    
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
    Scene2::render();
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();
    drawDepositZone();
    if(_player){
        renderHUD();
    }
    
    _inputDisplay->draw(_batch);
    _batch->end();
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

    float timerX = screenSize.width / 2 - barW * 0.5f;
    float timerY = margin;
    drawBox(timerX + hudOffset.x, timerY + hudOffset.y, barW, barH, boxColor);

    if (_font)
    {
        _batch->setColor(Color4::WHITE);
        std::shared_ptr<TextLayout> timerLabel = TextLayout::allocWithText(
                    "Time Remaining: " + std::to_string(_timer/60), _font);
        timerLabel->layout();
        Affine2 timerTrans;
        timerTrans.translate(timerX + 10 + hudOffset.x, timerY + 12 + hudOffset.y );
        _batch->drawText(timerLabel, timerTrans);

        auto boxTex = _assets->get<graphics::Texture>("item_box");
        if (boxTex) {
            constexpr size_t kSlotCount = 5;
            const auto& carriedItems = _player->getInventory();

            for (size_t i = 0; i < kSlotCount; ++i) {
                Rect slot = getInventorySlotBounds(i);
                float boxX = slot.origin.x + hudOffset.x;
                float boxY = slot.origin.y + hudOffset.y;
                float boxSize = slot.size.width;

                // Draw the frame
                _batch->setColor(Color4::WHITE);
                _batch->draw(boxTex, Rect(boxX, boxY, boxSize, boxSize));

                // Draw item texture centered inside the frame
                if (i < carriedItems.size()) {
                    const auto& carried = carriedItems[i];
                    auto itemTex = _assets->get<graphics::Texture>(carried.itemType);
                    if (itemTex) {
                        float padding = 20.0f;
                        float iconSize = boxSize - padding * 2;
                        float iconX = boxX + padding;
                        float iconY = boxY + padding;
                        _batch->setColor(Color4::WHITE);
                        _batch->draw(itemTex, Rect(iconX, iconY, iconSize, iconSize));
                    }
                }
            }
        }

        auto texContainer = _assets->get<graphics::Texture>("health_bar");
        auto texFull = _assets->get<graphics::Texture>("health_full");
        auto texHalf = _assets->get<graphics::Texture>("health_half");
        auto texEmpty = _assets->get<graphics::Texture>("health_empty");

        // Scale container with 269x79 aspect ratio (only need to change container cause actualy heart size is based on container)
        float containerH = 79.0f * 0.8f;
        float containerW = 269.0f * 0.8f;
        float containerX = margin + hudOffset.x + 15.0f; //extra offset because health gets clipped on iphone corner
        float containerY = screenSize.height - containerH - margin + hudOffset.y;

        if (texContainer && texFull && texHalf && texEmpty) {
            // Draw the health bar
            _batch->setColor(Color4::WHITE);
            _batch->draw(texContainer, Rect(containerX, containerY, containerW, containerH));

            // Scale hearts to fit inside container with 83x78 aspect ratio
            float heartH = containerH * 0.75f;
            float heartW = heartH * (83.0f / 78.0f);

            // Space 3 hearts equally
            float totalHeartsW = containerW / 3.0f;
            int hp = _player->getHealth();

            for (int i = 0; i < 3; ++i) {
                int heartPoints = hp - i * 2;
                std::shared_ptr<graphics::Texture> tex;
                if (heartPoints >= 2)   
                    tex = texFull;
                else if (heartPoints == 1) 
                    tex = texHalf;
                else                       
                    tex = texEmpty;

                // Center each heart in its section
                float slotCenterX = containerX + totalHeartsW * i + totalHeartsW * 0.5f;
                float hx = slotCenterX - heartW * 0.5f;
                float hy = containerY + (containerH - heartH) * 0.5f;
                _batch->setColor(Color4::WHITE);
                _batch->draw(tex, Rect(hx, hy, heartW, heartH));
            }
        }

        // quota
        float quotaX = margin;
        float quotaY = screenSize.height - containerH - margin - barH - margin;
        drawBox(quotaX + hudOffset.x, quotaY + hudOffset.y, barW, barH, boxColor);

        std::shared_ptr<TextLayout> quotaLabel = TextLayout::allocWithText(
            "Current Deposit: " + std::to_string(_depositedValue) + " / " + std::to_string(_quota), _font);
        quotaLabel->layout();
        Affine2 quotaTrans;
        quotaTrans.translate(quotaX + 10 + hudOffset.x, quotaY + 12 + hudOffset.y);
        _batch->drawText(quotaLabel, quotaTrans);
    }
}

cugl::Rect GameScene::getInventorySlotBounds(size_t slotIndex) const {
    Size screenSize = this->getSize();
    float margin = 16;

    constexpr size_t kSlotCount = 5;
    // Scale 112x112 box to a specific size (tbd)
    float boxSize = 96.0f;
    float boxGap = 8.0f;
    float totalW = kSlotCount * boxSize + (kSlotCount - 1) * boxGap; // full inventory bar width

    // Center the row in the right half of the screen
    float startX = (screenSize.width * 0.5f) - (totalW * 0.5f);
    float topY = screenSize.height - margin - boxSize;

    return Rect(startX + slotIndex * (boxSize + boxGap), topY, boxSize, boxSize);
}

int GameScene::getInventorySlotIndexAt(const cugl::Vec2& tapPos) const {
    if (!_player) {
        return -1;
    }
    const auto& carriedItems = _player->getInventory();
    for (size_t i = 0; i < carriedItems.size(); ++i) {
        if (getInventorySlotBounds(i).contains(tapPos)) {
            return (int)i;
        }
    }
    return -1;
}

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

void GameScene::initNetwork(std::shared_ptr<NetworkController> n) {
    if (_networkCtrl!=nullptr) return;
    _networkCtrl = n;
}

void GameScene::initMap() {
    if (_networkCtrl->getRandSeed()==0) return; // no randSeed
    if (_mapSeed==_networkCtrl->getRandSeed()) return; // already init
    // initialize physics here, since this should be called post HANDSHAKE
    std::function<void(const std::shared_ptr<physics2::Obstacle>&, const std::shared_ptr<scene2::SceneNode>&)> linkSceneToObsFunc = [=, this](const std::shared_ptr<physics2::Obstacle>& obs, const std::shared_ptr<scene2::SceneNode>& node) {
        this->linkSceneToObs(obs, node);
        };
    
    _networkCtrl->enablePhysics(_world, linkSceneToObsFunc);
    _spellFactId = _networkCtrl->getPhysController()->attachFactory(_spellFact);
    _playerFactId = _networkCtrl->getPhysController()->attachFactory(_playerFact);
    _enemyFactId = _networkCtrl->getPhysController()->attachFactory(_enemyFact);
    _itemFactId = _networkCtrl->getPhysController()->attachFactory(_itemFact);
    CULog("Enemy factory ID: %u", _enemyFactId);
    CULog("Item factory ID: %u", _itemFactId);

    _mapSeed= _networkCtrl->getRandSeed();
    CULog("set map seed to : %d", _mapSeed);
    
    if (_mapSeed != 0) {
        rebuildMapForRun();
    }
    if(_networkCtrl -> isHost()){
        auto players = _networkCtrl -> getPlayers();
        for(auto& p : players){
            CULog("i am making a player with the following id %s", p.c_str());
            auto params = _playerFact->serializeParams(
                            _map->getPlayerSpawn() / PHYSICS_SCALE,
                            p
                        );
            _networkCtrl->getPhysController()
                            ->addSharedObstacle(_playerFactId, params);
                    }
        }
    
    if (!_networkCtrl->isHost()) {
        _networkCtrl->markReady();
        CULog("client ready - init map");
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
            CULog("I am trying to get a player using this id %s", _networkCtrl -> getPID().c_str());
            _player = _playerFact->getPlayerByPID(_networkCtrl -> getPID());
            
           

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

    if (node->getParent() != nullptr) {
            return;
        }
    node->setPosition(obj->getPosition() * PHYSICS_SCALE);
    _worldnode->addChild(node);
    if (auto item = std::dynamic_pointer_cast<ItemObstacle>(obj)) {
        _itemNodes.push_back({ item.get(), node });
    }
    if (auto playerObs = std::dynamic_pointer_cast<PlayerObstacle>(obj)) {
           bool alreadyTracked = false;
           for (const auto& p : _playerFact->current) {
               if (p == playerObs) {
                   alreadyTracked = true;
                   break;
               }
           }
           if (!alreadyTracked) {
               _playerFact -> addCurrent(playerObs);
           }
       }
    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        if (auto p = dynamic_pointer_cast<PlayerObstacle>(obj)) {
            std::shared_ptr<PlayerNode> p_node = std::dynamic_pointer_cast<PlayerNode>(node);
            obj->setListener([=, this](physics2::Obstacle* obs) {
                    PlayerObstacle::AnimationState state = p->getState();
                    if (p->getSpell() != SpellType::NONE) {
                        state = PlayerObstacle::AnimationState::CASTING;
                    } else if (state == PlayerObstacle::AnimationState::CASTING) {
                        state = PlayerObstacle::AnimationState::IDLE;
                    }
                    p_node->updateAnimationState(state);
                    float leftover = Application::get()->getFixedRemainder() / 1000000.f;
                    Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
                    float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
                    p_node->setPosition(pos * PHYSICS_SCALE);
                    p_node->setAngle(angle);
                    p_node->setVelocity(p->getLinearVelocity() * PHYSICS_SCALE);
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
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();

    physics2::Obstacle* obs1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* obs2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    if (obs1->isRemoved() || obs2->isRemoved()) {
        return;
    }

    SpellObstacle* spell = nullptr;
    PlayerObstacle* player = nullptr;
    PlayerObstacle* player2 = nullptr;
    EnemyObstacle* enemy = nullptr;
    cugl::physics2::PolygonObstacle* wall = nullptr;
    ItemObstacle* item = nullptr;

    // Get types of the 2 obstacles; only care about collisions between different types of obstacles
    if (auto s = dynamic_cast<SpellObstacle*>(obs1)) {
        spell = s;
    } else if (auto p = dynamic_cast<PlayerObstacle*>(obs1)) {
        player = p;
    } else if (auto e = dynamic_cast<EnemyObstacle*>(obs1)) {
        enemy = e;
    } else if (auto w = dynamic_cast<cugl::physics2::PolygonObstacle*>(obs1)) {
        wall = w;
    } else if (auto w = dynamic_cast<ItemObstacle*>(obs1)){
        item = w;
    }

    if (auto s = dynamic_cast<SpellObstacle*>(obs2)) {
        spell = s;
    } else if (auto p = dynamic_cast<PlayerObstacle*>(obs2)) {
        if(player){
            player2 = p;
        }else{
            player = p;
        }
        
    } else if (auto e = dynamic_cast<EnemyObstacle*>(obs2)) {
        enemy = e;
    } else if (auto w = dynamic_cast<cugl::physics2::PolygonObstacle*>(obs2)) {
        wall = w;
    } else if (auto w = dynamic_cast<ItemObstacle*>(obs2)){
        item = w;
    }

    // Checking for collisions
    if (spell && spell){
        contact -> SetEnabled(false);
    }
    if (player && spell) {
        contact -> SetEnabled(false);
        if (spell->getCasterUID()==_networkCtrl->getPID()) return;
        if (spell->TYPE == SpellType::HEAL) {
            int nh = player->getHealth() + HEAL_AMT;
            player->setHealth(nh);
            spell->markRemoved(true);
        }
        return;
    }
    if (player && enemy) {
        contact -> SetEnabled(false);
        float knockbackSpeed = 10.0f;
        b2Vec2 vel = enemy->getBody()->GetLinearVelocity();
        b2Vec2 dir = vel;
        float len = dir.Length();
        if (len > 0.001f) {
            dir.x /= len;
            dir.y /= len;
        }
        enemy->getBody()->SetLinearVelocity(b2Vec2(-dir.x * knockbackSpeed, -dir.y * knockbackSpeed));

        if (enemy->getState() != EnemyObstacle::EnemyState::ATTACK_RESET)
            player->setHealth(player->getHealth() - 1);
        enemy->setState(EnemyObstacle::EnemyState::ATTACK_RESET);
        return;
        }
    if (enemy && spell) {
        enemy->setHealth(enemy->getHealth() - spell->getDamage());
        if(spell->TYPE == SpellType::FREEZE or spell->TYPE == SpellType::COMBO_FREEZE){
            enemy -> setFrozen(true);
            if(spell->TYPE == SpellType::FREEZE){
                enemy->setFrozenTimer(60);
            }else{
                enemy->setFrozenTimer(240);
            }
            
        }
        if(spell->TYPE == SpellType::EARTH){
            enemy -> setMud(true);
            enemy -> setMudTimer(300);
        }
        if(!(spell->TYPE == SpellType::COMBO_FREEZE or spell->TYPE == SpellType::COMBO_FIREBALL or spell->TYPE == SpellType::DEATH)){
            spell->markRemoved(true);
        }
        
        if (enemy->getHealth()==0) enemy->markRemoved(true);
        return;
    }
    if (wall && player){
        contact->SetEnabled(true);
    }
    if(wall && spell){
        contact->SetEnabled(false);
    }
    if (wall && enemy){
        //CULog("here wall and enemy");
        if (enemy->TYPE == EnemyType::GHOST) {
            contact->SetEnabled(false);
            return;
        }
    }
    if(player && player2){
        contact -> SetEnabled(false);
    }
    if(player && item){
        contact -> SetEnabled(false);
        _currOverlappedItem = item->getID();
    }
    if(enemy && item){
        contact -> SetEnabled(false);
    }
}

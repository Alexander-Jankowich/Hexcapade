//
// GameScene.cpp
// This is the primary class file for running the game.  It contains all core gameplay loop logic as well as
// calls to the network controller for processing networked events. 
// Authors: Serena Duncan, Alexander Jankowich, Kaz Tam, Frank Xie, Galiba Anjum


#include "GameScene.h"



// Uncomment for testing, allows players to use all spells regardless of what books they have in their inventory
//#define ENABLE_ALL_SPELLS

using namespace cugl;
using namespace cugl::audio;
using namespace std;
using namespace cugl::graphics;

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
void GameScene::beginContact(b2Contact* contact) {
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<std::vector<std::string>>& rooms, const std::shared_ptr<NetworkController>& network)
{
    // Initialize the scene to a locked width
    if (assets == nullptr || !cugl::scene2::Scene2::initWithHint(Size(0, SCENE_HEIGHT)))
    {
        return false;
    }

    _addedLWalls = false;
    _assets = assets;
    _rooms = rooms;
    _networkCtrl = network;

    auto orthographicCamera = std::dynamic_pointer_cast<OrthographicCamera>(getCamera());
    if (orthographicCamera) {
        orthographicCamera->setZoom(std::max(1.3f, 0.02f));
        orthographicCamera->update();
    }

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

    // Initialize spell combo information
    initCombos();

    _map = std::make_shared<Map>();
    _mapSeed = 0;

    // Initialize inventory system
    _depositedValue = 0;
    _playerWon = false;
    _gameEnded = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    try {
        _nameFont = assets->get<Font>("CoolveticaRegular16");
    }
    catch (...) {
        _nameFont = nullptr;
    }

    try {
        _uiFont = assets->get<Font>("CoolveticaRegular48");
    }
    catch (...) {
        _uiFont = nullptr;
    }

    // this resets pickup prompt state
    _canDeposit = false;
    _depositRadius = 120.0f;
    _depositPosition = Vec2::ZERO;

    // Create all the factories
    _spellFact = SpellFactory::alloc(_assets);
    _playerFact = PlayerFactory::alloc(_assets);
    _enemyFact = EnemyFactory::alloc(_assets);
    _itemFact = ItemFactory::alloc(_assets);

    // Create action timeline
    _animations = Animator::alloc();

    // Create other controllers
    _pathfinder = Pathfinder();


    _world = physics2::distrib::NetWorld::alloc(Rect(0, 0, 200, 200), Vec2::ZERO);
    _world->activateCollisionCallbacks(true);
    _world->activateFilterCallbacks(false);
    _world->activateDestructionCallbacks(true);
    //_world->onBeginContact = [this](b2Contact* contact) {
    //        beginContact(contact);
    //    };
    _world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact, oldManifold);
        };
    _world->update(1);
    
    _itemInfo = ItemTypeClass(_assets);
    
    // Create the scene graph
    _worldnode = cugl::scene2::OrderedNode::allocWithOrder(cugl::scene2::OrderedNode::Order::ASCEND);
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(Vec2::ZERO);
    addChild(_worldnode);
    
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    // NOTHING TO DO THIS TIME
    hideInventoryCard();
    hideTutorial();
    _inputDisplay->dispose();
    _map = nullptr;
    _assets->dispose();
    _assets = nullptr;
}

void GameScene::reset()
{
    CULog("game scene reset called");
    hideInventoryCard();
    hideTutorial();
    _mapSeed = 0;
    _map->reset();
    _world->clear(true); // reset box2d + phys world
    // is this correct...?
    _worldnode->removeAllChildren();
    removeAllChildren();
    _worldnode = nullptr;

    _input.reset();
    _inputDisplay->reset();

    _player = nullptr;
    _enemyFact->reset();
    _itemFact->reset();
    _spellFact->reset();
    _playerFact->reset();

    _addedLWalls = false;
    _gameEnded = false;
    _playerWon = false;
    _depositedValue = 0;
    // Match startup behavior on replay: reset clears active enemies and restarts
    // the host spawn timer instead of forcing one immediate spawn.
    lastEnemySpawn = 0;

    for (auto& playerObs : _playerFact->current) {
        if (playerObs) {
            playerObs->clearInventory();
        }
    }
    // clear any tracked item visuals so the next run can relink fresh world items
    _itemNodes.clear();
    _inventoryValue = 0;
    _canDeposit = false;
    _pendingInventoryTapIndex = -1;
    _currOverlappedItem = -1;
    _timer = 0;
    _glowItemId = -1;
    _playerWalkSoundCooldown = 0.0f;
    _lastPlayerHealthForSound = -1;
    _enemyHealthForSound.clear();
    Scene::reset();
}

#pragma mark -
#pragma mark Inventory

void GameScene::removeItemNode(const std::shared_ptr<ItemObstacle>& item) {
    if (!item) {
        return;
    }

    for (auto it = _itemNodes.begin(); it != _itemNodes.end(); ++it) {
        if (it->first.get() != item.get()) {
            continue;
        }

        if (it->second) {
            it->second->removeFromParent();
        }
        _itemNodes.erase(it);
        return;
    }
}

void GameScene::showInventoryCard(size_t slotIndex) {
    hideInventoryCard();

    auto cardTex = _itemInfo.getCard(_player->getInventory()[slotIndex].itemType);
    //CULog("inventory slot %zu card %s opened", slotIndex, textureKey.c_str());

    _inventoryCardNode = scene2::PolygonNode::allocWithTexture(cardTex);
    _inventoryCardNode->setAnchor(Vec2::ANCHOR_CENTER);
    float zoom = getActiveCameraZoom();
    Vec2 offset = getHUDOffset();
    _inventoryCardNode->setScale(1.0f / zoom);
    _inventoryCardNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
        offset.y + this->getSize().height * 0.5f / zoom));
    _inventoryCardNode->setPriority(1.0f);
    addChild(_inventoryCardNode);
}

void GameScene::hideInventoryCard() {
    if (_inventoryCardNode) {
        // _inventoryCardNode->removeFromParent();
        removeChild(_inventoryCardNode);
        _inventoryCardNode = nullptr;
    }
}

void GameScene::showTutorialPage(int page) {
    hideTutorial();
    hideInventoryCard();

    const std::string textureKey = (page == 2) ? "tutorial_2" : "tutorial_1";
    auto tutorialTex = _assets->get<graphics::Texture>(textureKey);
    _tutorialPage = page;
    _tutorialNode = scene2::PolygonNode::allocWithTexture(tutorialTex);
    _tutorialNode->setAnchor(Vec2::ANCHOR_CENTER);
    float zoom = getActiveCameraZoom();
    Vec2 offset = getHUDOffset();
    _tutorialNode->setScale(1.0f / zoom);
    _tutorialNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
        offset.y + this->getSize().height * 0.5f / zoom));
    addChild(_tutorialNode);
}

void GameScene::hideTutorial() {
    if (_tutorialNode) {
        removeChild(_tutorialNode);
        _tutorialNode = nullptr;
    }
    _tutorialPage = 0;
}

void GameScene::rebuildMapForRun() {
    if (!_map) {
        _map = std::make_shared<Map>();
    }
    // Create the scene graph
    // TODO: maybe just init once...?
    _worldnode = cugl::scene2::OrderedNode::allocWithOrder(cugl::scene2::OrderedNode::Order::ASCEND);
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(Vec2::ZERO);
    addChild(_worldnode);

    auto n = _map->generate(_networkCtrl->getRound(), _mapSeed, _assets, _rooms, getSize(), _world, _networkCtrl->getNumPlayers());
    n->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    n->setPosition(Vec2::ZERO);
    n->setPriority(0.0f);
    _worldnode->addChild(n);

    _quota = _map->getQuota();
    _depositPosition = _map->getDepositSpawn();
    auto sprite = cugl::scene2::PolygonNode::allocWithTexture(_assets->get<graphics::Texture>("deposit"));
    //CULog("X: %f, Y %f", x,y);
    sprite->setPriority(99.0f);
    sprite->setPosition(Vec2(_depositPosition));
    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    float maxHeight = _world->getBounds().getMaxY();
    sprite->setPriority((maxHeight - _depositPosition.y / PHYSICS_SCALE) / maxHeight * 2);
    _worldnode->addChild(sprite);
    
    auto obs = physics2::BoxObstacle::alloc(_depositPosition/PHYSICS_SCALE, _assets->get<graphics::Texture>("deposit")->getSize()/PHYSICS_SCALE);
    obs->setDensity(1.0);
    obs -> setEnabled(true);
    obs->setBodyType(b2_staticBody);
    _world->addObstacle(obs);
    //obs->setPosition(0,0);
    _timer = _map->getTime() * 60;

    if (_networkCtrl->isHost()) {
        spawnItemsFromMap();
    }
    //updateItemPickupState();
    //updateDepositState();
    if (_player) {
        updateCameraFollow();
    }
}

void GameScene::spawnItemsFromMap() {
    auto itemSpawns = _map->getItemSpawns();
    if (itemSpawns.empty()) {
        return;
    }

    for (auto spawn : itemSpawns) {
        Vec2 physPos = spawn.first / PHYSICS_SCALE;
        ItemTypeClass::ItemType t = spawn.second;

        auto params = _itemFact->serializeParams(physPos, _itemInfo.getConsts(t)->getInt("value"), _world->getBounds().getMaxY(), t);
        _networkCtrl->getPhysController()->addSharedObstacle(_itemFactId, params);
    }
    CULog("ItemFactory: spawned %zu items", itemSpawns.size());
}

void GameScene::updateCameraFollow() {
    std::shared_ptr<OrthographicCamera> orthographicCamera = std::dynamic_pointer_cast<OrthographicCamera>(getCamera());
    float zoom = getActiveCameraZoom();
    float halfW = getSize().width / (2.0f * zoom);
    float halfH = getSize().height / (2.0f * zoom);

    const Rect& world = Rect(0, 0, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);
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

    Vec3 cameraPos = orthographicCamera->getPosition();
    cameraPos.x = cx;
    cameraPos.y = cy;
    orthographicCamera->setPosition(cameraPos);
    orthographicCamera->update();
}

float GameScene::getActiveCameraZoom() const {
    return std::max(std::dynamic_pointer_cast<OrthographicCamera>(getCamera())->getZoom(), 0.02f);
}

Vec2 GameScene::getHUDOffset() const {
    return _cameraBottomLeft;
}

// gameplay audio stays here because it depends on local player/enemy state
// sound.json owns the base mix; code multipliers are for context changes
bool GameScene::playSoundWithKey(const std::string& playbackKey, const std::string& assetKey,
                                 float volumeMultiplier, bool force) {
    auto engine = AudioEngine::get();
    if (engine == nullptr || _assets == nullptr) {
        return false;
    }
    if (!force && engine->isActive(playbackKey)) {
        // stable-key sounds like walking should not stack over themselves
        return false;
    }
    auto sound = _assets->get<Sound>(assetKey);
    return sound && engine->play(playbackKey, sound, false, volumeMultiplier, force);
}

bool GameScene::playUniqueSound(const std::string& key, float volumeMultiplier) {
    // unique keys let overlapping casts, hits, and damage sounds all play
    std::string playbackKey = key + "#game#" + std::to_string(_soundCounter++);
    return playSoundWithKey(playbackKey, key, volumeMultiplier, false);
}

bool GameScene::restartSound(const std::string& key, float volumeMultiplier) {
    // restart sounds are button-like feedback that should replace the old copy
    return playSoundWithKey(key, key, volumeMultiplier, true);
}

static std::string playerCharacterSoundKey(const std::string& character, const std::string& prefix) {
    // builds character-specific keys like walking_cat, damage_duck, death_fox
    if (character == "player_cat") return prefix + "cat";
    if (character == "player_duck") return prefix + "duck";
    if (character == "player_bunny") return prefix + "bunny";
    return prefix + "fox";
}

static std::string enemyDamageSoundKey(EnemyType type) {
    // enemy damage sounds are chosen from the enemy type that was hit
    switch (type) {
    case EnemyType::BAT:
    case EnemyType::SLOWBAT:
        return "damage_bat";
    case EnemyType::GHOST:
        return "damage_ghost";
    case EnemyType::OCTO:
        return "damage_octopus";
    }
    return "damage_statue";
}

static std::string enemyWalkSoundKey(EnemyType type) {
    // enemy walking uses low repeated movement sounds, not attack sounds
    switch (type) {
    case EnemyType::BAT:
    case EnemyType::SLOWBAT:
        return "walking_bat";
    case EnemyType::GHOST:
        return "walking_ghost";
    case EnemyType::OCTO:
        return "walking_octopus";
    }
    return "walking_bat";
}

static std::string spellCastSoundKey(SpellType type) {
    switch (type) {
    case SpellType::FIREBALL:
        return "spell_fire";
    case SpellType::FREEZE:
        return "spell_ice";
    case SpellType::HEAL:
        return "spell_heal";
    case SpellType::DEATH:
        return "spell_dark";
    case SpellType::WATER:
        return "spell_water";
    case SpellType::EARTH:
        return "spell_earth";
    case SpellType::BUFF:
        return "spell_buff";
    case SpellType::SHIELD:
        return "spell_shield";
    case SpellType::COMBO_FREEZE:
        return "spell_big_ice";
    case SpellType::COMBO_EFREEZE:
        return "spell_stone";
    case SpellType::COMBO_FIREBALL:
        return "spell_big_fire";
    case SpellType::STEAM:
        return "spell_steam";
    case SpellType::MUD_WATER:
        return "spell_mud_water";
    default:
        return "";
    }
}

static std::string enemyAttackSoundKey(EnemyType type) {
    // this maps enemy attacks to the sound key in sound.json
    switch (type) {
    case EnemyType::BAT:
    case EnemyType::SLOWBAT:
        return "attack_bat";
    case EnemyType::GHOST:
        return "attack_ghost";
    case EnemyType::OCTO:
        return "attack_octopus";
    }
    return "";
}

void GameScene::updateWalkingSounds(float dt) {
    // walking is movement-gated for right now, working on frame/animation sync
    if (_playerWalkSoundCooldown > 0.0f) {
        _playerWalkSoundCooldown -= dt;
    }

    if (_player->getHealth() > 0 && !_player->isHurt() &&
        !_player->isCasting() && _player->getVelocity().length() > 0.05f &&
        _playerWalkSoundCooldown <= 0.0f) {
        std::string key = playerCharacterSoundKey(_networkCtrl->getPlayerCharacter(_player->getPID()), "walking_");
        playSoundWithKey(key, key, 1.0f, false);
        // player footsteps wait before firing again so movement does not spam audio
        _playerWalkSoundCooldown = 0.36f;
    }

    for (auto enemy : _enemyFact->current) {
        if (!enemy->isDead() && !enemy->isFroze() && !enemy->isHurt() && !enemy->isAttacking() &&
            enemy->getLinearVelocity().length() > 0.05f &&
            _player->getPosition().distance(enemy->getPosition()) <= 12.0f) {
            std::string key = enemyWalkSoundKey(enemy->TYPE);
            // one stable key per enemy type keeps repeated enemy walking from stacking too loudly
            playSoundWithKey(key, key, 1.0f, false);
        }
    }
}

void GameScene::playSpellCastSound(SpellType type) {
    // spell casts use unique playback so back-to-back gestures overlap
    std::string key = spellCastSoundKey(type);
    if (!key.empty()) {
        playUniqueSound(key, 1.0f);
    }
}

void GameScene::playEnemyAttackSound(EnemyType type) {
    // enemy attacks use unique playback so nearby hits can overlap
    std::string key = enemyAttackSoundKey(type);
    if (!key.empty()) {
        playUniqueSound(key, 1.0f);
    }
}

void GameScene::playItemDepositSound() {
    // restart deposit because players can spam this like a button
    CULog("sound play: deposit");
    restartSound("general_alter_deposit", 1.0f);
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
#pragma mark Input Updates and Player Initialiation
    if (!_player) {
        // Try to find our player now that replication may have run
        _player = _playerFact->getPlayerByPID(_networkCtrl->getPID());
        //Have we set our player yet / received signal
        if (!_player) return;
        _networkCtrl->initGameStuff(_itemFact, _playerFact, _enemyFact, _itemFactId,
            _player, _input, _depositPosition, _depositRadius, _world->getBounds().getMaxY());
    }
    if (_lastPlayerHealthForSound < 0) {
        _lastPlayerHealthForSound = _player->getHealth();
    }
    _glowItemId = -1;
    Vec2 playerPos = _player->getPosition();
    for (auto& pair : _itemNodes) {
        if (pair.first && !pair.first->isRemoved()) {
            // set PICKUP DISTANCE HERE
            if (playerPos.distance(pair.first->getPosition()) <= 0.8f) {
                _glowItemId = pair.first->getID();
                break;
            }
        }
    }
    if (_player && !_addedLWalls) {
        CULog("adding lower walls");
        _map->getLowerWallNode()->setPriority(1.0f);
        _worldnode->addChild(_map->getLowerWallNode());
        _addedLWalls = true;
    }
    //We have to acquire ourselves before we attempt to move
    //it is a little weird to do this every frame but this allows host to acquire themselves
    _networkCtrl->getPhysController()->acquireObs(_player, 0);

#ifndef ENABLE_ALL_SPELLS
    _input.syncGesturesToInventory(_player->getInventory());
#endif // !ENABLE_ALL_SPELLS

    if (_gameEnded) {
        return;
    }

    if (_timer > 0) {
        _timer -= 1;
    }
    if (_timer == 0) {
        bool won = _quota <= _depositedValue;
//        _playerWon = won;
        _networkCtrl->pushOutEvent(EndGameEvent::allocEndGameEvent(won));
        return;
    }
    // all dead check
    if (_networkCtrl->isHost() && _playerFact->allPlayersDead()) {
//        _gameEnded = true;
//        _playerWon = false;
        _networkCtrl->pushOutEvent(EndGameEvent::allocEndGameEvent(false));
        return;
    }
    

    _input.update(dt);

    if (_input.pressedReset()) {
        reset();
        _input.resolvedReset();
        return;
    }
#pragma mark Enemy Spawning Logic, Pathfinding, and Removal
    if (_networkCtrl->isHost()) {
        // spawn an enemy if host and need to spawn
        lastEnemySpawn += dt;
        if (lastEnemySpawn > _map->getEnemInterval()) {
            lastEnemySpawn = 0;

            auto res = _map->generateEnemySpawns(_playerFact->getPlayerPositions(), _enemyFact->getEnemyPositions());
            for (auto r : res) {
                auto params = _enemyFact->serializeParams(r.second, r.first / PHYSICS_SCALE);
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
                //If we create it we MUST acquire it
                _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
            }
        }

        auto toRemove = _enemyFact->getRemoved();
        for (auto en : toRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(en);
        }

        // Update enemy movement
        _pathfinder.updateEnemies(_enemyFact, _playerFact, _spellFact, _networkCtrl, _spellFactId, _world);

        // Send network events if enemies have updated
        for (const auto& en : _enemyFact->current) {
            if (en->isProperiesDirty()) {
                _networkCtrl->pushOutEvent(EnemyStateEvent::alloc(en));
                en->setPropertiesDirty(false);
            }
        }

        auto itRemove = _itemFact->update();
        for (auto item : itRemove) {
            removeItemNode(item);
            if (!item->isRemoved()) {
                _networkCtrl->getPhysController()->removeSharedObstacle(item);
            }
        }
        
        // removing players that disconnected
        auto pRemove = _playerFact->getRemoved();
        for (auto p : pRemove) {
            _networkCtrl->getPhysController()->removeSharedObstacle(p);

            auto params = _spellFact->serializeParams(
                (p->getPosition()),
                Vec2::UNIT_X, SpellType::STEAM, _networkCtrl->getPID(),_player -> getBuff(), _weakSpell);
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
            _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
        }
    }

    // Remove spells that have gone out of range
    auto spellRemove = _spellFact->update();
    for (auto spell : spellRemove) {
        spell->markRemoved(true);
        _networkCtrl->getPhysController()->removeSharedObstacle(spell);
        //CULog("Remove spell: %d", spell->isRemoved());
    }


    // Update animations
    _animations->update(dt);

    if (_player->getHealth() == 0) {
        _input.clearInput();
        return;
    } // if dead ignore updates


    _player->updateTimers(dt);

    if (_player->_bombExpired) {
        _player->_bombExpired = false;
        int bombIdx = _player->_bombExpiredIndex;
        _player->_bombExpiredIndex = -1;
        playUniqueSound("artifact_exploding_potion", 1.0f);
        if (_networkCtrl->isHost()) {
            _player->depositItem((size_t)bombIdx);
        }
        _inventoryValue = _player->getInventoryVal();
        _networkCtrl->pushOutEvent(
            DepositEvent::allocDepositEvent(_player->getPID(), 0, true, bombIdx));
    }
    if (_player->getHealth() < _lastPlayerHealthForSound) {
        // play local damage when health actually drops
        playUniqueSound(playerCharacterSoundKey(_networkCtrl->getPlayerCharacter(_player->getPID()),
            _player->getHealth() <= 0 ? "death_" : "damage_"), 1.0f);
    }
    _lastPlayerHealthForSound = _player->getHealth();

#pragma mark Player Movement
    // Player movement
    cugl::Vec2 input = _input.getMove();
    if (_input.showAim()) {
        _player->setAimSlowTimer(_player->getAimSlowTimer() - dt);
    }
    else {
        _player->setAimSlowTimer(2.0f);
    }
    cugl::Vec2 velocity = input * _player->getSpeed();

    if (_player->isFrozen()) {
        //_player->setVelocity(Vec2::ZERO);
    }
    else if (_player -> getWater()){
        _player -> setVelocity(_player -> getPushDir() * _player -> getSpeed());
    }
    else if (!_player->isHurt()) {
        _player->setVelocity(velocity);
    }
    updateWalkingSounds(dt);
    cugl::Vec2 playerScreenPos = _player->getPosition() * PHYSICS_SCALE;

#pragma mark Inventory Card, Tutorial Card, and Camera Logic
    updateCameraFollow();
    Vec2 offset = getHUDOffset();
    _input.setCameraOffset(offset);
    _inputDisplay->setUIOffset(offset);
    _inputDisplay->setUIZoom(getActiveCameraZoom());
    _inputDisplay->update(playerScreenPos);

    if (_inventoryCardNode) {
        float zoom = getActiveCameraZoom();
        _inventoryCardNode->setScale(1.0f / zoom);
        _inventoryCardNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
            offset.y + this->getSize().height * 0.5f / zoom));
    }
    if (_tutorialNode) {
        float zoom = getActiveCameraZoom();
        _tutorialNode->setScale(1.0f / zoom);
        _tutorialNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
            offset.y + this->getSize().height * 0.5f / zoom));
    }

    if (_input.cardDismissTap()) {
        Vec2 tapPos = _input.getInventoryTouchPos();
        if (_tutorialPage > 0) {
            Size tutorialSize = _tutorialNode->getSize();
            Vec2 tutorialCenter = _tutorialNode->getPosition() - offset;
            Rect tutorialBounds(tutorialCenter.x - tutorialSize.width * 0.5f,
                tutorialCenter.y - tutorialSize.height * 0.5f,
                tutorialSize.width,
                tutorialSize.height);
            float pageMidX = tutorialBounds.origin.x + tutorialBounds.size.width * 0.5f;

            if (tutorialBounds.contains(tapPos) && _tutorialPage == 1 && tapPos.x >= pageMidX) {
                // right side of tutorial page clicked go to tutorial page 2
                restartSound("general_item_card", 1.0f);
                showTutorialPage(2);
            }
            else if (tutorialBounds.contains(tapPos) && _tutorialPage == 2 && tapPos.x < pageMidX) {
                // left side of tutorial page clicked go back to tutorial page 1
                restartSound("general_item_card", 1.0f);
                showTutorialPage(1);
            }
            else {
                // tutorial tap was not a page turn so close it
                restartSound("general_item_card_close", 1.0f);
                hideTutorial();
            }
            _input.resolvedInventoryRelease();
            _input.resolvedCardDismissTap();
            return;
        }
        else if (getHelpButtonBounds().contains(tapPos)) {
            // help button clicked open tutorial page 1
            restartSound("general_item_card", 1.0f);
            showTutorialPage(1);
            _input.resolvedInventoryRelease();
            _input.resolvedCardDismissTap();
            return;
        }
    }

    if (_inventoryCardNode) {
        if (_input.cardDismissTap()) {
            //CULog("click detected, card closed");
            restartSound("general_item_card_close", 1.0f);
            hideInventoryCard();
            _pendingInventoryTapIndex = -1;
        }
    }
    _input.resolvedCardDismissTap();

#pragma mark Deposit handling and Inventory card display
    bool depositRequested = false;
    int depositIndex = -1;
    _canDeposit = playerScreenPos.distance(_depositPosition) <= _depositRadius;
    //CULog("dist: %.2f", playerScreenPos.distance(_depositPosition));

    if (_input.pressedDeposit()) {
        depositRequested = true;
        _input.resolvedDeposit();
    }

    if (_input.inventoryReleased()) {
        int tappedIndex = getInventorySlotIndexAt(_input.getInventoryAnchor());
        bool tappedSlot = _input.getInventoryAnchor().distance(_input.getInventoryTouchPos()) <= 5.0f;

        if (tappedSlot && tappedIndex >= 0) {
            Timestamp now;
            bool doubleTap = _pendingInventoryTapIndex == tappedIndex &&
                Timestamp::ellapsedMillis(_lastInventoryTapTime, now) <= 200;

            if (doubleTap) {
                CULog("slot double tapped");
                // double tap means deposit so do not play the normal card click
                depositIndex = tappedIndex;
                _pendingInventoryTapIndex = -1;
                hideInventoryCard();
            }
            else {
                CULog("slot tapped");
                hideInventoryCard();
                _pendingInventoryTapIndex = tappedIndex;
                _lastInventoryTapTime = now;
            }
        }
        else {
            _pendingInventoryTapIndex = -1;
        }
        _input.resolvedInventoryRelease();
    }

    if (_pendingInventoryTapIndex >= 0) {
        Timestamp now;
        if (Timestamp::ellapsedMillis(_lastInventoryTapTime, now) > 200) {
            // wait past the double tap window before treating this as a card open
            restartSound("general_item_card", 1.0f);
            showInventoryCard((size_t)_pendingInventoryTapIndex);
            _pendingInventoryTapIndex = -1;
        }
    }

    if (_input.getInventoryHotkeyIndex() >= 0) {
        depositIndex = _input.getInventoryHotkeyIndex();
        _input.resolvedInventoryHotkeyIndex();
    }

    // MOBILE DEPOSIT: DRAG ONE SLOT WHILE THE PLAYER IS IN DEPOSIT RANGE
    if (depositIndex >= 0) {
        if (_player->hasInventory()) {
            if (_networkCtrl->isHost()) {
                if (_canDeposit) {
                    int deposited = _player->depositItem((size_t)depositIndex);
                    if (deposited > 0) {
                        // deposit sound only plays after a successful drop off
                        _inventoryValue = _player->getInventoryVal();
                        playItemDepositSound();
                        _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), deposited, true, depositIndex));
                    }
                }

                // Drop anywhere else in the world.
                // Artifact bomb cannot be dropped!
                else if ((size_t)depositIndex < _player->getInventory().size() && !_player->isNonDroppable((size_t)depositIndex)) {
                    auto carried = _player->getInventory()[(size_t)depositIndex];
                    _player->depositItem((size_t)depositIndex);
                    _inventoryValue = _player->getInventoryVal();
                    Vec2 dropPos = _player->getPosition() + Vec2(0.0f, -0.35f);
                    auto params = _itemFact->serializeParams(dropPos, carried.value, _world->getBounds().getMaxY(), carried.itemType);
                    auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_itemFactId, params);
                    _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                    // drop feedback is separate from altar deposit
                    restartSound("general_drop", 1.0f);
                    _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), 0, true, depositIndex));
                }
            }
            else {
                _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), 0, false, depositIndex));
                CULog("CLIENT deposit request");
            }
        }
    }

    // DESKTOP DEPOSIT: F FOR ALL!
    if (depositRequested && _canDeposit) {
        if (_player && _player->hasInventory()) {
            if (_networkCtrl->isHost()) {
                int deposited = _player->depositItems();
                if (deposited > 0) {
                    // deposit sound only plays after a successful drop off
                    _inventoryValue = _player->getInventoryVal();
                    playItemDepositSound();
                    _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), deposited, true));
                }
            }
            else {
                // Clients only send a request here. The host checks the
                // position and inventory before sending back the result
                _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID()));
                CULog("CLIENT deposit request");
            }
        }
    }

#pragma mark Spellcasting, Combos, Blocking, and Casting logic
   // Spellcasting
   //This is really terrible... I know we said no casual comments but I apologize - Alex
    if (_player->blocksSpellcasting()) {
        // restriction artifact needs to supress all spells (even the ones casted alr)
        if (_player->getSpell() != SpellType::NONE) {
            _player->resolveSpell();
            _networkCtrl->pushOutEvent(
                SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), SpellType::NONE));
        }
        _input.resolveSpell();
    }
    else if (_input.getGesture() != SpellType::NONE && _player->getSpell() == SpellType::NONE) {
        SpellType newSpell = _input.getGesture();
        _weakSpell = false;
        _player->setSpell(newSpell);
        _networkCtrl->pushOutEvent(
            SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), newSpell));
    }else if (_input.getGestureLess() != SpellType::NONE && _player -> getSpell() == SpellType::NONE){
        SpellType newSpell = _input.getGestureLess();
        _player->setSpell(newSpell);
        _weakSpell = true;
        _networkCtrl->pushOutEvent(
            SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), newSpell));
    }
    else if (_input.failedGesture()) {
        // failed cast feedback
        restartSound("effect_error", 1.0f);
        _input.resolvedFailedGesture();
    }

    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero()) {
        angle = _player->getDirection();
    }

    if (_input.shouldReleaseSpell() && !_player->blocksSpellcasting()) {
        //CULog("Release spell--------------------------------------------------------");

        //CULog("Spell type: %d", _player->getSpell());
        if (_player->getSpell() == SpellType::NONE) {
            // failed cast feedback
            restartSound("effect_error", 1.0f);
            _input.resolveSpell();
            _player->resolveSpell();
        }
        else {
            //Spell and Combo Logic
            auto combo = getCombo();
            SpellType t = combo.first;

            // Not a combo, use base spell
            if (t == SpellType::NONE) {
                t = _player->getSpell();
            }
            else {
                // Send event stuff
                _networkCtrl->pushOutEvent(SetSpellEvent::allocSetSpellEvent(combo.second, SpellType::NONE));
                _playerFact->getPlayerByPID(combo.second)->resolveSpell();

                // Not sure why this exists, check later
                _networkCtrl->pushOutEvent(CastSpellEvent::allocCastSpellEvent(_player->getPID(), combo.second, t));

            }

            //CULog("Casting spell type: %d", t);
            // local player cast only enemy projectiles do not use player spell sfx
            playSpellCastSound(t);
            switch (t) {
            case SpellType::DEATH:
                for (int i = 0; i < 16; i++) {
                    angle = angle.rotate((M_PI / 8) * i);
                    auto params = _spellFact->serializeParams((playerScreenPos / PHYSICS_SCALE), angle, t, _networkCtrl->getPID());
                    auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                    _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                }
                _player->consumeItem(ItemTypeClass::ItemType::BOOK_DEATH);
                break;
            case SpellType::WATER: {
                int numShots = 8;
                float start = M_PI / 6.0f;
                float end = -M_PI / 6.0f;
                for (int i = 0; i < numShots; i++) {
                    float alpha = (float)i / (numShots - 1);
                    float theta = start + alpha * (end - start);

                    Vec2 dir = angle.getNormalization().rotate(theta);

                    auto params = _spellFact->serializeParams(
                        (playerScreenPos / PHYSICS_SCALE),
                        dir, t, _networkCtrl->getPID(), _player -> getBuff(), _weakSpell);
                    auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                    _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                }
                break;
            }
            case SpellType::MUD_WATER: {
                int numShots = 8;
                float start = M_PI / 6.0f;
                float end = -M_PI / 6.0f;
                for (int i = 0; i < numShots; i++) {
                    float alpha = (float)i / (numShots - 1);
                    float theta = start + alpha * (end - start);

                    Vec2 dir = angle.getNormalization().rotate(theta);

                    auto params = _spellFact->serializeParams(
                        (playerScreenPos / PHYSICS_SCALE),
                        dir, t, _networkCtrl->getPID(),_player -> getBuff(), _weakSpell);
                    auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                    _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                }
                break;
            }
            default:
                int damageBonus = _player->getDamageBonus();
                auto params = _spellFact->serializeParams(playerScreenPos / PHYSICS_SCALE, angle, t, _player->getPID(), damageBonus,
                    _player->getBuff(), _weakSpell);
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
                _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
                break;
            }


            _input.resolveSpell();
            _player->resolveSpell();
            _player->setReleasedSpell(true);
            _networkCtrl->pushOutEvent(
                SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), SpellType::NONE));
        }
    }
#pragma mark Pickup Logic
    if (_input.pressedPickup() && _glowItemId >= 0) {
        CULog("trying to resolve pickup");
        auto item = _itemFact->getItem(_glowItemId);
        if (_player && _player->canAddItem() && item && !item->isRemoved()) {
            // local pickup feedback; host still validates the pickup
            restartSound("general_pickup", 1.0f);
            _networkCtrl->pushOutEvent(
                PickupEvent::allocPickupEvent(_player->getPID(), _glowItemId)
            );
        }
    }
    if (_input.pressedPickup()) {
        // I just use this to test combo animations - Kaz
        //_player->setShield(true);
        //_player->setShieldCount(3);
        _input.resolvedPickup();
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
    
    // check disconnected players lol
    for (auto p : _playerFact->current) {
        auto curr_players = _networkCtrl->getPlayers();
        if (!curr_players.contains(p->getPID())) {
            p->markRemoved(true);
            _networkCtrl->getPhysController()->acquireObs(p, -1);
        }
    }
    std::tuple<int, int> values = _networkCtrl->customEvents();
    _gameEnded = _networkCtrl->gameState() == NetworkController::NCGameState::WON || _networkCtrl->gameState() == NetworkController::NCGameState::LOST;
    _playerWon = _networkCtrl->gameState() == NetworkController::NCGameState::WON;
    _inventoryValue = std::get<0>(values);
    _depositedValue += std::get<1>(values);
    if(_player and _player -> isDirty()){
        _networkCtrl->pushOutEvent(
            PlayerStateEvent::allocPlayerStateEvent(_networkCtrl->getPID(), _player -> getHealth(), _player -> isFrozen(), _player -> getBuff(), _player -> getMud(), _player -> getShieldCount()));
        _player -> setDirty(false);
    }
    

    _currOverlappedItem = -1;
    _canDeposit = false;
    _world->update(step / 1000000.0f);
    if (_player && _lastPlayerHealthForSound >= 0 && _player->getHealth() < _lastPlayerHealthForSound) {
        // play local damage when physics makes health drop
        playUniqueSound(playerCharacterSoundKey(_networkCtrl->getPlayerCharacter(_player->getPID()),
            _player->getHealth() <= 0 ? "death_" : "damage_"), 1.0f);
    }
    if (_player) {
        _lastPlayerHealthForSound = _player->getHealth();
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
    scene2::PolygonNode* glowedNode = nullptr;
    std::shared_ptr<cugl::graphics::Texture> normalTex = nullptr;
    if (_glowItemId >= 0) {
        for (auto& pair : _itemNodes) {
            if (pair.first && pair.first->getID() == _glowItemId) {
                auto polyNode = dynamic_cast<scene2::PolygonNode*>(pair.second.get());
                if (polyNode) {
                    auto glowTex = _itemInfo.getGlowTexture(pair.first->getItemType());
                    if (glowTex) {
                        normalTex = polyNode->getTexture();
                        polyNode->setTexture(glowTex);
                        glowedNode = polyNode;
                    }
                }
                break;
            }
        }
    }

    Scene2::render();

    if (glowedNode && normalTex) {
        glowedNode->setTexture(normalTex);
    }
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();

    if (_nameFont && _playerFact) {
        auto nameBgTex = _assets->get<graphics::Texture>("name_bg");
        for (const auto& p : _playerFact->current) {
            if (!p || p->isRemoved()) continue;
            const std::string& name = _networkCtrl->getPlayerName(p->getPID());
            if (name.empty()) continue;

            auto label = TextLayout::allocWithText(name, _nameFont);

            label->layout();

            cugl::Vec2 worldPos = p->getPosition() * PHYSICS_SCALE;
            float nameOffsetY = 30.0f;
            float labelW = label->getBounds().size.width;
            float labelH = label->getBounds().size.height;
            float padX = 10.0f;

            float bgH = 20.0f;
            float bgW = labelW + padX * 2.0f;
            float bgX = worldPos.x - bgW * 0.5f;
            float bgY = worldPos.y + nameOffsetY;

            //            if (nameBgTex) {
            //                _batch->setColor(Color4::WHITE);
            //                _batch->draw(nameBgTex, Rect(bgX, bgY, bgW, bgH));
            //            }

            Affine2 t;
            t.translate(worldPos.x - labelW * 0.5f, worldPos.y + nameOffsetY);
            _batch->setColor(Color4::BLACK);
            _batch->drawText(label, t);
        }
    }

    if (_player) {
        renderHUD();
    }

    _inputDisplay->draw(_batch);
    if (_inventoryCardNode) {
        _inventoryCardNode->render(_batch);
    }
    if (_tutorialNode) {
        _tutorialNode->render(_batch);
    }
    _batch->end();
}

void GameScene::renderHUD()
{
    Size screenSize = this->getSize();
    Vec2 hudOffset = getHUDOffset();
    float zoom = getActiveCameraZoom();

    float margin = 16;

    auto hudX = [hudOffset, zoom](float value) {
        return hudOffset.x + value / zoom;
        };
    auto hudY = [hudOffset, zoom](float value) {
        return hudOffset.y + value / zoom;
        };
    auto hudSize = [zoom](float value) {
        return value / zoom;
        };

    // Helper lambda to draw a solid colored box using Poly2
    auto drawBox = [&](float x, float y, float w, float h, Color4 color)
        {
            std::vector<Vec2> verts = {
                Vec2(x, y),
                Vec2(x + w, y),
                Vec2(x + w, y + h),
                Vec2(x, y + h) };
            std::vector<Uint32> idx = { 0, 1, 2, 0, 2, 3 };
            Poly2 box(verts, idx);
            _batch->setColor(color);
            _batch->fill(box);
        };

    _batch->setTexture(nullptr);

    // Inventory bar
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

            if (i < carriedItems.size()) {
                const auto& carried = carriedItems[i];
                if (carried.itemType == ItemTypeClass::ItemType::BOMB && carried.bombTimer >= 0 && carried.bombTimer <= 300) {
                    float t = 1.0f - ((float)carried.bombTimer / 300.0f);
                    float padding = boxSize * 0.1f;
                    _batch->setTexture(nullptr);
                    drawBox(boxX + padding, boxY + padding, boxSize * 0.8f, boxSize * 0.8f, Color4(255, 0, 0, (Uint8)(90 * t)));
                }
            }

            // Draw item texture centered inside the frame
            if (i < carriedItems.size()) {
                const auto& carried = carriedItems[i];
                auto itemTex = _itemInfo.getTexture(carried.itemType);
                if (itemTex) {
                    float padding = 0.0f;
                    float maxSize = boxSize - padding * 2;

                    // Preserve 3:4 aspect ratio
                    float iconW = maxSize * (3.0f / 4.0f);
                    float iconH = maxSize;

                    float iconX = boxX + (boxSize - iconW) * 0.5f;
                    float iconY = boxY + padding;

                    _batch->setColor(Color4::WHITE);
                    _batch->draw(itemTex, Rect(iconX, iconY, iconW, iconH));
                }
            }
        }
    }

    // Help button
    auto helpTex = _assets->get<graphics::Texture>("help_button");
    if (helpTex) {
        Rect helpBounds = getHelpButtonBounds();
        _batch->setColor(Color4::WHITE);
        _batch->draw(helpTex, Rect(helpBounds.origin.x + hudOffset.x,
            helpBounds.origin.y + hudOffset.y,
            helpBounds.size.width,
            helpBounds.size.height));
    }

    auto texContainer = _assets->get<graphics::Texture>("health_bar");
    auto texFull = _assets->get<graphics::Texture>("health_full");
    auto texHalf = _assets->get<graphics::Texture>("health_half");
    auto texEmpty = _assets->get<graphics::Texture>("health_empty");
    auto texHourglass = _assets->get<graphics::Texture>("hourglass");

    float iconRowH = 79.0f * 0.8f;
    float heartRowH = iconRowH;
    float innerPad = 2.0f; // gap between the rows

    float containerW = 269.0f * 0.8f;
    float totalPanelH = heartRowH + innerPad + iconRowH + innerPad + iconRowH;

    float containerX_local = margin + 25.0f;
    float containerY_local = screenSize.height - totalPanelH - margin;

    float containerX = hudX(containerX_local);
    float containerY = hudY(containerY_local);
    float containerWs = hudSize(containerW);
    float totalPanelHs = hudSize(totalPanelH);

    if (texContainer) {
        _batch->setColor(Color4::WHITE);
        _batch->draw(texContainer, Rect(containerX, containerY, containerWs, totalPanelHs));
    }
    else {
        // Fallback
        drawBox(containerX, containerY, containerWs, totalPanelHs, Color4(30, 30, 30, 220));
    }

    float quotaRowY_local = containerY_local;
    float timerRowY_local = containerY_local + iconRowH + innerPad;
    float heartsRowY_local = containerY_local + 2.0f * (iconRowH + innerPad);

    // ---- Hearts row -----------------------------------------------
    if (texFull && texHalf && texEmpty) {
        float heartRowYs = hudY(heartsRowY_local);
        float heartRowHs = hudSize(heartRowH);
        float containerWs2 = hudSize(containerW);

        float heartH = heartRowHs * 0.6f;
        float heartW = heartH * (83.0f / 78.0f);

        float totalHeartsW = containerWs2 / 3.0f;
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

            float slotCenterX = containerX + totalHeartsW * i + totalHeartsW * 0.5f;
            float hx = slotCenterX - heartW * 0.5f;
            float hy = heartRowYs + (heartRowHs - heartH) * 0.5f;
            _batch->setColor(Color4::WHITE);
            _batch->draw(tex, Rect(hx, hy, heartW, heartH));
        }
    }

    // ---- Timer row ---------------------------------------------
    if (_font) {
        float timerRowYs = hudY(timerRowY_local);
        float iconRowHs = hudSize(iconRowH);
        float iconSize = iconRowHs * 0.7f;
        float textOffsetX = iconSize + hudSize(8.0f);

        if (texHourglass) {
            float iconY = timerRowYs + (iconRowHs - iconSize) * 0.5f;
            _batch->setColor(Color4::WHITE);
            _batch->draw(texHourglass, Rect(containerX + hudSize(8.0f), iconY, iconSize, iconSize));
        }

        int totalSecs = (int)(_timer / 60.0f);
        int mins = totalSecs / 60;
        int secs = totalSecs % 60;
        std::stringstream tss;
        tss << mins << ":" << std::setw(2) << std::setfill('0') << secs;

        auto timerLabel = TextLayout::allocWithText(tss.str(), _uiFont);
        timerLabel->layout();
        float timerTextH = timerLabel->getBounds().size.height;
        float timerTextW = timerLabel->getBounds().size.width;
        float timerTextX = containerX + hudSize(8.0f) + textOffsetX + (hudSize(containerW) - hudSize(8.0f) - textOffsetX - hudSize(timerTextW)) * 0.5f;
        float textY = timerRowY_local + (iconRowH - timerTextH) * 0.5f + 15.0f;
        Affine2 timerTrans;
        timerTrans.scale(1.0f / zoom).translate(timerTextX, hudY(textY));
        _batch->setColor(Color4::BLACK);
        _batch->drawText(timerLabel, timerTrans);
    }

    // ---- Quota row ---------------------------------------------
    if (_font) {
        float quotaRowYs = hudY(quotaRowY_local);
        float iconRowHs = hudSize(iconRowH);
        float iconSize = iconRowHs * 0.7f;
        float textOffsetX = iconSize + hudSize(8.0f);

        float quotaPct = (_quota > 0) ? ((float)_depositedValue / (float)_quota) : 0.0f;
        std::string quotaTexKey;
        if (quotaPct >= 1.0f)
            quotaTexKey = "quota_full";
        else if (quotaPct >= 0.75f)
            quotaTexKey = "quota_75";
        else if (quotaPct >= 0.5f)
            quotaTexKey = "quota_50";
        else if (quotaPct >= 0.25f)
            quotaTexKey = "quota_25";
        else
            quotaTexKey = "quota_empty";

        auto texQuota = _assets->get<graphics::Texture>(quotaTexKey);
        if (texQuota) {
            float iconY = quotaRowYs + (iconRowHs - iconSize) * 0.5f;
            _batch->setColor(Color4::WHITE);
            _batch->draw(texQuota, Rect(containerX + hudSize(8.0f), iconY, iconSize, iconSize));
        }

        std::string quotaStr = std::to_string(_depositedValue) + "/" + std::to_string(_quota);
        auto quotaLabel = TextLayout::allocWithText(quotaStr, _uiFont);
        quotaLabel->layout();
        float quotaTextH = quotaLabel->getBounds().size.height;
        float quotaTextW = quotaLabel->getBounds().size.width;
        float quotaTextX = containerX + hudSize(8.0f) + textOffsetX + (hudSize(containerW) - hudSize(8.0f) - textOffsetX - hudSize(quotaTextW)) * 0.5f;
        float textY = quotaRowY_local + (iconRowH - quotaTextH) * 0.5f + 15.0f;
        Affine2 quotaTrans;
        quotaTrans.scale(1.0f / zoom).translate(quotaTextX, hudY(textY));
        _batch->setColor(Color4::BLACK);
        _batch->drawText(quotaLabel, quotaTrans);
    }
}

cugl::Rect GameScene::getInventorySlotBounds(size_t slotIndex) const {
    Size screenSize = this->getSize();
    float zoom = getActiveCameraZoom();
    float margin = 16;

    constexpr size_t kSlotCount = 5;
    // Scale 112x112 box to a specific size (tbd)
    float boxSize = 96.0f;
    float boxGap = 8.0f;
    float totalW = kSlotCount * boxSize + (kSlotCount - 1) * boxGap; // full inventory bar width

    // Center the row in the right half of the screen
    float startX = (screenSize.width * 0.7f) - (totalW * 0.5f);
    float topY = screenSize.height - margin - boxSize;

    return Rect((startX + slotIndex * (boxSize + boxGap)) / zoom, topY / zoom, boxSize / zoom, boxSize / zoom);
}

cugl::Rect GameScene::getHelpButtonBounds() const {
    float zoom = getActiveCameraZoom();
    float margin = 16.0f;
    float buttonSize = 79.0f * 0.8f;
    Size screenSize = this->getSize();

    return Rect((screenSize.width - margin - buttonSize - 20.0f) / zoom,
        (screenSize.height - margin - buttonSize - 20.0f) / zoom,
        buttonSize / zoom,
        buttonSize / zoom);
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

void GameScene::initPhysics() {
    // initialize physics here, since this should be called post HANDSHAKE
    if (_networkCtrl->getPhysEnabled()) return;
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
}

void GameScene::initMap() {
    if (_networkCtrl->getRandSeed() == 0) return; // no randSeed
    if (_mapSeed == _networkCtrl->getRandSeed()) return; // already init

    

    _mapSeed = _networkCtrl->getRandSeed();
    CULog("set map seed to : %d", _mapSeed);

    if (_mapSeed != 0) {
        rebuildMapForRun();
    }
}

void GameScene::spawnPlayers() {
    if (!_networkCtrl->isHost()) return;
    auto players = _networkCtrl->getPlayers();
    for (auto& p : players) {
        CULog("i am making a player with the following id %s", p.c_str());
        auto params = _playerFact->serializeParams(
            _map->getPlayerSpawn() / PHYSICS_SCALE,
            p,
            _networkCtrl->getPlayerCharacter(p)
        );
        _networkCtrl->getPhysController()
            ->addSharedObstacle(_playerFactId, params);
    }

    if (!_addedLWalls) {
        CULog("adding lower walls in initmap");
        _map->getLowerWallNode()->setPriority(1.0f);
        _worldnode->addChild(_map->getLowerWallNode());
        _addedLWalls = true;
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
            CULog("I am trying to get a player using this id %s", _networkCtrl->getPID().c_str());
            _player = _playerFact->getPlayerByPID(_networkCtrl->getPID());

            _networkCtrl->initGameStuff(_itemFact, _playerFact, _enemyFact, _itemFactId,
                _player, _input, _depositPosition, _depositRadius, _world->getBounds().getMaxY());

            CULog("GameScene setActive: hasNetwork=%d isHost=%d",
                _networkCtrl->hasNetwork(), _networkCtrl->isHost());
        }
        else
        {
            hideInventoryCard();
            hideTutorial();
            _input.deactivate();
        }
    }
}

void GameScene::linkSceneToObs(const std::shared_ptr<physics2::Obstacle>& obj,
    const std::shared_ptr<scene2::SceneNode>& node) {

    if (node->getParent() != nullptr) {
        return;
    }
    node->setPosition(obj->getPosition() * PHYSICS_SCALE);
    _worldnode->addChild(node);
    if (auto item = std::dynamic_pointer_cast<ItemObstacle>(obj)) {
        _itemNodes.push_back({ item, node });
    }

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        if (auto p = dynamic_pointer_cast<PlayerObstacle>(obj)) {
            // Add player to set
            if (!_playerFact->current.contains(p)) {
                _playerFact->addCurrent(p);
            }

            std::shared_ptr<PlayerNode> p_node = std::dynamic_pointer_cast<PlayerNode>(node);
            obj->setListener([=, this](physics2::Obstacle* obs) {
                _playerFact->linkFunc(p, p_node, _world->getBounds().getMaxY(), _animations);
                });

            // Add animation
            _animations->addNode(p_node);
        }
        else if (auto e = dynamic_pointer_cast<EnemyObstacle>(obj)) {
            // Add enemy to set
            if (!_enemyFact->current.contains(e)) {
                _enemyFact->addEnemy(e);
            }

            std::shared_ptr<EnemyNode> e_node = std::dynamic_pointer_cast<EnemyNode>(node);
            _enemyHealthForSound[e.get()] = e->getHealth();
            obj->setListener([=, this](physics2::Obstacle* obs) {
                bool wasFrozen = e->isFroze();
                int oldHealth = _enemyHealthForSound[e.get()];
                if (e->isAttacking() && !e_node->isAttacking()) {
                    // play attack audio when an enemy enters attack state
                    playEnemyAttackSound(e->TYPE);
                }
                _enemyFact->linkFunc(e, e_node, _networkCtrl->isHost(), _world->getBounds().getMaxY(), _animations);
                if (e->getHealth() < oldHealth) {
                    // play enemy damage when health actually drops
                    playUniqueSound(enemyDamageSoundKey(e->TYPE), 1.0f);
                }
                _enemyHealthForSound[e.get()] = e->getHealth();
                bool unFrozen = wasFrozen && !e->isFroze();
                if (unFrozen && !e->isDead()) {
                    if (_player->getPosition().distance(e->getPosition()) <= 12.0f) {
                        // play unfreeze when the enemy leaves frozen state
                        playUniqueSound("hit_unfreeze", 1.0f);
                    }
                }
                });

            // Add animation
            _animations->addNode(e_node);
        }
        else if (auto s = dynamic_pointer_cast<SpellObstacle>(obj)) {
            std::shared_ptr<SpellNode> s_node = std::dynamic_pointer_cast<SpellNode>(node);
            obj->setListener([=, this](physics2::Obstacle* obs) {
                _spellFact->linkFunc(s, s_node, _world->getBounds().getMaxY(), _animations);
                });

            // Add animation
            _animations->addNode(s_node);
        }
        else {
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

void GameScene::initCombos() {
    _spellCombos.insert({
        {{SpellType::FIREBALL, SpellType::FIREBALL}, SpellType::COMBO_FIREBALL},
        {{SpellType::FREEZE, SpellType::FREEZE}, SpellType::COMBO_FREEZE},
        {{SpellType::FREEZE, SpellType::EARTH}, SpellType::COMBO_EFREEZE},
        {{SpellType::FIREBALL, SpellType::HEAL}, SpellType::BUFF},
        {{SpellType::EARTH, SpellType::HEAL}, SpellType::SHIELD},
        {{SpellType::FIREBALL, SpellType::WATER}, SpellType::STEAM},
        {{SpellType::FIREBALL, SpellType::FREEZE}, SpellType::WATER}
        });
}

std::pair<SpellType, string> GameScene::getCombo() {
    SpellType spell = _player->getSpell();
    string cid = _player->getPID();

    for (auto p : _playerFact->current) {
        if (_player->getPosition().distance(p->getPosition()) < COMBO_RADIUS && p->getPID() != cid) {
            auto it = _spellCombos.find({ spell, p->getSpell() });
            if (it != _spellCombos.end()) {
                return { it->second, p->getPID() };
            }

            it = _spellCombos.find({ p->getSpell(), spell });
            if (it != _spellCombos.end()) {
                return { it->second, p->getPID() };
            }
        }
    }

    // No combo
    return { SpellType::NONE, "" };
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
        contact->SetEnabled(false);
        return;
    }

    SpellObstacle* spell = nullptr;
    SpellObstacle* spell2 = nullptr;
    PlayerObstacle* player = nullptr;
    PlayerObstacle* player2 = nullptr;
    EnemyObstacle* enemy = nullptr;
    cugl::physics2::PolygonObstacle* wall = nullptr;
    ItemObstacle* item = nullptr;
    cugl::physics2::BoxObstacle* deposit = nullptr;

    // Get types of the 2 obstacles; only care about collisions between different types of obstacles
    if (auto s = dynamic_cast<SpellObstacle*>(obs1)) {
        spell = s;
    }
    else if (auto p = dynamic_cast<PlayerObstacle*>(obs1)) {
        player = p;
    }
    else if (auto e = dynamic_cast<EnemyObstacle*>(obs1)) {
        enemy = e;
    }
    else if (auto w = dynamic_cast<ItemObstacle*>(obs1)) {
        item = w;
    }
    else if (auto w = dynamic_cast<cugl::physics2::PolygonObstacle*>(obs1)) {
        wall = w;
    }
    else if (auto d = dynamic_cast<cugl::physics2::BoxObstacle*>(obs1)) {
        deposit = d;
    }

    if (auto s = dynamic_cast<SpellObstacle*>(obs2)) {
        if (!spell) {
            spell = s;
        }
        else {
            spell2 = s;
        }
    }
    else if (auto p = dynamic_cast<PlayerObstacle*>(obs2)) {
        if (player) {
            player2 = p;
        }
        else {
            player = p;
        }
    }
    else if (auto e = dynamic_cast<EnemyObstacle*>(obs2)) {
        enemy = e;
    }
    else if (auto w = dynamic_cast<ItemObstacle*>(obs2)) {
        item = w;
    }
    else if (auto w = dynamic_cast<cugl::physics2::PolygonObstacle*>(obs2)) {
        //CULog("i am a wall");
        wall = w;
    }
    else if (auto d = dynamic_cast<cugl::physics2::BoxObstacle*>(obs2)) {
        deposit = d;
    }

    auto damagePlayer = [this](PlayerObstacle* p, float damage) {
        bool hadShield = p->hasShield();
        p->damage(damage);
        if (hadShield) {
            playUniqueSound(p->hasShield() && p->getShieldCount() > 0 ? "hit_shield" : "hit_shatter_shield", 1.0f);
        }
        };
    auto damageEnemy = [](EnemyObstacle* e, float damage) {
        e->setHealth(e->getHealth() - damage);
        };

    // Checking for collisions
    if (spell && spell2) {
        //CULog("spell and spell");
        contact->SetEnabled(false);
    }
    else if (player && spell) {
        //CULog("spell and player");
        contact->SetEnabled(false);

        // If spell is in end animation, do not do any calculations
        if (spell->shouldRemove()) {
            return;
        }
        if (player->getPID() != _player->getPID() && spell->TYPE != SpellType::HEAL) {
            return;
        }
        if (spell->getCasterUID() == player->getPID()) return;
        int nh = player->getHealth() + HEAL_AMT;
        // spell hit feedback is played here when the effect changes player state
        switch (spell->TYPE) {
        case SpellType::HEAL:
            if (player->getHealth() <= 0) {
                player->setHealth(1);
                spell->setRemove();
                auto caster = _playerFact->getPlayerByPID(spell->getCasterUID());
                if (caster) {
                    caster->consumeItem(ItemTypeClass::ItemType::BOOK_HEAL);
                }
                break;
            }
            player->setHealth(nh);
            spell->setRemove();
            break;
        case SpellType::FREEZE:
            player->setFrozeTimer(90);
            player->setFrozen(true);
            spell->setRemove();
            playUniqueSound("hit_freeze", 1.0f);
            damagePlayer(player, 1);
            break;
        case SpellType::COMBO_FREEZE:
            player->setFrozeTimer(180);
            player->setFrozen(true);
            playUniqueSound("hit_freeze", 1.0f);
            damagePlayer(player, 1);
            spell->setRemove();
            break;
        case SpellType::COMBO_EFREEZE:
            player->setFrozeTimer(360);
            player->setFrozen(true);
            playUniqueSound("hit_freeze", 1.0f);
            spell->setRemove();
            break;
        case SpellType::EARTH:
            player->setMudTimer(240);
            player->setMud(true);
            spell->setRemove();
            damagePlayer(player, 1);
            break;
        case SpellType::BUFF:
            player->setBuffTimer(3600);
            player->setBuff(true);
            playUniqueSound("hit_buff", 1.0f);
            spell->setRemove();
            break;
        case SpellType::WATER:
            {
            if(player -> getWater()){
                break;
            }
            Vec2 pushDir = player->getPosition() - spell->getPosition();
            pushDir.normalize();
            player->setWater(true, pushDir);
            player -> setWaterTimer(60);
            }
            break;
        case SpellType::MUD_WATER:
            {
            if (player -> getWater()){
                break;
            }
            Vec2 pushDir = player->getPosition() - spell->getPosition();
            pushDir.normalize();
            player->setWater(true, pushDir);
            player -> setWaterTimer(90);
            player -> setMud(true);
            player -> setMudTimer(240);
            }
            break;
        case SpellType::SHIELD:
            player->setShield(true);
            player->setShieldCount(3);
            playUniqueSound("hit_shield", 1.0f);
            spell->setRemove();
            break;
        case SpellType::FIREBALL:
            damagePlayer(player, 2);
            spell->setRemove();
            break;
        case SpellType::COMBO_FIREBALL:
            damagePlayer(player, 4);
            spell->setRemove();
            break;
        case SpellType::DEATH:
            player->damage(500);
            break;
        case SpellType::OCTO_ATTACK:
            if (!player->isHurt()) {
                CULog("hit by octo attack!!");
                player->setWater(true, (player->getPosition() - spell->getPosition()).getNormalization());
                player->setWaterTimer(10);
                player->damage(spell->getDamage());
            }
            break;
        default:
            CULog("A None Type Spell Should Not be Colliding With Anything");
        }
    }
    else if (player && enemy) {
        if (enemy -> TYPE == EnemyType::STATUE){
            contact->SetEnabled(false);
            return;
        }
        contact->SetEnabled(true);

        if (player->getHealth() <= 0 || enemy->isDead() || player->getPID() != _player->getPID() || enemy->isFroze()) {
            return;
        }

        float knockbackSpeed = 10.0f;
        b2Vec2 vel = enemy->getBody()->GetLinearVelocity();
        b2Vec2 dir = vel;
        float len = dir.Length();
        if (len > 0.001f) {
            dir.x /= len;
            dir.y /= len;
        }
        enemy->getBody()->SetLinearVelocity(b2Vec2(-dir.x * knockbackSpeed, -dir.y * knockbackSpeed));
        //if (!enemy->canAttack()) {
        //    return;
        //}

        enemy->setAttackTimer(90);

        //CULog("enemy and player");
        if (enemy->TYPE == EnemyType::GHOST) {
            // ghosts attack by contact damage instead of an attack animation
            playEnemyAttackSound(enemy->TYPE);
        }
        damagePlayer(player, 1);
        return;
    }
    else if (enemy && spell) {
        contact->SetEnabled(false);
        if (enemy -> TYPE == EnemyType::STATUE){
            return;
        }

        if (!_networkCtrl->isHost() || spell->shouldRemove() || spell->getCasterUID() == ENEMY_ID) {
            return;
        }
        // spell hit feedback is played here when the effect changes enemy state
        switch (spell->TYPE) {
        case SpellType::WATER:
        {
            Vec2 pushDir = enemy->getPosition() - spell->getPosition();
            pushDir.normalize();
            enemy->setWaterPush(pushDir, spell -> getWeak() ? 30 : 60);
            break;
        }
        case SpellType::FREEZE:
            enemy->setFrozen(true);
            enemy->setFrozenTimer(spell -> getWeak() ? 120 : 240);
            playUniqueSound("hit_freeze", 1.0f);
            damageEnemy(enemy, spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::COMBO_FREEZE:
            enemy->setFrozen(true);
            enemy->setFrozenTimer(600);
            playUniqueSound("hit_freeze", 1.0f);
            damageEnemy(enemy, spell->getDamage());
            break;
        case SpellType::COMBO_EFREEZE:
            enemy->setFrozen(true);
            enemy->setFrozenTimer(-1);
            playUniqueSound("hit_freeze", 1.0f);
            damageEnemy(enemy, spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::EARTH:
            enemy->setMud(true);
            enemy->setMudTimer(spell -> getWeak() ? 150 : 300);
            damageEnemy(enemy, spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::MUD_WATER:
        {
            Vec2 pushDir = enemy->getPosition() - spell->getPosition();
            pushDir.normalize();
            enemy->setWaterPush(pushDir, 60);
            enemy->setMud(true);
            enemy->setMudTimer(600);
            break;
        }
        case SpellType::FIREBALL:
            damageEnemy(enemy, spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::COMBO_FIREBALL:
            damageEnemy(enemy, spell->getDamage());
            break;
        case SpellType::DEATH:
            damageEnemy(enemy, spell->getDamage());
            break;
        default:
            spell->setRemove();
        }
    }
    else if (wall && player) {
        contact->SetEnabled(true);
        return;
    }
    else if (wall && spell) {
        contact->SetEnabled(false);
        if (spell->VISIBLE) {
            spell->setRemove();
        }
        return;
    }
    else if (wall && enemy) {
        if (enemy->TYPE == EnemyType::GHOST) {
            contact->SetEnabled(false);
            return;
        }
    }
    else if (player && player2) {
        contact->SetEnabled(false);
        return;
    }
    else if (player && item) {
        contact->SetEnabled(false);
        _currOverlappedItem = item->getID();
        return;
    }
    else if (enemy && item) {
        contact->SetEnabled(false);
        return;
    }
    else if (spell && item) {
        contact->SetEnabled(false);
    }
    else if (deposit && player) {
        contact->SetEnabled(true);
        CULog("at deposit");
        _canDeposit = true;
        return;
    }
    else {
        contact->SetEnabled(false);
    }

}

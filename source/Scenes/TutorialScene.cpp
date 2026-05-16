//
//  TutorialScene.cpp
//  hexcapade
//
//  Created by Serena Duncan on 5/8/26.
//
// Authors: Serena Duncan, Alexander Jankowich, Kaz Tam, Frank Xie, Galiba Anjum


#include "TutorialScene.h"

using namespace cugl;
using namespace cugl::audio;
using namespace std;
using namespace cugl::graphics;

namespace {
    void restartTutorialSound(const std::shared_ptr<AssetManager>& assets, const std::string& key) {
        auto sound = assets->get<Sound>(key);
        AudioEngine::get()->play(key, sound, false, 1.0f, true);
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
void TutorialScene::beginContact(b2Contact* contact) {}

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
bool TutorialScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    // Initialize the scene to a locked width
    if (assets == nullptr || !cugl::scene2::Scene2::initWithHint(Size(0, SCENE_HEIGHT)))
    {
        return false;
    }

    _addedLWalls = false;
    _assets = assets;
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
    _inputDisplay = InputDisplay(&_input, _constants->get("input"));

    // Initialize spell combo information
    initCombos();


    // Initialize inventory system
    _depositedValue = 0;

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

    // this resets pickup prompt state
    _canDeposit = false;
    _depositRadius = 42.0f;
    _depositPosition = Vec2::ZERO;

    // Create all the factories
    _spellFact = SpellFactory::alloc(_assets);
    _playerFact = PlayerFactory::alloc(_assets);
    _enemyFact = EnemyFactory::alloc(_assets);
    _itemFact = ItemFactory::alloc(_assets);

    // Create action timeline
    _animations = Animator::alloc();

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
    

    // init map and network
    _map = std::make_shared<StaticMap>();
    _networkCtrl = std::make_shared<NetworkController>();
    _networkCtrl->init(_assets);
     while (!_networkCtrl->connectHost());
    CULog("tutorial scene connecting host");
    
    _itemInfo = ItemTypeClass(_assets);

    
    setActive(false);
    return true;
}
void TutorialScene::reset() {
    CULog("tutorial scene reset called");
    _networkCtrl->disconnect();
    _networkCtrl->resetStatus();
    hideTutorial();
    hideInventoryCard();
    _pendingTutorialTex = nullptr;
    _world->clear(true); // reset box2d + phys world
    // is this correct...?
    _worldnode->removeAllChildren();
    removeAllChildren();
    _input.reset();
    _input.dispose();
    
    _inputDisplay.reset();

    _player = nullptr;
    _enemyFact->reset();
    _itemFact->reset();
    _spellFact->reset();
    _playerFact->reset();

    _addedLWalls = false;
    _tutorialEnded = false;
    _depositedValue = 0;
    // Match startup behavior on replay: reset clears active enemies and restarts
    // the host spawn timer instead of forcing one immediate spawn.

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
    _tutorialTimer = 0;
    //_networkCtrl->connectHost(); // prepare to start again?
    Scene::reset();
}
/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void TutorialScene::dispose()
{
    // NOTHING TO DO THIS TIME
    hideInventoryCard();
    _inputDisplay.dispose();
    _map = nullptr;
    _assets->dispose();
    _assets = nullptr;
}

#pragma mark -
#pragma mark Inventory

void TutorialScene::removeItemNode(const std::shared_ptr<ItemObstacle>& item) {
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

void TutorialScene::showInventoryCard(size_t slotIndex) {
    hideInventoryCard();

    auto cardTex = _itemInfo.getCard(_player->getInventory()[slotIndex].itemType);

    _inventoryCardNode = scene2::PolygonNode::allocWithTexture(cardTex);
    _inventoryCardNode->setAnchor(Vec2::ANCHOR_CENTER);
    float zoom = getActiveCameraZoom();
    Vec2 offset = getHUDOffset();
    _inventoryCardNode->setScale(1.0f / zoom);
    _inventoryCardNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
        offset.y + this->getSize().height * 0.5f / zoom));
    _inventoryCardNode->setPriority(1.0f);
    addChild(_inventoryCardNode);
    restartTutorialSound(_assets, "general_item_card");
}

void TutorialScene::hideInventoryCard() {
    if (_inventoryCardNode) {
        // _inventoryCardNode->removeFromParent();
        removeChild(_inventoryCardNode);
        _inventoryCardNode = nullptr;
    }
}

void TutorialScene::showTutorial(std::shared_ptr<graphics::Texture> tex) {
    hideTutorial();
    _tutorialNode = scene2::PolygonNode::allocWithTexture(tex);
    _tutorialNode -> setAnchor(Vec2::ANCHOR_CENTER);
    float zoom = getActiveCameraZoom();
    Vec2 offset = getHUDOffset();
    _tutorialNode->setScale(1.0f / zoom);
    _tutorialNode->setPosition(Vec2(offset.x + this->getSize().width * 0.5f / zoom,
        offset.y + this->getSize().height * 0.5f / zoom));
    _tutorialNode->setPriority(1.0f);
    addChild(_tutorialNode);
    restartTutorialSound(_assets, "general_item_card");
}

void TutorialScene::hideTutorial() {
    if (_tutorialNode) {
        removeChild(_tutorialNode);
        _tutorialNode = nullptr;
    }
}

void TutorialScene::updateCameraFollow() {
    std::shared_ptr<OrthographicCamera> orthographicCamera = std::dynamic_pointer_cast<OrthographicCamera>(getCamera());
    float zoom = getActiveCameraZoom();
    float halfW = getSize().width / (2.0f * zoom);
    float halfH = getSize().height / (2.0f * zoom);

    const Rect& world = Rect(0, 0, _map->getData()->getWidth() * TILE_SIZE, _map->getData()->getHeight() * TILE_SIZE);
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

float TutorialScene::getActiveCameraZoom() const {
    return std::max(std::dynamic_pointer_cast<OrthographicCamera>(getCamera())->getZoom(), 0.02f);
}

Vec2 TutorialScene::getHUDOffset() const {
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
void TutorialScene::preUpdate(float dt)
{
    if (_depositedValue >= _quota) {
        _tutorialEnded = true;
    }
#pragma mark Tutorial Stuff I Am Putting Here for Simplicity
    
    _tutorialTimer -= 1;
    if(_playerFact -> getPlayerByPID("bot0") -> getSpell() == SpellType::NONE){
        _playerFact -> getPlayerByPID("bot0") -> setSpell(SpellType::FREEZE);
    }
    if(_playerFact -> getPlayerByPID("bot1") -> getSpell() == SpellType::NONE){
        _playerFact -> getPlayerByPID("bot1") -> setSpell(SpellType::FIREBALL);
    }
    
#pragma mark Input Updates and Player Initialiation
    // @ alex i cleared the rest of the checks since we are always host
    
    //We have to acquire ourselves before we attempt to move
    //it is a little weird to do this every frame but this allows host to acquire themselves
    _networkCtrl->getPhysController()->acquireObs(_player, 0);

    _input.syncGesturesToInventory(_player->getInventory());

    _input.update(dt);
    
    if (_pendingTutorialTex) {
        CULog("i am trying to display");
        showTutorial(_pendingTutorialTex);
        _pendingTutorialTex = nullptr;
        _input.resolvedCardDismissTap();
        _tutorialTimer = 90;
    }

    //CULog("current player position: %f, %f", _player -> getPosition().x, _player -> getPosition().y);
    if (_input.pressedReset()) {
        reset();
        _input.resolvedReset();
        return;
    }
#pragma mark Enemy Spawning Logic, Pathfinding, and Removal
    auto toRemove = _enemyFact->getRemoved();
    for (auto en : toRemove) {
        _networkCtrl->getPhysController()->removeSharedObstacle(en);
    }

    auto itRemove = _itemFact->update();
    for (auto item : itRemove) {
        if (!item->isRemoved()) {
            _networkCtrl->getPhysController()->removeSharedObstacle(item);
        }
    }

    // Remove spells that have gone out of range
    auto spellRemove = _spellFact->update();
    for (auto spell : spellRemove) {
        spell->markRemoved(true);
        _networkCtrl->getPhysController()->removeSharedObstacle(spell);
        //CULog("Remove spell: %d", spell->isRemoved());
    }

    cugl::Vec2 playerScreenPos = _player->getPosition() * PHYSICS_SCALE;

#pragma mark Inventory Card, Tutorial Card, and Camera Logic
    updateCameraFollow();
    Vec2 offset = getHUDOffset();
    _input.setCameraOffset(offset);
    _inputDisplay.setUIOffset(offset);
    _inputDisplay.setUIZoom(getActiveCameraZoom());
    _inputDisplay.update(playerScreenPos);

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

            _input.resolvedInventoryRelease();
            _input.resolvedCardDismissTap();
            return;
        }
    }
    if (_inventoryCardNode) {
        if (_input.cardDismissTap()) {
            //CULog("click detected, card closed");
            restartTutorialSound(_assets, "general_item_card_close");
            hideInventoryCard();
            _pendingInventoryTapIndex = -1;
        }
    }
    else if (_tutorialNode && _tutorialTimer <= 0) {
        if (_input.cardDismissTap()) {
            restartTutorialSound(_assets, "general_item_card_close");
            hideTutorial();
        }
    }
    _input.resolvedCardDismissTap();

    // Update animations
    _animations->update(dt);

    if (_tutorialNode) {
        // Skip updates if tutorial is showing
        _player->setLinearVelocity(Vec2::ZERO);
        return;
    }

#pragma mark Update Player
    //CULog("do player updates");
    // Player movement
    cugl::Vec2 input = _input.getMove();
    if (_input.showAim()) {
        _player->setAimSlowTimer(_player->getAimSlowTimer() - dt);
    }
    else {
        _player->setAimSlowTimer(2.0f);
    }
    cugl::Vec2 velocity = input * _player->getSpeed();

    if (_player->isFrozen() || _player->isHurt()) {
        //_player->setVelocity(Vec2::ZERO);
    }
    else {
        _player->setVelocity(velocity);
    }

#pragma mark Deposit handling and Inventory card display
    bool depositRequested = false;
    int depositIndex = -1;
    //_canDeposit = playerScreenPos.distance(_depositPosition) <= _depositRadius;

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
            if (_canDeposit) {
                int deposited = _player->depositItem((size_t)depositIndex);
                if (deposited > 0) {
                    _inventoryValue = _player->getInventoryVal();
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
                _networkCtrl->pushOutEvent(DepositEvent::allocDepositEvent(_player->getPID(), 0, true, depositIndex));
            }
        }
    }

    // DESKTOP DEPOSIT: F FOR ALL!
    if (depositRequested && _canDeposit) {
        if (_player && _player->hasInventory()) {
            if (_networkCtrl->isHost()) {
                int deposited = _player->depositItems();
                if (deposited > 0) {
                    _inventoryValue = _player->getInventoryVal();
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

    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero()) {
        angle = _player->getDirection();
    }

    if (_input.shouldReleaseSpell() && !_player->blocksSpellcasting()) {
        //CULog("Release spell--------------------------------------------------------");

        //CULog("Spell type: %d", _player->getSpell());
        if (_player->getSpell() == SpellType::NONE) {
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
            // removed stuff for spells not in tutorial
            int damageBonus = _player->getDamageBonus();
            auto params = _spellFact->serializeParams(playerScreenPos / PHYSICS_SCALE, angle, t, _player->getPID(), damageBonus, _player->getBuff());
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
            _networkCtrl->getPhysController()->acquireObs(pair.first, 0);

            _input.resolveSpell();
            _player->resolveSpell();
            _player->setReleasedSpell(true);
            _networkCtrl->pushOutEvent(
                SetSpellEvent::allocSetSpellEvent(_networkCtrl->getPID(), SpellType::NONE));
        }
    }
#pragma mark Pickup Logic
    if (_input.pressedPickup() && _currOverlappedItem >= 0) {
        CULog("trying to resolve pickup");
        auto item = _itemFact->getItem(_currOverlappedItem);
        // Checks if player can pick up anything so inventory doesn't overflow
        if (_player && _player->canAddItem() && item && !item->isRemoved()) {
            _networkCtrl->pushOutEvent(
                PickupEvent::allocPickupEvent(_player->getPID(), _currOverlappedItem)
            );
        }
    }
    if (_input.pressedPickup()) {
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
void TutorialScene::fixedUpdate(Uint64 step)
{
    // First process incoming events
    _networkCtrl->updateNet();

    std::tuple<int, int> values = _networkCtrl->customEvents();
    _inventoryValue = std::get<0>(values);
    _depositedValue += std::get<1>(values);

    _currOverlappedItem = -1;
    _canDeposit = false;
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
void TutorialScene::postUpdate(float remain)
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
void TutorialScene::render()
{
    Scene2::render();
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

    _inputDisplay.draw(_batch);
    if (_inventoryCardNode) {
        _inventoryCardNode->render(_batch);
    }
    if (_tutorialNode) {
        _tutorialNode->render(_batch);
    }
    _batch->end();
}

void TutorialScene::renderHUD()
{
    Size screenSize = this->getSize();
    Vec2 hudOffset = getHUDOffset();
    float zoom = getActiveCameraZoom();

    float barW = 320;
    float barH = 50;
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

    Color4 boxColor(30, 30, 30, 255);

    if (_font)
    {
        _batch->setColor(Color4::WHITE);

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

        auto texContainer = _assets->get<graphics::Texture>("health_bar");
        auto texFull = _assets->get<graphics::Texture>("health_full");
        auto texHalf = _assets->get<graphics::Texture>("health_half");
        auto texEmpty = _assets->get<graphics::Texture>("health_empty");

        // Scale container with 269x79 aspect ratio (only need to change container cause actualy heart size is based on container)
        float containerH = hudSize(79.0f * 0.8f);
        float containerW = hudSize(269.0f * 0.8f);
        float containerX = hudX(margin + 25.0f); //extra offset because health gets clipped on iphone corner
        float containerY = hudY(screenSize.height - 79.0f * 0.8f - margin);
        float quotaY = screenSize.height - 79.0f * 0.8f - margin - barH - margin;

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
        //drawBox(quotaX + hudOffset.x, quotaY + hudOffset.y, barW, barH, boxColor);

        std::shared_ptr<TextLayout> quotaLabel = TextLayout::allocWithText(
            "Current Deposit: " + std::to_string(_depositedValue) + " / " + std::to_string(_quota), _font);
        quotaLabel->layout();
        Affine2 quotaTrans;
        quotaTrans.scale(1.0f / zoom).translate(hudX(quotaX + 10), hudY(quotaY + 12));
        _batch->setColor(Color4::BLACK);
        _batch->drawText(quotaLabel, quotaTrans);
    }
}

cugl::Rect TutorialScene::getInventorySlotBounds(size_t slotIndex) const {
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

int TutorialScene::getInventorySlotIndexAt(const cugl::Vec2& tapPos) const {
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

void TutorialScene::initPhysics() {
    // initializes physics and adds walls
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
    auto players = _networkCtrl->getPlayers();
    for (auto& p : players) {
        CULog("i am making a player with the following id %s", p.c_str());
        auto params = _playerFact->serializeParams(
            _map->getData()->getPlayerSpawn() / PHYSICS_SCALE,
            p
        );
        _networkCtrl->getPhysController()
            ->addSharedObstacle(_playerFactId, params);
    }
    int counter = 0;
    for(auto& p : _map -> getData() -> getBotSpawnPoints()){
        std::string bot_id = "bot" + std::to_string(counter);
        _networkCtrl -> setPlayerName(bot_id, bot_id == "bot0" ? "Duck" : "Cat");
        _networkCtrl -> setPlayerCharacter(bot_id, bot_id == "bot0" ? "player_duck" : "player_cat");
        
        auto params = _playerFact->serializeParams(
            p.location / PHYSICS_SCALE,
            bot_id,
            _networkCtrl -> getPlayerCharacter(bot_id)
        );
        _networkCtrl -> getPhysController()
            -> addSharedObstacle(_playerFactId, params);
        counter += 1;
        
    }
    _map->getLowerWallNode()->setPriority(1.0f);
    _worldnode->addChild(_map->getLowerWallNode());
    _addedLWalls = true;
}

void TutorialScene::spawnFromMap() {
    _quota = _map->getQuota();
    
    for (auto e : _map->getData()->getEnemSpawnPoints()) {
        auto params = _enemyFact->serializeParams((EnemyType) e.enem, e.location / PHYSICS_SCALE);
        auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
        _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
    }
    
    for (auto e : _map->getData()->getItemSpawnPoints()) {
        auto params = _itemFact->serializeParams(e.location / PHYSICS_SCALE,
            _itemInfo.getConsts((ItemTypeClass::ItemType) e.item)->getInt("value"), _world->getBounds().getMaxY(),
            (ItemTypeClass::ItemType) e.item);
        auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_itemFactId, params);
        _networkCtrl->getPhysController()->acquireObs(pair.first, 0);
    }
    
    _depositPosition = _map->getData()->getDepositSpawn();
    auto sprite = cugl::scene2::PolygonNode::allocWithTexture(_assets->get<graphics::Texture>("deposit"));
    Vec2 pos = _map->getData()->getDepositSpawn(); // bottom left corner of deposit
    //CULog("X: %f, Y %f", x,y);
    sprite->setPriority(99.0f);
    sprite->setPosition(Vec2(pos));
    sprite->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    float maxHeight = _world->getBounds().getMaxY();
    sprite->setPriority((maxHeight - _depositPosition.y / PHYSICS_SCALE) / maxHeight * 2);
    _worldnode->addChild(sprite);
    
    auto obs = physics2::BoxObstacle::alloc(pos/PHYSICS_SCALE, _assets->get<graphics::Texture>("deposit")->getSize()/PHYSICS_SCALE);
    //obs->setPosition(0,0);
    obs->setDensity(1.0);
    obs -> setEnabled(true);
    obs->setBodyType(b2_staticBody);
    _world->addObstacle(obs);
    
}

void TutorialScene::setActive(bool value)
{
    static bool inProgress = false;

    if (isActive() != value)
    {
        if (value)
        {
            if (!inProgress) {
                inProgress = true;
                _worldnode = cugl::scene2::OrderedNode::allocWithOrder(cugl::scene2::OrderedNode::Order::ASCEND);
                _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                _worldnode->setPosition(Vec2::ZERO);
                addChild(_worldnode);

                auto n = _map->generateStaticMap(_assets, _world);
                n->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                n->setPosition(Vec2::ZERO);
                n->setPriority(0.0f);
                _worldnode->addChild(n);

                _itemInfo = ItemTypeClass(_assets);
            }

            if (!_networkCtrl->getShortUID()) {
                CULog("waiting for shortUID");
                return;
            }

            initPhysics();
            spawnFromMap();
            _input.activate();
            
            CULog("I am trying to get a player using this id %s", _networkCtrl->getPID().c_str());
            _player = _playerFact->getPlayerByPID(_networkCtrl->getPID());

            _networkCtrl->initGameStuff(_itemFact, _playerFact, _enemyFact, _itemFactId,
                _player, _input, _depositPosition, _depositRadius, _world->getBounds().getMaxY());
            _player->consumeItem(ItemTypeClass::ItemType::BOOK_EARTH);
            CULog("tut setActive: hasNetwork=%d isHost=%d",
                _networkCtrl->hasNetwork(), _networkCtrl->isHost());

            inProgress = false;
            Scene2::setActive(true);
        }
        else
        {
            hideInventoryCard();
            _input.deactivate();
            Scene2::setActive(false);
        }
    }
}

void TutorialScene::linkSceneToObs(const std::shared_ptr<physics2::Obstacle>& obj,
    const std::shared_ptr<scene2::SceneNode>& node) {

    if (node->getParent() != nullptr) {
        return;
    }
    node->setPosition(obj->getPosition() * PHYSICS_SCALE);
    _worldnode->addChild(node);
    if (auto item = std::dynamic_pointer_cast<ItemObstacle>(obj)) {
        _itemNodes.push_back({ item.get(), node });
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
            std::shared_ptr<EnemyNode> e_node = std::dynamic_pointer_cast<EnemyNode>(node);
            obj->setListener([=, this](physics2::Obstacle* obs) {
                _enemyFact->linkFunc(e, e_node, true, _world->getBounds().getMaxY(), _animations);
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

void TutorialScene::initCombos() {
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

std::pair<SpellType, string> TutorialScene::getCombo() {
    SpellType spell = _player->getSpell();
    string cid = _player->getPID();

    for (auto p : _playerFact->current) {
        if (_player->getPosition().distance(p->getPosition()) < COMBO_RAD && p->getPID() != cid) {
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
 * @param  contact      The two bodies that collided
 * @param  oldManfold      The collision manifold before contact
 */
void TutorialScene::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();

    physics2::Obstacle* obs1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* obs2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    SpellObstacle* spell = nullptr;
    SpellObstacle* spell2 = nullptr;
    PlayerObstacle* player = nullptr;
    PlayerObstacle* player2 = nullptr;
    EnemyObstacle* enemy = nullptr;
    cugl::physics2::PolygonObstacle* wall = nullptr;
    ItemObstacle* item = nullptr;
    TutorialObstacle* tut = nullptr;
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
    }else if (auto o = dynamic_cast<TutorialObstacle*>(obs1)){
        tut = o;
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
    }else if (auto o = dynamic_cast<TutorialObstacle*>(obs2)) {
        tut = o;
    }
    else if (auto w = dynamic_cast<cugl::physics2::PolygonObstacle*>(obs2)) {
        //CULog("i am a wall");
        wall = w;
    }
    else if (auto d = dynamic_cast<cugl::physics2::BoxObstacle*>(obs2)) {
        deposit = d;
    }

    if (!tut && (obs1->isRemoved() || obs2->isRemoved())) {
        return;
    }

    // Checking for collisions
    if (spell && spell2) {
        //CULog("spell and spell");
        contact->SetEnabled(false);
    }
    else if (player && spell) {
        //CULog("spell and player");
        contact->SetEnabled(false);
    }
    else if (player && enemy) {
        // no attacks in tutorial
        contact->SetEnabled(false);
    }
    else if (enemy && spell) {
        //CULog("enemy and spell");
        contact->SetEnabled(false);
        if (spell->getCasterUID() == "bat") {
            return;
        }
        // If spell is in end animation, do not do any calculations
        if (spell->shouldRemove()) {
            return;
        }
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
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::COMBO_FREEZE:
            enemy->setFrozen(true);
            enemy->setFrozenTimer(600);
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            break;
        case SpellType::COMBO_EFREEZE:
            enemy->setFrozen(true);
            enemy->setFrozenTimer(-1);
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::EARTH:
            enemy->setMud(true);
            enemy->setMudTimer(spell -> getWeak() ? 150 : 300);
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
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
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            spell->setRemove();
            break;
        case SpellType::COMBO_FIREBALL:
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            break;
        case SpellType::DEATH:
            enemy->setHealth(enemy->getHealth() - spell->getDamage());
            break;
        default:
            spell->setRemove();
        }
    }
    else if (wall && player) {
        contact->SetEnabled(true);
        CULog("wall");
        return;
    }
    else if (wall && spell) {
        contact->SetEnabled(false);
        spell->setRemove();
        return;
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
    else if (player && tut) {
        CULog("tutorial box");
        if (!tut -> getSeen()){
            auto tex = _assets->get<graphics::Texture>(tut->getPng());
            if (tex) {
                _pendingTutorialTex = tex;
            } else {
                CULog("texture still null for key: %s", tut->getPng().c_str());
            }
            tut -> setSeen(true);
        }
        contact->SetEnabled(false);
    }
    else if (deposit && player) {
        contact->SetEnabled(true);
        CULog("at deposit");
        _canDeposit = true;
        return;
    }

}

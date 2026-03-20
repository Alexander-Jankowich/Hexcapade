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
#include <iostream>
#include <sstream>
#include <random> //itemssss

#include "GLGameScene.h"

using namespace cugl;
using namespace std;
using namespace cugl::graphics;

#pragma mark -
#pragma mark Level Layout

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** The ratio between the physics world and the screen. */
#define PHYSICS_SCALE 50

#define PI 3.14159265358979323

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
    else if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT)))
    {
        return false;
    }

    _assets = assets;

    // Get constants
    _constants = assets->get<JsonValue>("constants");
    _gestures = assets->get<JsonValue>("gestures");

    // Start up the input handler
    // This is the size of scene! We want to screenToWorld and worldToScreen
    // Factor out conversion with lambda gamescene -> input
    _input.init(_constants->get("input"), cugl::Rect(0, 0, getSize().width, getSize().height));
    _inputDisplay = make_shared<InputDisplay>(&_input, _constants->get("input"));

    lastEnemySpawn = 0;

    // Initialize inventory system
    _currInventoryScore = 0;
    _quota = 100;
    _playerWon = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    spells = SpellSet(_constants->get("spells"));
    spells.setTexture(assets->get<Texture>("spell"));
    _collisions.init(getSize());

    // this resets pickup prompt state
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";

    // this spawns one artifact and one spell for prototype startup
    spawnInitialItems();


    _spellFact = SpellFactory::alloc(_assets);
    _playerFact = PlayerFactory::alloc(_assets);
    _enemyFact = EnemyFactory::alloc(_assets);

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

    _world = physics2::distrib::NetWorld::alloc(Rect(0, 0, dimen.width, SCENE_HEIGHT), Vec2::ZERO);
    //_world->onBeginContact = [this](b2Contact* contact) {
    //    beginContact(contact);
    //    };
    _world->update(0.02f);
    
    
    _networkCtrl = NetworkController::alloc(_assets);
    _players = std::make_shared<PlayerSet>();
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    // NOTHING TO DO THIS TIME
    spells.dispose();
    _inputDisplay->dispose();
    _assets->dispose();
    _assets = nullptr;
}

void GameScene::reset()
{
    pair<shared_ptr<physics2::Obstacle>, shared_ptr<scene2::SceneNode>> player_pair =
    _playerFact -> createObstacle(Vec2(0,0),_networkCtrl -> getPID());
    _player = std::dynamic_pointer_cast<PlayerObstacle>(player_pair.first);
    _players -> addPlayer(std::dynamic_pointer_cast<Player>(player_pair.second));
    _playerLost = false;
    _playerWon = false;
    _currInventoryScore = 0;
    lastEnemySpawn = 0;

    spells.current.clear();
    spells.pending.clear();
    spawnInitialItems();
    
    if (_ishost) { // broadcast to all in room
        _networkCtrl->startGame();
    }
}

#pragma mark -
#pragma mark Inventory
// this has been bumped down to player model but keeping here just in case
// void GameScene::addItem(int value) {
//    _inventoryValue += value;
//    if (_inventoryValue >= _quota) {
//        _playerWon = true;
//    }
//}

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

    if (_input.pressedReset())
    {
        reset();
        _input.resolvedReset();
        return;
    }

    if (_playerLost || _playerWon)
    {
        return;
    }
    // spawn an enemy if host and need to spawn
    if (_ishost)
    {
        lastEnemySpawn += dt;
        if (lastEnemySpawn > 8)
        {
            lastEnemySpawn = 0;
            //_enemy->spawnEnemy(getSize() / 2, 4);
            auto params = _enemyFact->serializeParams(getSize() / 2, Vec2::ZERO, 4);
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
        }
    }

    // Player movement
    cugl::Vec2 input = _input.getMove();
    cugl::Vec2 velocity = input * _player->getSpeed();
    CULog("x: %f, y: %f", velocity.x, velocity.y);
    _player->setVelocity(velocity);
    _player->setAniState(_input.showAim() ? PlayerObstacle::AnimationState::CASTING : _player->getState());
    _player->update(dt);

    cugl::Vec2 playerScreenPos = _player->getPosition();

    updateItemPickupState();
    tryPickupItem();

    // Spellcasting
    if (_input.getGesture() != SpellType::NONE && _player->getSpell() == SpellType::NONE)
    {
        _player->setSpell(_input.getGesture());
    }

    bool sendSpell = false;
    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero())
    {
        angle = _player->getDirection();
    }
    if (_input.shouldReleaseSpell())
    {
        sendSpell = true;
        CULog("Release spell--------------------------------------------------------");
        // CULog("Pos: %.2f %.2f", playerScreenPos.x, playerScreenPos.y);
        if (_ishost)
        {
            //spells.spawnSpell(_player->getSpellId(), _player->getSpell(), playerScreenPos, angle);
            auto params = _spellFact->serializeParams(playerScreenPos, angle*5, _player->getSpell());
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_spellFactId, params);
        }
        _input.resolveSpell();
        _player->resolveSpell();
    }

    // spawn an enemy if host and need to spawn
    if (_ishost)
    {
        lastEnemySpawn += dt;
        if (lastEnemySpawn > 8)
        {
            lastEnemySpawn = 0;
            //_enemy->spawnEnemy(getSize() / 2, 4);
            auto params = _enemyFact->serializeParams(getSize() / 2, Vec2::ZERO, 4);
            auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_enemyFactId, params);
        }

        // Change to be all player positions
        _enemyFact->update({_player->getPosition()});
    }

    _inputDisplay->update(playerScreenPos);

    if (_player->getHealth() <= 0)
    {
        _playerLost = true;
    }
    if ((_playerLost || _playerWon) && _ishost)
    {
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
void GameScene::fixedUpdate(Uint64 step)
{
    // First process incoming events

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
    // DO NOT DO THIS IN YOUR FINAL GAME
    //_batch->setPerspective(getCamera()->getCombined());
    //_batch->begin();

    //Color4 original = _batch->getColor();

    //drawItems();

    //_players->draw(_batch);

    //_enemy->draw(_batch);

    //spells.draw(_batch);

    //renderHUD();

    //_inputDisplay->draw(_batch);

    //_batch->setColor(original);
    //_batch->end();

    Scene2::render();
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();
    renderHUD();
    _batch->end();
}

void GameScene::renderHUD()
{
    Size screenSize = getSize();

    float barW = 280;
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

    // Inventory box - top left
    float invX = margin;
    float invY = screenSize.height - barH - margin;
    drawBox(invX, invY, barW, barH, boxColor);

    // Quota box - top right
    float quotaX = screenSize.width - barW - margin;
    float quotaY = screenSize.height - barH - margin;
    drawBox(quotaX, quotaY, barW, barH, boxColor);

    // Health box - top center
    float healthW = 200;
    float healthX = screenSize.width / 2 - healthW / 2;
    float healthY = screenSize.height - barH - margin;
    drawBox(healthX, healthY, healthW, barH, boxColor);

    // Pickup prompt box - bottom right
    float pickupX = screenSize.width - barW - margin;
    float pickupY = margin;
    drawBox(pickupX, pickupY, barW, barH, boxColor);

    if (_font)
    {
        _batch->setColor(Color4::WHITE);

        std::shared_ptr<TextLayout> invLabel = TextLayout::allocWithText(
            "Inventory: $" + std::to_string(_player->getInventoryVal()), _font);
        invLabel->layout();
        Affine2 invTrans;
        invTrans.translate(invX + 10, invY + 12);
        _batch->drawText(invLabel, invTrans);

        std::shared_ptr<TextLayout> quotaLabel = TextLayout::allocWithText(
            "Quota: $" + std::to_string(_quota), _font);
        quotaLabel->layout();
        Affine2 quotaTrans;
        quotaTrans.translate(quotaX + 10, quotaY + 12);
        _batch->drawText(quotaLabel, quotaTrans);

        std::shared_ptr<TextLayout> healthLabel = TextLayout::allocWithText(
            "Health: " + std::to_string(_player->getHealth()), _font);
        healthLabel->layout();
        Affine2 healthTrans;
        healthTrans.translate(healthX + 10, healthY + 12);
        _batch->drawText(healthLabel, healthTrans);

        std::string pickupText = _canPickupItem ? _pickupPrompt : "move close to item";
        std::shared_ptr<TextLayout> pickupLabel = TextLayout::allocWithText(pickupText, _font);
        pickupLabel->layout();
        Affine2 pickupTrans;
        pickupTrans.translate(pickupX + 10, pickupY + 12);
        _batch->drawText(pickupLabel, pickupTrans);
    }
}

GameScene::ItemData GameScene::createRandomItem(bool forceArtifact, bool forceSpell)
{
    static std::random_device rd;
    static std::mt19937 rng(rd());

    std::uniform_real_distribution<float> xdist(80.0f, getSize().width - 80.0f);
    std::uniform_real_distribution<float> ydist(100.0f, getSize().height - 100.0f);
    std::uniform_int_distribution<int> kindDist(0, 1);

    ItemData data;
    bool makeArtifact = forceArtifact || (!forceSpell && kindDist(rng) == 0);

    // this sets item type + values for proto
    if (makeArtifact)
    {
        data.type = ItemData::ItemType::ARTIFACT;
        data.name = "artifact";
        data.value = 40;
        data.color = Color4(255, 170, 60, 255);
    }
    else
    {
        data.type = ItemData::ItemType::SPELL;
        data.name = "spellbook";
        data.value = 25;
        data.color = Color4(120, 180, 255, 255);
    }

    // this chooses a random point on screen and sets a small pickup radius (can be smaller?)
    data.position = Vec2(xdist(rng), ydist(rng));
    data.radius = 14.0f;
    return data;
}

void GameScene::spawnInitialItems()
{
    _items.clear();
    _items.push_back(createRandomItem(true, false));
    _items.push_back(createRandomItem(false, true));
}

void GameScene::spawnReplacementItem()
{
    _items.push_back(createRandomItem(false, false));
}

void GameScene::updateItemPickupState()
{
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";
// not rn
    // this converts player center from physics units into screen units
//    for (size_t ii = 0; ii < _items.size(); ++ii)
//    {
//        if (_collisions.resolvePlayerItem(_player, _items[ii].position, _items[ii].radius))
//        {
//            _canPickupItem = true;
//            _pickupIndex = (int)ii;
//            // Can't use CU_TOUCH_SCREEN in this manner, since it just isn't defined on PC
//            //_pickupPrompt = CU_TOUCH_SCREEN ? "tap to pickup " + _items[ii].name : "press e to pickup " + _items[ii].name;
//            return;
//        }
//    }
}

void GameScene::tryPickupItem()
{
    if (!_input.pressedPickup())
        return;
    _input.resolvedPickup();

    if (!_canPickupItem || _pickupIndex < 0)
        return;

    if (_pickupIndex < (int)_items.size())
    {
        _player->addItem(_items[_pickupIndex].value);
        _items.erase(_items.begin() + _pickupIndex);
        spawnReplacementItem();

        // this refreshes prompt state right away after pickup
        updateItemPickupState();
    }
}

void GameScene::drawItems()
{
    _batch->setTexture(nullptr);
    for (const auto &item : _items)
    {
        // this draws a small rectangle so we do not need extra textures yet
        Rect body(item.position.x - item.radius, item.position.y - item.radius,
                  item.radius * 2.0f, item.radius * 2.0f);
        _batch->setColor(item.color);
        _batch->setTexture(nullptr);
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
            
            // init network and world once we start the game
            //Make a std::function reference of the linkSceneToObs function in game scene for network controller
            std::function<void(const std::shared_ptr<physics2::Obstacle>&, const std::shared_ptr<scene2::SceneNode>&)> linkSceneToObsFunc = [=, this](const std::shared_ptr<physics2::Obstacle>& obs, const std::shared_ptr<scene2::SceneNode>& node) {
                this->linkSceneToObs(obs, node);
                };
            
            _networkCtrl->enablePhysics(_world, linkSceneToObsFunc);

            _spellFactId = _networkCtrl->getPhysController()->attachFactory(_spellFact);
            _playerFactId = _networkCtrl->getPhysController()->attachFactory(_playerFact);
            _enemyFactId = _networkCtrl->getPhysController()->attachFactory(_enemyFact);
            
            // create all players (both host n client do this) - must do once game scene takes over network handling
            for (auto p : _networkCtrl->getPlayers())
            {
                auto params = _playerFact->serializeParams(getSize() / 2, _networkCtrl->getPID());
                auto pair = _networkCtrl->getPhysController()->addSharedObstacle(_playerFactId, params);
            }
            // set _player to the correct object
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
                p_node->setPosition(pos * 1);
                p_node->setAngle(angle);
                });
            
        } else {
            obj->setListener([=, this](physics2::Obstacle* obs) {
                float leftover = Application::get()->getFixedRemainder() / 1000000.f;
                Vec2 pos = obs->getPosition() + leftover * obs->getLinearVelocity();
                float angle = obs->getAngle() + leftover * obs->getAngularVelocity();
                weak->setPosition(pos * 1);
                weak->setAngle(angle);
                });
        }
        
    }
}

void GameScene::initPlayers() {
    //auto image = _assets->get<graphics::Texture>("player"); // TODO this will need to update with additional metadata (which asset to get)
    //auto size = image->getSize();
    //_player = std::make_shared<Player>(getSize() / 2, 12, image);

    //Vec2 playerPos = (_player->getPosition());
    //auto playerOb = PlayerObstacle::alloc(playerPos, "???", size);

    //addInitObstacle(playerOb, _player);

}


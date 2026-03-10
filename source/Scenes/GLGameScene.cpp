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
#include <cugl/core/math/polygon/CUPathFactory.h>  // Bug that this is not in <cugl/cugl.h>
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    if (assets == nullptr) {
        return false;
    }
    else if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) {
        return false;
    }

    _assets = assets;

    // Start up the input handler
    _input.init(cugl::Rect(0, 0, getSize().width, getSize().height));
    _inputDisplay = make_shared<InputDisplay>(&_input);

    // create the player
    _player = make_shared<Player>(getSize() / 2, 12);
    _player->setTexture(assets->get<Texture>("player"));
    
    _players = make_shared<PlayerSet>();
    _players->addPlayer(_player);
    
    // Create enemy with 10 health
    _enemy = make_shared<EnemySet>();
    _enemy->setTexture(assets->get<Texture>("enemy"));
//    _enemy->spawnEnemy(getSize() / 4, 4);
    lastEnemySpawn = 0;

    // Initialize inventory system
    _inventoryValue = 0;
    _quota = 100;
    _playerWon = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    spells = SpellSet();
    spells.setTexture(assets->get<Texture>("spell"));
    _collisions.init(getSize());


    // this resets pickup prompt state
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";

    // this spawns one artifact and one spell for prototype startup
    spawnInitialItems();


    //Creation of the physics world and star translator
    cugl::Rect bounds = Rect(0, 0, getSize().width / PHYSICS_SCALE, getSize().height / PHYSICS_SCALE);
    _world = cugl::physics2::ObstacleWorld::alloc(bounds, Vec2(0, 0));


    buildGeometry();
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    // NOTHING TO DO THIS TIME
    spells.dispose();
    _inputDisplay->dispose();
    _assets->dispose();
    _assets = nullptr;
}

void GameScene::reset() {
    _player->setHealth(100);
    _player->setPosition(getSize() / 2);
    _player->setVelocity(Vec2::ZERO);
    _playerLost = false;
    _playerWon = false;
    _inventoryValue = 0;
    _enemy->reset();
    _enemy->spawnEnemy(getSize() / 4, 4);
    lastEnemySpawn = 0;

    spells.current.clear();
    spells.pending.clear();
    spawnInitialItems();
}

#pragma mark -
#pragma mark Inventory

void GameScene::addItem(int value) {
    _inventoryValue += value;
    if (_inventoryValue >= _quota) {
        _playerWon = true;
    }
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
void GameScene::preUpdate(float dt) {
    _input.update(dt);

    if (_networkCtrl && _networkCtrl->hasNetwork()) {
        _networkCtrl->update();
    }

    if (_input.pressedReset()) {
        reset();
        _input.resolvedReset();
        return;
    }

    if (_playerLost || _playerWon) {
        return;
    }
    // spawn an enemy if host and need to spawn
    if (_ishost){
        lastEnemySpawn += dt;
        if (lastEnemySpawn > 15) {
            lastEnemySpawn = 0;
            _enemy->spawnEnemy(getSize() / 2, 4);
        }
    }
    

    // Player movement
    cugl::Vec2 input = _input.getMove();
    cugl::Vec2 velocity = input * _player->getSpeed();
    _player->setVelocity(velocity);
    _player->setColor(_input.showAim() ? Color4::BLUE : Color4::WHITE);
    _player->update();

    cugl::Vec2 playerScreenPos = _player->getPosition();

    updateItemPickupState();
    tryPickupItem();

    // Spellcasting
    if (_input.getGesture() != SpellType::NONE && _player->getSpell() == SpellType::NONE) {
        _player->setSpell(_input.getGesture());
    }
    
    bool sendSpell = false;

    if (_input.shouldReleaseSpell()) {
        sendSpell = true;
        CULog("Release spell");
        CULog("Pos: %.2f %.2f", playerScreenPos.x, playerScreenPos.y);

        Vec2 angle = _input.getSpellAngle();
        if (angle.isZero()) {
            angle = _player->getDirection();
        }
        if (_ishost) {
            switch (_player->getSpell()) {
            case SpellType::FREEZE:
                spells.spawnSpell(playerScreenPos, angle, true);
                CULog("spawn freeze spell");

                break;
            case SpellType::FIREBALL:
                spells.spawnSpell(playerScreenPos, angle, false);
                break;
            }
        }
        
        _input.resolveSpell();
        _player->resolveSpell();
        CULog("Resolve spell");
        
    }

    //collision update, only for host!
    if (_ishost) {
        spells.update();
        float playerRadius = 15.0f;
        _collisions.resolvePlayerEnemies(_player, playerScreenPos, playerRadius, _enemy);
        _collisions.resolveEnemiesSpell(spells, _enemy, playerScreenPos * -1.5);
    }


    if (_player->getHealth() <= 0) {
        _playerLost = true;
    }

    if (!_ishost && _networkCtrl->hasNetwork()) {
        ClientInput ci;
        ci.joystickAngle = _input.getMove().isZero() ? 0.0f : _input.getMove().getAngle();
        ci.pickupPressed = _input.pressedPickup();
        ci.posX = playerScreenPos.x;
        ci.posY = playerScreenPos.y;
        if (sendSpell) {
            ci.spellType = _input.getGesture();
            ci.spellAngle = _player->getAngle();
        }
        _networkCtrl->sendInput(ci);
    }

    // client recieve !!
    if (!_ishost && _networkCtrl->hasNetwork() && _networkCtrl->hasNewHostInput()) {
        HostInput hi = _networkCtrl->getLatestHostInput();
        _networkCtrl->clearNewHostInput();

        for (auto& pd : hi.players) {
            if (pd.uuid == _player->getPID()) continue;
            auto p = _players->getPlayerByPID(pd.uuid);
            if (p) {
                p->setPosition(cugl::Vec2(pd.posX, pd.posY));
                p->setHealth(pd.health);
            }
        }
        _enemy->reset(); // clear all
        for (auto& ed : hi.enemies) {
            _enemy->updateEnemyAsClient(ed.eid, cugl::Vec2(ed.posX, ed.posY), ed.health);
        }

        //spells.current.clear();
        for (auto& sd : hi.spells) {
            spells.spawnSpell(cugl::Vec2(sd.posX, sd.posY), sd.velAngle, sd.type == SpellType::FREEZE);
        }
        spells.update(); // MAYBE?????

        // needs to be changed in the future depending on how we're doing artifact
        if (!hi.added_items.empty()) {
            _items.clear();
            for (auto& id : hi.added_items) {
                ItemData item;
                item.position = cugl::Vec2(id.posX, id.posY);
                item.name = id.item;
                item.radius = 14.0f;
                item.value = (id.item == "artifact") ? 40 : 25;
                item.color = (id.item == "artifact") ? Color4(255, 170, 60, 255) : Color4(120, 180, 255, 255);
                item.type = (id.item == "artifact") ? ItemData::ItemType::ARTIFACT : ItemData::ItemType::SPELL;
                _items.push_back(item);
            }
        }
    }

    // HOST: broadcast game state to clients
    if (_ishost && _networkCtrl && _networkCtrl->hasNetwork()) {
        //updates player positions (based on position currently, should change)
        for (auto& [pid, input] : _clientInputs) {
            auto p = _players->getPlayerByPID(pid);
            if (p) {
                
                p->setPosition(cugl::Vec2(input.posX, input.posY));
                if (input.spellType != SpellType::NONE) { // a spell WAS cast
                    spells.spawnSpell(cugl::Vec2(input.posX, input.posY), p->getAngle(), input.spellType==SpellType::FREEZE);
                }
            }
        }
        // update enemy positions - just see if they target host atm
        _enemy->update(playerScreenPos);


        HostInput hi;
        hi.dt = dt;

        for (auto p : _players->current) {
            HostPlayerData pd;
            pd.uuid = p->getPID();
            pd.posX = p->getPosition().x;
            pd.posY = p->getPosition().y;
            pd.speed = p->getVelocity().length();
            pd.velAngle = p->getVelocity().isZero() ? 0.0f : p->getVelocity().getAngle();
            pd.health = p->getHealth();
            hi.players.push_back(pd);
        }

        for (auto enem : _enemy->current) {
            auto e = std::get<1>(enem);
            HostEnemyData ed;
            ed.posX = e->getPosition().x;
            ed.posY = e->getPosition().y;
            ed.speed = e->getVelocity().length();
            ed.velAngle = e->getVelocity().isZero() ? 0.0f : e->getVelocity().getAngle();
            ed.health = e->getHealth();
            ed.eid = std::get<0>(enem);
            hi.enemies.push_back(ed);
        }

        for (auto s : spells.pending) { // i think we may need something different. maybe send set of pending??
            HostSpellData sd;
            sd.posX = s->position.x;
            sd.posY = s->position.y;
            sd.speed = s->velocity.length();
            sd.velAngle = s->velocity.isZero() ? 0.0f : s->velocity.getAngle();
            sd.type = s->froze ? SpellType::FREEZE : SpellType::FIREBALL;
            hi.spells.push_back(sd);
        }

        for (auto& item : _items) {
            HostItemData id;
            id.posX = item.position.x;
            id.posY = item.position.y;
            id.itemId = (int)(&item - &_items[0]); // the index is the id for now!
            id.item = item.name;
            hi.added_items.push_back(id);
        }

        auto bytes = MessageSerializer::encodeHostInput(hi);
        _networkCtrl->getNetwork()->broadcast(bytes);
    }

    _inputDisplay->update(playerScreenPos);


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
void GameScene::fixedUpdate(Uint64 step) {
    //float dt = step / 1000000.0f;
    //_world->update(dt);
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
void GameScene::postUpdate(float remain) {
}

/**
 * Draws all this scene to its associated SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch. By overriding it, you can do custom drawing
 * in its place. The expectation is that you will use the associated
 * SpriteBatch for drawing, but this is not required.
 */
void GameScene::render() {
    // DO NOT DO THIS IN YOUR FINAL GAME
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();

    Color4 original = _batch->getColor();

    drawItems();

    _players->draw(_batch);

    _enemy->draw(_batch);

    spells.draw(_batch);

    renderHUD();

    _inputDisplay->draw(_batch);

    _batch->setColor(original);
    _batch->end();
}

void GameScene::renderHUD() {
    Size screenSize = getSize();

    float barW = 280;
    float barH = 50;
    float margin = 16;

    // Helper lambda to draw a solid colored box using Poly2
    auto drawBox = [&](float x, float y, float w, float h, Color4 color) {
        std::vector<Vec2> verts = {
            Vec2(x,     y),
            Vec2(x + w, y),
            Vec2(x + w, y + h),
            Vec2(x,     y + h)
        };
        std::vector<Uint32> idx = { 0, 1, 2, 0, 2, 3 };
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

    if (_font) {
        _batch->setColor(Color4::WHITE);

        std::shared_ptr<TextLayout> invLabel = TextLayout::allocWithText(
            "Inventory: $" + std::to_string(_inventoryValue), _font);
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


GameScene::ItemData GameScene::createRandomItem(bool forceArtifact, bool forceSpell) {
    static std::random_device rd;
    static std::mt19937 rng(rd());

    std::uniform_real_distribution<float> xdist(80.0f, getSize().width - 80.0f);
    std::uniform_real_distribution<float> ydist(100.0f, getSize().height - 100.0f);
    std::uniform_int_distribution<int> kindDist(0, 1);

    ItemData data;
    bool makeArtifact = forceArtifact || (!forceSpell && kindDist(rng) == 0);

    // this sets item type + values for proto
    if (makeArtifact) {
        data.type = ItemData::ItemType::ARTIFACT;
        data.name = "artifact";
        data.value = 40;
        data.color = Color4(255, 170, 60, 255);
    }
    else {
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

void GameScene::spawnInitialItems() {
    _items.clear();
    _items.push_back(createRandomItem(true, false));
    _items.push_back(createRandomItem(false, true));
}

void GameScene::spawnReplacementItem() {
    _items.push_back(createRandomItem(false, false));
}

void GameScene::updateItemPickupState() {
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";

    // this converts player center from physics units into screen units
    for (size_t ii = 0; ii < _items.size(); ++ii) {
        if (_collisions.resolvePlayerItem(_player, _items[ii].position, _items[ii].radius)) {
            _canPickupItem = true;
            _pickupIndex = (int)ii;
            // Can't use CU_TOUCH_SCREEN in this manner, since it just isn't defined on PC
            //_pickupPrompt = CU_TOUCH_SCREEN ? "tap to pickup " + _items[ii].name : "press e to pickup " + _items[ii].name;
            return;
        }
    }
}

void GameScene::tryPickupItem() {
    if (!_input.pressedPickup()) return;
    _input.resolvedPickup();

    if (!_canPickupItem || _pickupIndex < 0) return;

    if (_pickupIndex < (int)_items.size()) {
        addItem(_items[_pickupIndex].value);
        _items.erase(_items.begin() + _pickupIndex);
        spawnReplacementItem();

        // this refreshes prompt state right away after pickup
        updateItemPickupState();
    }

}

void GameScene::drawItems() {
    _batch->setTexture(nullptr);
    for (const auto& item : _items) {
        // this draws a small rectangle so we do not need extra textures yet
        Rect body(item.position.x - item.radius, item.position.y - item.radius,
            item.radius * 2.0f, item.radius * 2.0f);
        _batch->setColor(item.color);
        _batch->setTexture(nullptr);
        _batch->fill(body);
    }
}




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
void GameScene::buildGeometry() {
    //_world -> clear();

    //_player->addTo(_world);
}


void GameScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _input.activate();
            // Host registers callback to collect client inputs each frame
            if (_ishost) {
                _networkCtrl->setInputCallback(
                    [this](const std::string& playerId, const ClientInput& input) {
                        _clientInputs[playerId] = input;
                    }
                );
            }
            // create the OTHER players (both host n client do this) - must do once game scene takes over network handling
            _player->setPID(_networkCtrl->getPID());
            CULog("num players acc to game network: %d", _networkCtrl->getPlayers().size());
            for (auto p : _networkCtrl->getPlayers()) {
                if (p!=_networkCtrl->getPID()) {
                    CULog("adding ANOTHER player");
                    _players->addPlayer(make_shared<Player>(cugl::Vec2(2 + getSize().width/2, 2 + getSize().height/2), 12, p,  _assets->get<Texture>("player")));
                }
            }
            CULog("num players in set: %d", _players->numPlayers() );
            CULog("GameScene setActive: hasNetwork=%d isHost=%d",
                _networkCtrl->hasNetwork(), _networkCtrl->isHost());
        }
        else {
            _input.deactivate();
        }
    }
}

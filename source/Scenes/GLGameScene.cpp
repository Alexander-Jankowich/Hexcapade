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
#include <cmath>
#include <iostream>
#include <sstream>
#include <random> //itemssss


#include "GLGameScene.h"
#include "../Models/Artifact.h"
#include "../Models/Inventory.h"
#include "../Models/SpellBook.h"

using namespace cugl;
using namespace std;
using namespace cugl::graphics;

namespace {
    constexpr size_t kPrototypeInventoryCapacity = 4;
    constexpr int kGuaranteedArtifacts = 1;
    constexpr int kGuaranteedSpellbooks = 1;
    constexpr float kTargetWorldValueMultiplier = 1.35f;
    constexpr int kMaxAdditionalPrototypeItems = 40;

    std::shared_ptr<Item> makePrototypeNetItem(const std::string& itemName, int itemId) {
        if (itemName == "artifact") {
            return std::make_shared<Artifact>(
                "net_artifact_" + std::to_string(itemId),
                "artifact",
                40,
                Artifact::Effect::ARTIFACT_EFFECT
            );
        }

        return std::make_shared<SpellBook>(
            "net_spellbook_" + std::to_string(itemId),
            "spellbook",
            25,
            SpellBook::Spell::SPELL_EFFECT
        );
    }
}

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
    _input.setInventoryPanelOpen(false);
    _inputDisplay = make_shared<InputDisplay>(&_input);

    // create the player
    _player = make_shared<Player>(getSize() / 2, 12);
    _player->setTexture(assets->get<Texture>("player"));
    _player->setInventory(std::make_shared<Inventory>(kPrototypeInventoryCapacity));
    
    _players = make_shared<PlayerSet>();
    _players->addPlayer(_player);
    
    // Create enemy with 10 health
    _enemy = make_shared<EnemySet>();
    _enemy->setTexture(assets->get<Texture>("enemy"));
//    _enemy->spawnEnemy(getSize() / 4, 4);
    lastEnemySpawn = 0;

    // Initialize inventory system
    _depositedValue = 0;
    _quota = 100;
    _playerWon = false;
    _playerLost = false;

    try {
        _font = assets->get<Font>("OpenSans");
    }
    catch (...) {
        _font = nullptr;
    }

    spells = SpellSet();
    spells.setTexture(assets->get<Texture>("spell"));
    _collisions.init(getSize());

    // GA: itemnet
    // Until procgen room/item placement is merged, the host seeds a temporary
    // screen-space spawn area so ItemSet can own world pickups.
    _itemSet.setSpawnBounds(Rect(60, 80, getSize().width - 120, getSize().height - 170));

    // this resets pickup prompt state
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";
    _canDeposit = false;
    _depositRadius = 42.0f;
    _depositPosition = Vec2(getSize().width * 0.5f, getSize().height * 0.5f);
    _inventoryPanelOpen = false;

    _itemSet.reset();
    if (kGuaranteedArtifacts > 0 || kGuaranteedSpellbooks > 0) {
        _itemSet.spawnInitialPrototypeItems();
    }
    _itemSet.spawnUntilTotalValueAtLeast(
        (int)std::ceil((float)_quota * kTargetWorldValueMultiplier),
        kMaxAdditionalPrototypeItems
    );


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
    _depositedValue = 0;
    _enemy->reset();
    _enemy->spawnEnemy(getSize() / 4, 4);
    lastEnemySpawn = 0;

    if (_player->getInventory()) {
        _player->getInventory()->clear();
    }

    spells.current.clear();
    spells.pending.clear();
    _itemSet.reset();
    _itemSet.setSpawnBounds(Rect(60, 80, getSize().width - 120, getSize().height - 170));
    _itemSet.spawnInitialPrototypeItems();
    _itemSet.spawnUntilTotalValueAtLeast(
        (int)std::ceil((float)_quota * kTargetWorldValueMultiplier),
        kMaxAdditionalPrototypeItems
    );
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt.clear();
    _canDeposit = false;
    _inventoryPanelOpen = false;
    _input.setInventoryPanelOpen(false);
}

#pragma mark -
#pragma mark Inventory

void GameScene::addItem(int value) {
    // GA: itemnet old scene-local carry-value path kept only as a migration
    // bridge for any remaining call sites while Inventory becomes authoritative.
    _inventoryValue += value;
}

int GameScene::getInventoryValue() const {
    if (!_player || !_player->getInventory()) {
        return _inventoryValue;
    }
    return _player->getInventory()->getCarriedValue();
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
    updateDepositState();
    _input.setInventoryPanelOpen(_inventoryPanelOpen);

    bool pickupRequested = false;
    bool depositRequested = false;
    int dropIndex = -1;

    if (_input.pressedPickup()) {
        pickupRequested = true;
        _input.resolvedPickup();
    }
    if (_input.pressedDrop()) {
        std::shared_ptr<Inventory> inventory = _player->getInventory();
        if (inventory && !inventory->isEmpty()) {
            dropIndex = (int)inventory->getCount() - 1;
        }
        _input.resolvedDrop();
    }
    if (_input.pressedDeposit()) {
        depositRequested = true;
        _input.resolvedDeposit();
    }
    handleMobileHUDInput(pickupRequested, depositRequested, dropIndex);

    bool hasNetwork = _networkCtrl && _networkCtrl->hasNetwork();
    if (!hasNetwork || _ishost) {
        if (pickupRequested) {
            pickupNearbyItem();
        }
        if (dropIndex >= 0) {
            dropInventoryIndex((size_t)dropIndex);
        }
        if (depositRequested) {
            depositAtDropoff();
        }
    }

    // Spellcasting
    if (_input.getGesture() != SpellType::NONE && _player->getSpell() == SpellType::NONE) {
        _player->setSpell(_input.getGesture());
    }
    
    bool sendSpell = false;
    Vec2 angle = _input.getSpellAngle();
    if (angle.isZero()) {
        angle = _player->getDirection();
    }
    if (_input.shouldReleaseSpell()) {
        sendSpell = true;
        CULog("Release spell--------------------------------------------------------");
        //CULog("Pos: %.2f %.2f", playerScreenPos.x, playerScreenPos.y);
        if (_ishost) {
            spells.spawnSpell(_player->getSpellId(), _player->getSpell(), playerScreenPos, angle);

            _input.resolveSpell();
            _player->resolveSpell();
        }
    }
    //collision update, only for host!
    if (_ishost) {
        float playerRadius = 15.0f;
        _collisions.resolvePlayerEnemies(_player, playerScreenPos, playerRadius, _enemy);
        _collisions.resolveEnemiesSpell(spells, _enemy, playerScreenPos * -1.5);
    }


    if (!_ishost && _networkCtrl->hasNetwork()) {
        ClientInput ci;
        ci.joystickAngle = _input.getMove().isZero() ? 0.0f : _input.getMove().getAngle();
        ci.pickupPressed = pickupRequested;
        ci.dropPressed = dropIndex >= 0;
        ci.depositPressed = depositRequested;
        ci.dropIndex = dropIndex;
        ci.posX = playerScreenPos.x;
        ci.posY = playerScreenPos.y;
        ci.spellType = _player->getSpell();
        if (sendSpell) {
            ci.sid = _player->getSpellId();
            ci.spellAngle = angle.getAngle();
            //CULog("Angle of spell: %f", angle.getAngle());
        }
        _networkCtrl->sendInput(ci);
    }

    // client recieve !!
    if (!_ishost && _networkCtrl->hasNetwork() && _networkCtrl->hasNewHostInput()) {
        HostInput hi = _networkCtrl->getLatestHostInput();
        _networkCtrl->clearNewHostInput();

        for (auto& pd : hi.players) {
            if (pd.uuid == _player->getPID()) {
                continue;
            }
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

        spells.current.clear();
        for (auto& sd : hi.spells) {
            spells.spawnSpell(sd.id, sd.type, Vec2(sd.posX, sd.posY), sd.velAngle);
        }
        spells.update(); // MAYBE?????

        if (spells.contains(_player->getSpellId())) {
            _input.resolveSpell();
            _player->resolveSpell();
        }

        // GA: itemnet old added_items path still acts as a temporary host snapshot
        // until the explicit world-item snapshot structs land.
        _itemSet.reset();
        _itemSet.setSpawnBounds(Rect(60, 80, getSize().width - 120, getSize().height - 170));
        for (auto& id : hi.added_items) {
            _itemSet.addDroppedItem(
                makePrototypeNetItem(id.item, id.itemId),
                cugl::Vec2(id.posX, id.posY),
                14.0f
            );
        }
    }

    // HOST: broadcast game state to clients
    if (_ishost && _networkCtrl && _networkCtrl->hasNetwork()) {
        //updates player positions (based on position currently, should change)
        HostInput hi;

        for (auto& [pid, input] : _clientInputs) {
            auto p = _players->getPlayerByPID(pid);
            if (p) {
                p->setPosition(cugl::Vec2(input.posX, input.posY));

                if (input.pickupPressed) {
                    hostPickupForPlayer(p);
                }
                if (input.dropPressed && input.dropIndex >= 0) {
                    hostDropInventoryIndex(p, (size_t)input.dropIndex);
                }
                if (input.depositPressed) {
                    hostDepositForPlayer(p);
                }

                if (input.sid.length() > 1) { // a spell WAS cast
                    if (!spells.contains(input.sid)) { // First time trying to spawn this spell
                        CULog("Other Spawn-------------------------------------------------");
                        spells.spawnSpell(input.sid, input.spellType, Vec2(input.posX, input.posY), input.spellAngle);
                    }
                    p->resolveSpell();
                }
            }
        }
        // update enemy positions - just see if they target host atm
        std::vector<cugl::Vec2> player_pos;
        for (auto p : _players->current) {
            player_pos.push_back(p->getPosition());
        }
        _enemy->update(player_pos);


        hi.dt = dt;

        for (auto p : _players->current) {
            HostPlayerData pd;
            pd.uuid = p->getPID();
            pd.posX = p->getPosition().x;
            pd.posY = p->getPosition().y;
            pd.speed = p->getVelocity().length();
            pd.velAngle = p->getVelocity().isZero() ? 0.0f : p->getVelocity().getAngle();
            pd.inventoryVal = p->getInventory() ? p->getInventory()->getCarriedValue() : 0;
            pd.curSpell = p->getSpell();
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
        
        for (auto [sid, sp] : spells.current) {
            HostSpellData sd;
            sd.id = sid;
            sd.posX = sp->position.x;
            sd.posY = sp->position.y;
            sd.velAngle = sp->velocity.getAngle();
            sd.type = sp->froze ? SpellType::FREEZE : SpellType::FIREBALL;
            hi.spells.push_back(sd);
        }

        const std::vector<ItemSet::WorldItem>& worldItems = _itemSet.getItems();
        for (size_t ii = 0; ii < worldItems.size(); ++ii) {
            HostItemData id;
            id.posX = worldItems[ii].position.x;
            id.posY = worldItems[ii].position.y;
            id.itemId = (int)ii;
            id.item = worldItems[ii].item->getKind() == Item::Kind::ARTIFACT ? "artifact" : "spellbook";
            hi.added_items.push_back(id);
        }

        auto bytes = MessageSerializer::encodeHostInput(hi);
        _networkCtrl->getNetwork()->broadcast(bytes);
    }

    _inputDisplay->update(playerScreenPos);
    if (sendSpell) {
    }
    
    //collision update, only for host!
    if (_ishost) {
        spells.update();
    }


    if (_player->getHealth() <= 0) {
        _playerLost = true;
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

    drawDepositZone();
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

    float barW = 320;
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

    Rect inventoryHeader = _input.getInventoryHeaderBounds();
    drawBox(inventoryHeader.origin.x, inventoryHeader.origin.y,
            inventoryHeader.size.width, inventoryHeader.size.height, boxColor);

    // Quota box - top right
    float quotaX = screenSize.width - barW - margin;
    float quotaY = screenSize.height - barH - margin;
    drawBox(quotaX, quotaY, barW, barH, boxColor);

    // Health box - top center
    float healthW = 200;
    float healthX = screenSize.width / 2 - healthW / 2;
    float healthY = screenSize.height - barH - margin;
    drawBox(healthX, healthY, healthW, barH, boxColor);

    Rect actionButton = _input.getActionButtonBounds();
    drawBox(actionButton.origin.x, actionButton.origin.y,
            actionButton.size.width, actionButton.size.height, boxColor);

    if (_font) {
        _batch->setColor(Color4::WHITE);

        std::shared_ptr<Inventory> inventory = _player ? _player->getInventory() : nullptr;
        std::shared_ptr<TextLayout> invLabel = TextLayout::allocWithText(
            "Inventory: $" + std::to_string(getInventoryValue()), _font);
        invLabel->layout();
        Affine2 invTrans;
        invTrans.translate(inventoryHeader.origin.x + 10, inventoryHeader.origin.y + 12);
        _batch->drawText(invLabel, invTrans);

        std::shared_ptr<TextLayout> quotaLabel = TextLayout::allocWithText(
            "Quota: $" + std::to_string(_depositedValue) + " / $" + std::to_string(_quota), _font);
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

        std::string pickupText = "move close to item";
#ifdef CU_TOUCH_SCREEN
        if (_canPickupItem) {
            if (inventory && !inventory->canAddItem()) {
                pickupText = "inventory full";
            } else {
                pickupText = "pickup item";
            }
        } else if (_canDeposit && inventory && !inventory->isEmpty()) {
            pickupText = "deposit at origin";
        }
#else
        if (_canPickupItem) {
            pickupText = _pickupPrompt;
        } else if (_canDeposit && inventory && !inventory->isEmpty()) {
            pickupText = "press f to deposit carried items";
        } else if (inventory && !inventory->isEmpty()) {
            pickupText = "e pickup  q drop  f deposit";
        }
#endif
        std::shared_ptr<TextLayout> pickupLabel = TextLayout::allocWithText(pickupText, _font);
        pickupLabel->layout();
        Affine2 pickupTrans;
        pickupTrans.translate(actionButton.origin.x + 10, actionButton.origin.y + 12);
        _batch->drawText(pickupLabel, pickupTrans);

#ifdef CU_TOUCH_SCREEN
        if (_inventoryPanelOpen && inventory) {
            Rect panelBounds = _input.getInventoryPanelBounds();
            drawBox(panelBounds.origin.x, panelBounds.origin.y,
                    panelBounds.size.width, panelBounds.size.height,
                    Color4(20, 20, 20, 230));

            float rowH = panelBounds.size.height / 4.0f;
            for (int ii = 0; ii < 4; ++ii) {
                float rowY = panelBounds.getMaxY() - rowH * (ii + 1);
                drawBox(panelBounds.origin.x + 2.0f, rowY + 1.0f,
                        panelBounds.size.width - 4.0f, rowH - 2.0f,
                        Color4(40, 40, 40, 220));

                std::string rowText = "-";
                if ((size_t)ii < inventory->getCount()) {
                    rowText = inventory->getItems()[(size_t)ii]->getName();
                }

                std::shared_ptr<TextLayout> rowLabel = TextLayout::allocWithText(rowText, _font);
                rowLabel->layout();
                Affine2 rowTrans;
                rowTrans.translate(panelBounds.origin.x + 10.0f, rowY + 8.0f);
                _batch->drawText(rowLabel, rowTrans);
            }
        }
#endif
    }
}

void GameScene::handleMobileHUDInput(bool& pickupRequested, bool& depositRequested, int& dropIndex) {
#ifdef CU_TOUCH_SCREEN
    if (_input.pressedInventoryToggle()) {
        _inventoryPanelOpen = !_inventoryPanelOpen;
        _input.setInventoryPanelOpen(_inventoryPanelOpen);
        _input.resolvedInventoryToggle();
        CULog("inventory panel toggled: %d", _inventoryPanelOpen ? 1 : 0);
    }

    if (_input.pressedInventoryPanel()) {
        if (_inventoryPanelOpen) {
            int index = getInventoryPanelIndexAt(_input.getLastUITap());
            if (index >= 0) {
                dropIndex = index;
            }
        }
        _input.resolvedInventoryPanel();
    }

    if (_input.pressedContextAction()) {
        _input.resolvedContextAction();
        if (_canPickupItem) {
            pickupRequested = true;
        } else if (_canDeposit) {
            depositRequested = true;
        }
    }
#else
    if (_input.pressedInventoryToggle()) {
        _inventoryPanelOpen = !_inventoryPanelOpen;
        _input.resolvedInventoryToggle();
    }
#endif
}

bool GameScene::pickupNearbyItem() {
    return hostPickupForPlayer(_player);
}

bool GameScene::depositAtDropoff() {
    return hostDepositForPlayer(_player);
}

bool GameScene::dropInventoryIndex(size_t index) {
    return hostDropInventoryIndex(_player, index);
}

bool GameScene::hostPickupForPlayer(const std::shared_ptr<Player>& player) {
    // Host-authoritative item pickup. The host validates overlap and capacity
    // before moving an item from ItemSet into the player's Inventory.
    if (!player) {
        return false;
    }

    int pickupIndex = _itemSet.findNearestPickupIndex(player->getPosition(), player->getSize());
    if (pickupIndex < 0) {
        return false;
    }

    std::shared_ptr<Inventory> inventory = player->getInventory();
    if (!inventory || !inventory->canAddItem()) {
        if (player == _player) {
#ifdef CU_TOUCH_SCREEN
            _pickupPrompt = "inventory full";
#else
            _pickupPrompt = "inventory full, press q to drop";
#endif
        }
        return false;
    }

    std::shared_ptr<Item> item = _itemSet.takeAt((size_t)pickupIndex);
    if (!item) {
        return false;
    }

    if (!inventory->addItem(item)) {
        _itemSet.addDroppedItem(item, player->getPosition());
        return false;
    }

    if (player == _player) {
        _inventoryValue = inventory->getCarriedValue();
        updateItemPickupState();
        updateDepositState();
    }
    return true;
}

bool GameScene::hostDropInventoryIndex(const std::shared_ptr<Player>& player, size_t index) {
    // Host-authoritative drop. The host verifies slot ownership before putting
    // the item back into ItemSet near the player.
    if (!player) {
        return false;
    }

    std::shared_ptr<Inventory> inventory = player->getInventory();
    if (!inventory) {
        return false;
    }

    std::shared_ptr<Item> dropped = inventory->dropItemAt(index);
    if (!dropped) {
        return false;
    }

    Vec2 dropDir = player->getDirection();
    if (dropDir.isZero()) {
        dropDir = Vec2(1, 0);
    }

    Vec2 dropPos = player->getPosition() + dropDir.getNormalization() * 26.0f;
    _itemSet.addDroppedItem(dropped, dropPos);

    if (player == _player) {
        _inventoryValue = inventory->getCarriedValue();
        updateItemPickupState();
        updateDepositState();
    }
    return true;
}

bool GameScene::hostDepositForPlayer(const std::shared_ptr<Player>& player) {
    // Host-authoritative deposit. The host checks deposit-zone overlap and then
    // moves carried value into the shared team quota total.
    if (!player) {
        return false;
    }

    if (!_collisions.resolvePlayerItem(player, _depositPosition, _depositRadius)) {
        return false;
    }

    std::shared_ptr<Inventory> inventory = player->getInventory();
    if (!inventory || inventory->isEmpty()) {
        return false;
    }

    _depositedValue += inventory->depositAllItems();
    if (_depositedValue >= _quota) {
        _playerWon = true;
    }

    if (player == _player) {
        _inventoryValue = inventory->getCarriedValue();
        updateDepositState();
    }
    return true;
}

int GameScene::getInventoryPanelIndexAt(const Vec2& tapPos) const {
    Rect panel = _input.getInventoryPanelBounds();
    if (!panel.contains(tapPos)) {
        return -1;
    }

    std::shared_ptr<Inventory> inventory = _player ? _player->getInventory() : nullptr;
    if (!inventory || inventory->isEmpty()) {
        return -1;
    }

    float rowH = panel.size.height / 4.0f;
    float fromTop = panel.getMaxY() - tapPos.y;
    int index = (int)(fromTop / rowH);
    if (index < 0 || index >= (int)inventory->getCount()) {
        return -1;
    }
    return index;
}

void GameScene::updateDepositState() {
    _canDeposit = _collisions.resolvePlayerItem(_player, _depositPosition, _depositRadius);
}

void GameScene::tryDropItem() {
    if (!_input.pressedDrop()) {
        return;
    }
    _input.resolvedDrop();

    std::shared_ptr<Inventory> inventory = _player->getInventory();
    if (!inventory || inventory->isEmpty()) {
        return;
    }

    dropInventoryIndex(inventory->getCount() - 1);
}

void GameScene::tryDepositItems() {
    if (!_input.pressedDeposit()) {
        return;
    }
    _input.resolvedDeposit();
    depositAtDropoff();
}

void GameScene::drawDepositZone() {
    _batch->setTexture(nullptr);

    Color4 color = _canDeposit ? Color4(70, 180, 95, 220) : Color4(60, 120, 70, 200);
    Rect zone(_depositPosition.x - _depositRadius, _depositPosition.y - _depositRadius,
              _depositRadius * 2.0f, _depositRadius * 2.0f);

    _batch->setColor(color);
    _batch->fill(zone);
}

// GA: itemnet old scene-local item prototype kept for reference while ItemSet
// and Inventory replace it as the gameplay source of truth.
#if 0
GameScene::ItemData GameScene::createRandomItem(bool forceArtifact, bool forceSpell) {
    ...
}

void GameScene::spawnInitialItems() {
    ...
}

void GameScene::spawnReplacementItem() {
    ...
}
#endif

void GameScene::updateItemPickupState() {
    _canPickupItem = false;
    _pickupIndex = -1;
    _pickupPrompt = "";

    _pickupIndex = _itemSet.findNearestPickupIndex(_player->getPosition(), _player->getSize());
    if (_pickupIndex < 0) {
        return;
    }

    const std::vector<ItemSet::WorldItem>& items = _itemSet.getItems();
    if (_pickupIndex >= (int)items.size()) {
        _pickupIndex = -1;
        return;
    }

    _canPickupItem = true;
    std::shared_ptr<Inventory> inventory = _player->getInventory();
    if (inventory && !inventory->canAddItem()) {
#ifdef CU_TOUCH_SCREEN
        _pickupPrompt = "inventory full";
#else
        _pickupPrompt = "inventory full, press q to drop";
#endif
    } else {
#ifdef CU_TOUCH_SCREEN
        _pickupPrompt = "pickup item";
#else
        _pickupPrompt = "press e to pickup " + items[(size_t)_pickupIndex].item->getName();
#endif
    }
}

void GameScene::tryPickupItem() {
    if (!_input.pressedPickup()) {
        return;
    }
    _input.resolvedPickup();
    pickupNearbyItem();
}

void GameScene::drawItems() {
    _batch->setTexture(nullptr);
    for (const ItemSet::WorldItem& world : _itemSet.getItems()) {
        Rect body(world.position.x - world.radius, world.position.y - world.radius,
                  world.radius * 2.0f, world.radius * 2.0f);
        _batch->setColor(world.item->getDebugColor());
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
                    std::shared_ptr<Player> remote = make_shared<Player>(
                        cugl::Vec2(2 + getSize().width/2, 2 + getSize().height/2), 12, p, _assets->get<Texture>("player")
                    );
                    remote->setInventory(std::make_shared<Inventory>(kPrototypeInventoryCapacity));
                    _players->addPlayer(remote);
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

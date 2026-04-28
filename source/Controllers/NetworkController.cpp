//
//  NetworkController.cpp
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//

#include "NetworkController.h"

using namespace cugl;
using namespace cugl::netcode;
using namespace std;
#define PHYSICS_SCALE 50

#pragma mark PrivateClientHelpers



std::string NetworkController::dec2hex(const std::string& dec) {
    Uint32 value = strtool::stou32(dec);
    if (value >= 65536) value = 0;
    return strtool::to_hexstring(value, 4);
}

#pragma mark PrivateHostHelpers

std::string NetworkController::hex2dec(const std::string& hex) {
    Uint32 value = strtool::stou32(hex, 0, 16);
    std::string result = strtool::to_string(value);
    // Pad to 5 digits so room codes always look the same length
    if (result.size() < 5) {
        std::string padded(5, '0');
        size_t diff = 5 - result.size();
        for (size_t i = 0; i < result.size(); i++) padded[diff + i] = result[i];
        result = padded;
    }
    return result;
}



#pragma mark PublicMethods

void NetworkController::init(const std::shared_ptr<AssetManager> &a) {
    attachEventType<InitMapEvent>();
    attachEventType<CastSpellEvent>();
    attachEventType<EndGameEvent>();
    attachEventType<DepositEvent>();
    attachEventType<PickupEvent>();
    attachEventType<SetSpellEvent>();
    attachEventType<RestartGameEvent>();
    attachEventType<StartGameEvent>();
    cugl::physics2::distrib::NetEventController::init(a);
    _rng = Random::alloc();
    _rng->init();
    _randSeed = 0;
    _round = 0;
    _itemFactory = nullptr;
    _playerFactory = nullptr;
    _itemFactId = -1;
    _localPlayer = nullptr;
    _inputCtrl = nullptr;
    _depositPosition = Vec2::ZERO;
    _depositRadius = -1;
}

bool NetworkController::connectClient(string roomId) {
    if (_status != Status::IDLE) return true; // already connected
    return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
}



// this needs to be refactored heavily, give network controller pointers
// booleans pop up into network controller state, accessed by endScene
// idk what deposit radius is
std::tuple<int,int> NetworkController::customEvents(){
    // handle cast spell event
    int inventoryValue = 0;
    int depositedValue = 0;
    while (isInAvailable()){
        auto e = popInEvent();
        if(auto cse = std::dynamic_pointer_cast<CastSpellEvent>(e)){
            CULog("COMBO SPELL GOT");
            processComboEvent(cse);
        } else if (auto cse = std::dynamic_pointer_cast<EndGameEvent>(e)){
            CULog("END GAME GOT");
            processEndEvent(cse);
        } else if (auto cse = std::dynamic_pointer_cast<StartGameEvent>(e)){
            CULog("START GAME GOT");
            _gameState = NCGameState::RUNNING;
        } else if (auto ime = std::dynamic_pointer_cast<InitMapEvent>(e)){
            CULog("MAP INIT  GOT");
            _randSeed = ime->getSeed();
        } else if (auto rse = std::dynamic_pointer_cast<RestartGameEvent>(e)){
            CULog("RESTART GAME GOT");
            processRestartEvent(rse);
        } else if (auto sse = std::dynamic_pointer_cast<SetSpellEvent>(e)){
            //CULog("SET SPELL GOT");
            for (auto& p : _playerFactory->current) {
                if (p->getPID() == sse->getPID()) {
                    p->setSpell(sse->getSpell());
                    break;
                }
            }
        } else if (auto de = std::dynamic_pointer_cast<DepositEvent>(e)) {
            // TODO: rewrite
            // 1. Host recieve 2. Host applies deposit 3. Client applies deposit
            if (isHost()) {
                if (!de->isSync()) {
                    // Clients send deposit request and the host sends back the
                    // final synced amount after checking zone and inventory
                    CULog("HOST deposit request");
                    for (auto& p : _playerFactory->current) {
                        if (p->getPID() == de->getPID()) {
                            Vec2 playerScreenPos = p->getPosition() * PHYSICS_SCALE;
                            bool canDeposit = playerScreenPos.distance(_depositPosition) <= _depositRadius;
                            int slotIndex = de->getSlotIndex();
                            if (slotIndex >= 0) {
                                if (canDeposit) {
                                    int deposited = p->depositItem((size_t)slotIndex);
                                    if (deposited > 0) {
                                        if (p == _localPlayer) {
                                            inventoryValue = _localPlayer->getInventoryVal();
                                        }
                                        pushOutEvent(DepositEvent::allocDepositEvent(p->getPID(), deposited, true, slotIndex));
                                    }
                                } else if ((size_t)slotIndex < p->getInventory().size()) {
                                    auto carried = p->getInventory()[(size_t)slotIndex];
                                    p->depositItem((size_t)slotIndex);
                                    if (p == _localPlayer) {
                                        inventoryValue = _localPlayer->getInventoryVal();
                                    }
                                    auto params = _itemFactory->serializeParams(p->getPosition() + Vec2(0.0f, -0.35f), carried.value, carried.itemType);
                                    auto pair = getPhysController()->addSharedObstacle(_itemFactId, params);
                                    getPhysController()->acquireObs(pair.first, 0);
                                    pushOutEvent(DepositEvent::allocDepositEvent(p->getPID(), 0, true, slotIndex));
                                }
                            } else if (canDeposit) {
                                int deposited = p->depositItems();
                                if (deposited > 0) {
                                    if (p == _localPlayer) {
                                        inventoryValue = _localPlayer->getInventoryVal();
                                    }
                                    pushOutEvent(DepositEvent::allocDepositEvent(p->getPID(), deposited, true));
                                }
                            }
                            break;
                        }
                    }
                } else {
                    depositedValue += de->getAmount();
                }
            } else if (de->isSync()) {
                // Clients only apply deposit updates that came back from host
                for (auto& p : _playerFactory->current) {
                    if (p->getPID() == de->getPID()) {
                        if (de->getSlotIndex() >= 0) {
                            p->depositItem((size_t)de->getSlotIndex());
                            if (p == _localPlayer) {
                                inventoryValue = _localPlayer->getInventoryVal();
                            }
                        }
                        else {
                            p->clearInventory();
                            if (p == _localPlayer) {
                                inventoryValue = _localPlayer->getInventoryVal();
                            }
                        }
                        break;
                    }
                }
                depositedValue += de->getAmount();
            }
        } else if (auto pre = std::dynamic_pointer_cast<PickupEvent>(e)) {
            if (_itemFactory->getItem(pre->getItemID()) != nullptr) {
                CULog("pickup event got!");
                auto item = _itemFactory->getItem(pre->getItemID());
                if (item && !item->isRemoved()) {
                    for (auto& p : _playerFactory->current) {
                        if (p->getPID() == pre->getPID()) {
                            if (p->addItem(item->getItemType(), item->getValue())) {
                                if (p == _localPlayer) {
                                    inventoryValue = _localPlayer->getInventoryVal();
                                }
                                _itemFactory->getItem(pre->getItemID())->markRemoved(true);
                                getPhysController()->removeSharedObstacle(item);
                            }
                            break;
                        }
                    }
                }
                
            }
            
        }
    }
    return std::tuple<int,int>(inventoryValue,depositedValue);
}

void NetworkController::processEndEvent(const std::shared_ptr<EndGameEvent> &e)
{
    _gameState = e->getResult() ? NCGameState::WON : NCGameState::LOST;
    _randSeed = 0;
}

void NetworkController::processRestartEvent(const std::shared_ptr<RestartGameEvent> &e)
{
    _gameState=NCGameState::UNSTARTED;
    _round++;
}



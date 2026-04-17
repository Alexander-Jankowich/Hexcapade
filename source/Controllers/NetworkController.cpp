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
#define PHYSICS_SCALE 100

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
    attachEventType<GameInitEvent>();
    attachEventType<CastSpellEvent>();
    attachEventType<EndGameEvent>();
    attachEventType<DepositEvent>();
    attachEventType<PickupEvent>();
    attachEventType<SetSpellEvent>();
    cugl::physics2::distrib::NetEventController::init(a);
    _randSeed=0;
}

bool NetworkController::connectClient(string roomId) {
    return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
}



std::tuple<int,int> NetworkController::customEvents(std::shared_ptr<ItemFactory> itemFact,std::shared_ptr<PlayerFactory> playerFact,
                                                    std::shared_ptr<PlayerObstacle> _player,InputController& _input, bool* _playerWon, bool* _gameEnded, cugl::Vec2 _depositPosition, float _depositRadius){
    // handle cast spell event
    int inventoryValue = 0;
    int depositedValue = 0;
    while (isInAvailable()){
        auto e = popInEvent();
        if(auto cse = std::dynamic_pointer_cast<CastSpellEvent>(e)){
            CULog("COMBO SPELL GOT");
            processComboEvent(cse,_player, _input);
        } else if (auto cse = std::dynamic_pointer_cast<EndGameEvent>(e)){
            CULog("END GAME GOT");
            processEndEvent(cse,_playerWon, _gameEnded);
        }else if (auto sse = std::dynamic_pointer_cast<SetSpellEvent>(e)){
            //CULog("SET SPELL GOT");
            for (auto& p : playerFact->current) {
                if (p->getPID() == sse->getPID()) {
                    p->setSpell(sse->getSpell());
                    if(sse->getSpell() == SpellType::NONE){
                        p->setAniState(PlayerObstacle::AnimationState::IDLE);
                    }else{
                        p->setAniState(PlayerObstacle::AnimationState::CASTING);
                    }
                    break;
                }
            }
        } else if (auto de = std::dynamic_pointer_cast<DepositEvent>(e)) {
            // 1. Host recieve 2. Host applies deposit 3. Client applies deposit
            if (isHost()) {
                if (!de->isSync()) {
                    // Clients send deposit request and the host sends back the
                    // final synced amount after checking zone and inventory
                    CULog("HOST deposit request");
                    for (auto& p : playerFact->current) {
                        if (p->getPID() == de->getPID()) {
                            Vec2 playerScreenPos = p->getPosition() * PHYSICS_SCALE;
                            bool canDeposit = playerScreenPos.distance(_depositPosition) <= _depositRadius;
                            if (canDeposit) {
                                int slotIndex = de->getSlotIndex();
                                if (slotIndex >= 0) {
                                    int deposited = p->depositItem((size_t)slotIndex);
                                    if (deposited > 0) {
                                        if (_player && p == _player) {
                                            inventoryValue = _player->getInventoryVal();
                                        }
                                        pushOutEvent(DepositEvent::allocDepositEvent(p->getPID(), deposited, true, slotIndex));
                                    }
                                }
                                else {
                                    int deposited = p->depositItems();
                                    if (deposited > 0) {
                                        if (_player && p == _player) {
                                            inventoryValue = _player->getInventoryVal();
                                        }
                                        pushOutEvent(DepositEvent::allocDepositEvent(p->getPID(), deposited, true));
                                    }
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
                for (auto& p : playerFact->current) {
                    if (p->getPID() == de->getPID()) {
                        if (de->getSlotIndex() >= 0) {
                            p->depositItem((size_t)de->getSlotIndex());
                            if (_player && p == _player) {
                                inventoryValue = _player->getInventoryVal();
                            }
                        }
                        else {
                            p->clearInventory();
                            if (_player && p == _player) {
                                inventoryValue = _player->getInventoryVal();
                            }
                        }
                        break;
                    }
                }
                depositedValue += de->getAmount();
            }
        } else if (auto pre = std::dynamic_pointer_cast<PickupEvent>(e)) {
            if (itemFact->getItem(pre->getItemID()) != nullptr) {
                CULog("pickup event got!");
                auto item = itemFact->getItem(pre->getItemID());
                if (item && !item->isRemoved()) {
                    for (auto& p : playerFact->current) {
                        if (p->getPID() == pre->getPID()) {
                            if (p->addItem(item->getItemType(), item->getValue())) {
                                if (_player && p == _player) {
                                    inventoryValue = _player->getInventoryVal();
                                }
                                itemFact->getItem(pre->getItemID())->markRemoved(true);
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
};

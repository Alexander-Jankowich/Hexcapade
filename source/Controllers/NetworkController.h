//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//
#include <cugl/cugl.h>
#include "MessageSerializer.h"

#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

class NetworkController : public cugl::physics2::distrib::NetEventController {
public:

    using InputCallback = std::function<void(const std::string& playerId, const ClientInput&)>;

private:
#pragma mark Attributes
    std::unordered_set<std::string> _players;
    std::string _currHost;
//    InputCallback _inputCallback;
//    bool _handlerAttached = false;
//    void attachReceiptHandler();
//    HostInput _latestHostInput;
//    bool _hasNewHostInput = false;

#pragma mark Helpers
    static std::string dec2hex(const std::string& dec);
    static std::string hex2dec(const std::string& hex);

// TODO clean of all the methods that netevent controller already provides

public:
    NetworkController() {}
    ~NetworkController() {}

    // TODO ADJUST THIS

    std::shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }

//    void setInputCallback(InputCallback cb) {
//        _inputCallback = cb;
//        if (_network) attachReceiptHandler();
//    }
//
//    void sendInput(const ClientInput& input) {
//        if (!_network || _isHost) return;
//        //CULog("CLIENT sending input: joystick=%.2f spellType=%d pickup=%d pos=(%.2f, %.2f)",
//            //input.joystickAngle, input.spellType, input.pickupPressed, input.posX, input.posY);
//        auto bytes = MessageSerializer::encodeClientInput(input);
//        _network->sendToHost(bytes);
//    }
//
//
    void resetStatus() { _status = Status::IDLE; }
    
    std::unordered_set<std::string> getPlayers() {return _network->getPlayers();}
    
    std::string getPID() {return _network->getUUID();}
    
    bool connectClient(std::string roomId) {
        return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
    }

    bool hasNetwork() { return _network != nullptr; }

    std::string getRoomCode() {
        if (!_network) return "";
        std::string room = _network->getRoom();
        if (room.empty()) return "";
        return hex2dec(room);
    }


//    HostInput getLatestHostInput() { return _latestHostInput; }
//    bool hasNewHostInput() { return _hasNewHostInput; }
//    void clearNewHostInput() { _hasNewHostInput = false; }
    
    void endGame(bool won);
};
#endif // !__NETWORK_CONTROLLER_H__


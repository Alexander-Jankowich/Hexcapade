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

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

class NetworkController : public NetEventController {
public:

    using InputCallback = function<void(const string& playerId, const ClientInput&)>;

private:
#pragma mark Attributes
    unordered_set<string> _players;
    string _currHost;
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

    shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }

    void resetStatus() { _status = Status::IDLE; }
    
    unordered_set<string> getPlayers() {return _network->getPlayers();}
    
    string getPID() {return _network->getUUID();}
    
    bool connectClient(string roomId) {
        return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
    }

    bool hasNetwork() { return _network != nullptr; }

    string getRoomCode() {
        if (!_network) return "";
        string room = _network->getRoom();
        if (room.empty()) return "";
        return hex2dec(room);
    }

    void endGame(bool won);
};
#endif // !__NETWORK_CONTROLLER_H__


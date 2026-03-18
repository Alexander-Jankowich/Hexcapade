//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//
#include <cugl/cugl.h>

#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

class NetworkController : public NetEventController {

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
    static string dec2hex(const string& dec);
    static string hex2dec(const string& hex);

// TODO clean of all the methods that netevent controller already provides

public:
    NetworkController() {}
    ~NetworkController() {}

    static shared_ptr<NetworkController> alloc(const std::shared_ptr<AssetManager>& assets) {
        auto f = make_shared<NetworkController>();
        f->init(assets);
        return f;
    }

    // TODO ADJUST THIS

    shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }

    void resetStatus() { _status = Status::IDLE; }
    
    unordered_set<string> getPlayers() {return _network->getPlayers();}
    
    string getPID() {return _network->getUUID();}
    
    bool connectClient(string roomId);

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


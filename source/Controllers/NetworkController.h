//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//
#include <cugl/cugl.h>
#include "../Models/GameInitEvent.h"
#include "../Models/ComboEvent.h"

#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

class NetworkController : public NetEventController {

private:
#pragma mark Attributes
    unordered_set<string> _players;
    string uuid;
    string _currHost;
    unsigned int _randSeed;
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
    void init(const std::shared_ptr<AssetManager>& a);
    // TODO ADJUST THIS
    void setRandSeed(int s) {_randSeed=s;}
    unsigned int getRandSeed() {return _randSeed;}
    shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }

    void resetStatus() { _status = Status::IDLE; }
    
    unordered_set<string> getPlayers() {return _network->getPlayers();}
    
    string getPID() {return _network->getUUID();}
    
    bool connectClient(string roomId);

    bool hasNetwork() { return _network != nullptr; }
    
    string getRoomCode() {
        string room = getRoomID();
        return hex2dec(room);
    }

    void endGame(bool won);
};
#endif // !__NETWORK_CONTROLLER_H__

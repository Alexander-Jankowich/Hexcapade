//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//
#include <cugl/cugl.h>
#include "../Models/Events.h"
#include "../Models/Items.h"
#include "../Models/Players.h"
#include "../Models/Spells.h"
#include "../Models/Enemies.h"
#include "../Scenes/InputDisplay.h"

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


#pragma mark Helpers
    static string dec2hex(const string& dec);
    static string hex2dec(const string& hex);


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
    
    void processComboEvent(const std::shared_ptr<CastSpellEvent>& e, std::shared_ptr<PlayerObstacle> player, InputController input) {
        if (e->getP2() == player->getPID()) {
            input.resolveSpell();
            player->resolveSpell();
        }
    }
    void processEndEvent(const std::shared_ptr<EndGameEvent> &e,bool* _playerWon, bool* _gameEnded)
    {
        *_playerWon = e->getResult();
        *_gameEnded = true;
    }
    
    std::tuple<int,int> customEvents(std::shared_ptr<ItemFactory> itemFact,std::shared_ptr<PlayerFactory> playerFact,
                                     std::shared_ptr<PlayerObstacle> _player, InputController& _input,bool* _playerWon, bool* _gameEnded, cugl::Vec2 _depositPosition, float _depositRadius);
};
#endif // !__NETWORK_CONTROLLER_H__
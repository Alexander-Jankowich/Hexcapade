//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//

#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

#include <cugl/cugl.h>
#include "../Models/Events.h"
#include "../Models/Items/ItemFactory.h"
#include "../Models/Players/PlayerFactory.h"
#include "../Models/Spells/SpellFactory.h"
#include "../Models/Enemies/EnemyFactory.h"
#include "../Scenes/InputDisplay.h"

using namespace cugl;
using namespace std;
using namespace cugl::physics2::distrib;

class NetworkController : public NetEventController {
public:
    enum class NCGameState {UNSTARTED, RUNNING, WON, LOST};
private:
#pragma mark Attributes
    unordered_set<string> _players;
    string uuid;
    string _currHost;
    unsigned int _randSeed;
    shared_ptr<Random> _rng;
    NCGameState _gameState = NCGameState::UNSTARTED;
    int _round;
    unordered_map<string, string> _playerNames;
    unordered_map<string, string> _playerCharacters;
    unordered_map<string, int> _playerDeposits;
    
    // network controller ptrs to models it needs
    shared_ptr<ItemFactory> _itemFactory;
    shared_ptr<PlayerFactory> _playerFactory;
    shared_ptr<EnemyFactory> _enemyFactory;

    Uint32 _itemFactId;
    shared_ptr<PlayerObstacle> _localPlayer;
    InputController* _inputCtrl;
    Vec2 _depositPosition;
    float _depositRadius;
    
    float _worldMaxHeight;

#pragma mark Helpers
    static string dec2hex(const string& dec);
    static string hex2dec(const string& hex);

#pragma mark Process Events
    void processComboEvent(const std::shared_ptr<CastSpellEvent>& e) {
        if (e->getP2() == _localPlayer->getPID()) {
            _inputCtrl->resolveSpell();
            _localPlayer->resolveSpell();
        }
    }
    void processEndEvent(const shared_ptr<EndGameEvent>& e);

    void processRestartEvent(const shared_ptr<RestartGameEvent>& e);

    void processPlayerStateEvent(const shared_ptr<PlayerStateEvent>& e);

    void processEnemyStateEvent(const shared_ptr<EnemyStateEvent>& e);

public:
    NetworkController() {}
    ~NetworkController() {}

    static shared_ptr<NetworkController> alloc(const shared_ptr<AssetManager>& assets) {
        auto f = make_shared<NetworkController>();
        f->init(assets);
        return f;
    }

    void init(const shared_ptr<AssetManager>& a);
    void initGameStuff(shared_ptr<ItemFactory> itemFact, shared_ptr<PlayerFactory> playerFact,
                        shared_ptr<EnemyFactory> enemyFact, Uint32 ifd, shared_ptr<PlayerObstacle> player,
                        InputController& input, Vec2 dp, float dr, float maxHeight){
        _itemFactory = itemFact;
        _playerFactory = playerFact;
        _enemyFactory = enemyFact;
        _itemFactId = ifd;
        _localPlayer = player;
        _inputCtrl = &input;
        _depositPosition = dp;
        _depositRadius = dr;
        _worldMaxHeight = maxHeight;
    }

    unsigned int getRandSeed() {return _randSeed;}
    shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }
    NCGameState gameState() {return _gameState;}
    int getRound() {return _round;}
    bool canHostStart() {return _numReady == getNumPlayers() || getNumPlayers()==1;}
    void unreadyPlayers() {_numReady = 1;} // only called by host, always leave host as ready
    void resetStatus() {
        _randSeed=0;
        _gameState=NCGameState::UNSTARTED;
        _playerNames.clear();
        _playerCharacters.clear();
        _round = 1;
    }
    void rerollRandSeed() { if(_randSeed==0) _randSeed = _rng->getUint32();}
    bool getPhysEnabled() {return _physEnabled;}
    
    unordered_set<string> getPlayers() {return _network->getPlayers();}
    
    string getPID() {return _network->getUUID();}
    
    bool connectClient(string roomId);
    
    void updateNetwork();
    
    bool connectHost() {
        if (_randSeed==0)
            _randSeed = _rng->getUint32();
        if (_status==Status::CONNECTED) return true;
        return connectAsHost();
    }

    bool hasNetwork() { return _network != nullptr; }
    
    string getRoomCode() {
        string room = getRoomID();
        return hex2dec(room);
    }
    
    void resetHost();

    const std::string& getPlayerName(const std::string& pid) const {
        static const std::string fallback = "PLAYER" + to_string(_shortUID);
        auto it = _playerNames.find(pid);
        return it != _playerNames.end() ? it->second : fallback;
    }

    void setPlayerName(const std::string& pid, const std::string& name) {
        _playerNames[pid] = name;
    }

    const std::string& getPlayerCharacter(const std::string& pid) const {
        static const std::string fallback = "player_fox";
        auto it = _playerCharacters.find(pid);
        return it != _playerCharacters.end() ? it->second : fallback;
    }

    void setPlayerCharacter(const std::string& pid, const std::string& character) {
        _playerCharacters[pid] = character;
    }

    int getPlayerDeposit(const std::string& pid) const {
        auto it = _playerDeposits.find(pid);
        return it != _playerDeposits.end() ? it->second : 0;
    }
    
    std::tuple<int,int> customEvents();
};
#endif // !__NETWORK_CONTROLLER_H__

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

class NetworkController {
public:
    enum class Status { IDLE, JOIN, WAIT, START, ABORT };

    using InputCallback = std::function<void(const std::string& playerId, const ClientInput&)>;

private:
#pragma mark Attributes
    std::shared_ptr<cugl::netcode::NetcodeConnection> _network;
    cugl::netcode::NetcodeConfig _config;
    int _playerCount;
    Status _status;
    bool _ishost;
    std::string _uuid;
    std::unordered_set<std::string> _players;
    std::string _currHost;
    InputCallback _inputCallback;
    bool _handlerAttached = false;
    void attachReceiptHandler();

#pragma mark Helpers
    static std::string dec2hex(const std::string& dec);
    static std::string hex2dec(const std::string& hex);
    void dispatcher(const std::string source,
        const std::vector<std::byte>& message);

    bool checkHostConnection();
    bool checkClientConnection();


public:
    NetworkController() : _ishost(false), _playerCount(0), _status(Status::IDLE) {}
    ~NetworkController() {}

    void init(const std::shared_ptr<cugl::JsonValue> json, bool isHost);

    void initWithNetwork(const std::shared_ptr<cugl::netcode::NetcodeConnection> network, bool isHost) {
        _ishost = isHost;
        _network = network;
        _playerCount = network->getNumPlayers();
        // idk what to do about status
    }

    std::shared_ptr<cugl::netcode::NetcodeConnection> getNetwork() {
        return _network;
    }

    void setInputCallback(InputCallback cb) {
        _inputCallback = cb;
        if (_network) attachReceiptHandler();
    }

    void sendInput(const ClientInput& input) {
        if (!_network || _ishost) return;
        CULog("CLIENT sending input: joystick=%.2f spellType=%d pickup=%d pos=(%.2f, %.2f)",
            input.joystickAngle, input.spellType, input.pickupPressed, input.posX, input.posY);
        auto bytes = MessageSerializer::encodeClientInput(input);
        _network->sendToHost(bytes);
    }

    bool isHost() const {
        return _ishost;
    }

    void connect(bool isHost, const std::string& roomCode = "");

    Status getStatus() { return _status; }

    void resetStatus() { _status = Status::IDLE; }

    int playerCount() { return _playerCount; }
    
    std::unordered_set<std::string> getPlayers() {return _players;}
    
    std::string getPID() {return _uuid;}

    bool hasNetwork() { return _network != nullptr; }

    std::string getRoomCode() {
        if (!_network) return "";
        std::string room = _network->getRoom();
        if (room.empty()) return "";
        return hex2dec(room);
    }

    void update() {
        if (!_network) return;
        if (_ishost) checkHostConnection();
        else checkClientConnection();
    }

    void disconnect() {
        if (_network) { _network->close(); }
        _network = nullptr;
        _status = Status::ABORT;
    }


    void startGame();

};
#endif // !__NETWORK_CONTROLLER_H__


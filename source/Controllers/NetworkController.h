//
//  NetworkController.h
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//
#include <cugl/cugl.h>

#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

class NetworkController {
public:
    enum class Status { IDLE, JOIN, WAIT, START, ABORT };

private:
#pragma mark Attributes
    std::shared_ptr<cugl::netcode::NetcodeConnection> _network;
    cugl::netcode::NetcodeConfig _config;
    int _playerCount;
    
    static std::string dec2hex(const std::string& dec);
    static std::string hex2dec(const std::string& hex);

    
public:
    void init(const std::shared_ptr<cugl::JsonValue> json) {_playerCount=1; _config.set(json);}
    std::string connect(bool isHost, const std::string& roomCode="");
    

    int playerCount() {return _playerCount;}
    cugl::netcode::NetcodeConnection::State checkConnection() {return _network->getState();}
    std::shared_ptr<cugl::netcode::NetcodeConnection> getConnection() { return _network; };
    std::string getRoomCode(){return hex2dec(_network->getRoom());}
    void disconnect() {
        if (_network) { _network->close(); }
        _network = nullptr;
    }
    
    void processData(std::vector<std::byte> data);
    
};
#endif // !__NETWORK_CONTROLLER_H__


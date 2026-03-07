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
    Status _status;
    bool _ishost;
    
#pragma mark Helpers
    static std::string dec2hex(const std::string& dec);
    static std::string hex2dec(const std::string& hex);
    void dispatcher(const std::string source,
                    const std::vector<std::byte>& message);
    
    bool checkHostConnection();
    bool checkClientConnection();

    
public:
    NetworkController() {}
    ~NetworkController() {disconnect();}
    
    void init(const std::shared_ptr<cugl::JsonValue> json, bool isHost);
    
    void initWithNetwork(const std::shared_ptr<cugl::netcode::NetcodeConnection> network, bool isHost) {
        _ishost = isHost;
        _network = network;
        _playerCount = network->getNumPlayers();
        // idk what to do about status
    }
    
    void connect(bool isHost, const std::string& roomCode="");
    
    Status getStatus(){return _status;}
    

    int playerCount() {return _playerCount;}

    std::string getRoomCode(){return hex2dec(_network->getRoom());}
    
    
    void disconnect() {
        if (_network) { _network->close(); }
        _network = nullptr;
        _status = Status::ABORT;
    }
        
    void startGame();
    

};
#endif // !__NETWORK_CONTROLLER_H__


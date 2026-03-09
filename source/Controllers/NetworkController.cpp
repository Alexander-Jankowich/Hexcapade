//
//  NetworkController.cpp
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//

#include "NetworkController.h"

using namespace cugl;
using namespace cugl::netcode;


#pragma mark PrivateClientHelpers


std::string NetworkController::dec2hex(const std::string& dec) {
    Uint32 value = strtool::stou32(dec);
    if (value >= 65536) value = 0;
    return strtool::to_hexstring(value, 4);
}

bool NetworkController::checkClientConnection() {
    if (!_network) return false;
    auto state = _network->getState();
    switch (state) {
    case NetcodeConnection::State::NEGOTIATING:
        _status=Status::JOIN;
            break;
    case NetcodeConnection::State::CONNECTED:
        attachReceiptHandler();
        if (_status != Status::START) {
            _status = Status::WAIT;
        }
        _playerCount = (int)_network->getNumPlayers();
        break;

    case NetcodeConnection::State::DENIED:
    case NetcodeConnection::State::MISMATCHED:
    case NetcodeConnection::State::INVALID:
    case NetcodeConnection::State::FAILED:
    case NetcodeConnection::State::DISCONNECTED:
        disconnect();
        _status = Status::IDLE;
        return false;

    default:
        break;
    }
    return true;
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

bool NetworkController::checkHostConnection() {
    if (!_network) return false;

    NetcodeConnection::State state = _network->getState();
    _playerCount = _network->getNumPlayers();

    
    switch (state) {
    case NetcodeConnection::State::NEGOTIATING:
        _status = Status::WAIT;
        break;

    case NetcodeConnection::State::CONNECTED:
        if (_status == Status::WAIT) {
            _status = Status::IDLE;
        }
        break;

    case NetcodeConnection::State::DENIED:
    case NetcodeConnection::State::MISMATCHED:
    case NetcodeConnection::State::INVALID:
    case NetcodeConnection::State::FAILED:
    case NetcodeConnection::State::DISCONNECTED:
        disconnect();
        _status = Status::WAIT;
        return false;

    default:
        break;
    }
    return true;
}


#pragma mark PublicMethods

void NetworkController::init(const std::shared_ptr<cugl::JsonValue> json, bool isHost){
    _playerCount=1;
    _config.set(json);
    _ishost=isHost;
    _status = isHost ? Status::WAIT : Status::IDLE;
}



void NetworkController::connect(bool isHost, const std::string& roomCode){
    _ishost = isHost;
    if(isHost){
        _network = NetcodeConnection::alloc(_config);;
        _network->open();
        checkHostConnection();
    } else {
        std::string hex = dec2hex(roomCode);
        _network = NetcodeConnection::alloc(_config, hex);
        _network->open();
        CULog("connecting client");
        _currHost = _network->getHost(); // this mayyyy have locking issues hopefully not
        checkClientConnection();
    }
    //attachReceiptHandler(); // attach recieve listener when we start up
}

void NetworkController::startGame() {
        if (_network) {
            _network->startSession();
            std::vector<std::byte> msg = { std::byte(255) };
            _network->broadcast(msg);
        }
        _status = Status::START;
}

void NetworkController::attachReceiptHandler() {
    if (!_network || _handlerAttached) return;
    _handlerAttached = true;
    CULog("attachReceiptHandler called, network open=%d", _network->isOpen());
    _network->onReceipt([this](const std::string& source,
        const std::vector<std::byte>& data) {
        if (data.empty()) return;
            if (!_ishost && data[0]==std::byte(255)) {
                // Client path: broadcast from host = game starting
                _status = Status::START;
                checkClientConnection();
            }
            else {
                if (_inputCallback) {
                    ClientInput input;
                    if (MessageSerializer::decodeClientInput(data, input)) {
                        CULog("HOST received input from %s: joystick=%.2f spellType=%d pickup=%d pos=(%.2f, %.2f)",
                            source.c_str(), input.joystickAngle, input.spellType, input.pickupPressed, input.posX, input.posY);
                    }
                }
            }
        });
}

//
//  NetworkController.cpp
//  gp
//
//  Created by Serena Duncan on 3/2/26.
//

#include "NetworkController.h"

using namespace cugl;
using namespace cugl::netcode;
using namespace std;


#pragma mark PrivateClientHelpers


std::string NetworkController::dec2hex(const std::string& dec) {
    Uint32 value = strtool::stou32(dec);
    if (value >= 65536) value = 0;
    return strtool::to_hexstring(value, 4);
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



#pragma mark PublicMethods

bool NetworkController::connectClient(string roomId) {
    return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
}


void NetworkController::endGame(bool won) {
//        if (_network) {
//            CULog("broadcasting end game");
//            std::byte data = won ? std::byte(101) : std::byte(102);
//            std::vector<std::byte> msg =  { data };
//            _network->broadcast(msg);
//        }
//        _status = won ? Status::WON : Status::LOST;
}


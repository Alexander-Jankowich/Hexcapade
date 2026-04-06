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

void NetworkController::init(const std::shared_ptr<AssetManager> &a) {
    attachEventType<GameInitEvent>();
    attachEventType<CastSpellEvent>();
    attachEventType<EndGameEvent>();
    attachEventType<DepositEvent>();
    attachEventType<PickupEvent>();
    attachEventType<SetSpellEvent>();
    cugl::physics2::distrib::NetEventController::init(a);
    _randSeed=0;
}

bool NetworkController::connectClient(string roomId) {
    return connectAsClient(dec2hex(roomId)); // this will become custom translate fn later
}

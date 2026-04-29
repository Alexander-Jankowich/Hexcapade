//
//  PlayerNameEvent.cpp
//  alpha_final
//
//  Created by Frank Xie on 4/27/26.
//

#include "PlayerNameEvent.h"

using namespace cugl::physics2::distrib;

std::shared_ptr<NetEvent> PlayerNameEvent::newEvent() {
    return std::make_shared<PlayerNameEvent>();
}

std::shared_ptr<NetEvent> PlayerNameEvent::allocPlayerNameEvent(
    const std::string& pid, const std::string& name) {
    auto event = std::make_shared<PlayerNameEvent>();
    event->_pid = pid;
    event->_name = name;
    return event;
}

std::vector<std::byte> PlayerNameEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(_pid);
    _serializer.writeString(_name);
    return _serializer.serialize();
}

void PlayerNameEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _pid = _deserializer.readString();
    _name = _deserializer.readString();
}
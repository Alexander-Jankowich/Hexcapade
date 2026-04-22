//
//  DeathEvent.cpp
//  alpha_final
//
//  Created by Alex Jankowich on 4/21/26.
//
#include "DeathEvent.h"
std::shared_ptr<NetEvent> DeathEvent::newEvent() {
    return std::make_shared<DeathEvent>();
}

std::shared_ptr<NetEvent> DeathEvent::allocDeathEvent(const std::string& pid) {
    auto event = std::make_shared<DeathEvent>();
    event->_pid = pid;
    return event;
}

std::vector<std::byte> DeathEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(_pid);
    return _serializer.serialize();
}

void DeathEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _pid = _deserializer.readString();
}

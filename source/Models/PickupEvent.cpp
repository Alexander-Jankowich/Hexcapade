//
//  PickupEvent.cpp
//  tech_final
//
//  Created by Frank Xie on 3/27/26
//
#include "PickupEvent.h"

std::shared_ptr<NetEvent> PickupEvent::newEvent() {
    return std::make_shared<PickupEvent>();
}

std::shared_ptr<NetEvent> PickupEvent::allocPickupEvent(const std::string& pid) {
    auto event = std::make_shared<PickupEvent>();
    event->_pid = pid;
    return event;
}

std::vector<std::byte> PickupEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(_pid);
    return _serializer.serialize();
}

void PickupEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _pid = _deserializer.readString();
}


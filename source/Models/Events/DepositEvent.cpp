//
//  DepositEvent.cpp
//  tech_final
//
//  Created by Frank Xie on 3/24/26
//
#include "DepositEvent.h"

std::shared_ptr<NetEvent> DepositEvent::newEvent() {
    return std::make_shared<DepositEvent>();
}

std::shared_ptr<NetEvent> DepositEvent::allocDepositEvent(int amount) {
    auto event = std::make_shared<DepositEvent>();
    event->_amount = amount;
    return event;
}

std::vector<std::byte> DepositEvent::serialize() {
    _serializer.reset();
    _serializer.writeSint32(_amount);
    return _serializer.serialize();
}

void DepositEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _amount = _deserializer.readSint32();
}

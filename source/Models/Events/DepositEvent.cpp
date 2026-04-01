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

std::shared_ptr<NetEvent> DepositEvent::allocDepositEvent(const std::string& pid, int amount, bool sync) {
    auto event = std::make_shared<DepositEvent>();
    event->_pid = pid;
    event->_amount = amount;
    event->_sync = sync;
    return event;
}

std::vector<std::byte> DepositEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(_pid);
    _serializer.writeSint32(_amount);
    _serializer.writeBool(_sync);
    return _serializer.serialize();
}

void DepositEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _pid = _deserializer.readString();
    _amount = _deserializer.readSint32();
    _sync = _deserializer.readBool();
}

//
//  SetSpellEvent.cpp
//  tech_final
//
//  Created by Alex Jankowich on 4/2/26.
//
#include "SetSpellEvent.h"

using namespace cugl::physics2::distrib;

std::shared_ptr<NetEvent> SetSpellEvent::newEvent() {
    return std::make_shared<SetSpellEvent>();
}

std::shared_ptr<NetEvent> SetSpellEvent::allocSetSpellEvent(std::string pid, SpellType spell) {
    auto event = std::make_shared<SetSpellEvent>();
    event->_pid = pid;
    event->_spell = spell;
    return event;
}

std::vector<std::byte> SetSpellEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(_pid);
    _serializer.writeUint32((uint32_t)_spell);
    return _serializer.serialize();
}

void SetSpellEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    _pid = _deserializer.readString();
    _spell = (SpellType)_deserializer.readUint32();
}

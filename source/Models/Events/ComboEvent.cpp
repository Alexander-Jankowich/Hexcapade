//
//  CastSpellEvent.cpp
//  tech_final
//
//  Created by Serena Duncan on 3/20/26.
//

#include "ComboEvent.h"

using namespace cugl::physics2::distrib;

/**
 * This method is used by the NetEventController to create a new event of using
 * a reference of the same type.
 *
 * Not that this method is not static, it differs from the static alloc() method
 * and all methods must implement this method.
 */
std::shared_ptr<NetEvent> CastSpellEvent::newEvent(){
    return std::make_shared<CastSpellEvent>();
}

std::shared_ptr<NetEvent> CastSpellEvent::allocCastSpellEvent(std::string p1id, std::string p2id, SpellType c){
    auto event = std::make_shared<CastSpellEvent>();
    event->_p1ID = p1id;
    event->_p2ID = p2id;
    event->_combo = c;
    return event;
}

/**
 * Serialize any paramater that the event contains to a vector of bytes.
 */
std::vector<std::byte> CastSpellEvent::serialize(){
    _serializer.reset();
    _serializer.writeString(_p1ID);
    _serializer.writeString(_p2ID);
    _serializer.writeUint32((uint32_t)_combo);
    return _serializer.serialize();
}
/**
 * Deserialize a vector of bytes and set the corresponding parameters.
 *
 * @param data  a byte vector packed by serialize()
 *
 * This function should be the "reverse" of the serialize() function: it
 * should be able to recreate a serialized event entirely, setting all the
 * useful parameters of this class.
 */
void CastSpellEvent::deserialize(const std::vector<std::byte>& data){

    _deserializer.reset();
    _deserializer.receive(data);
    _p1ID = _deserializer.readString();
    _p2ID = _deserializer.readString();
    _combo = (SpellType) _deserializer.readUint32();
}

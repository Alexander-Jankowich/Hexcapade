//
//  RestartGameEvent.cpp
//  alpha_final
//
//  Created by Serena Duncan on 4/18/26.
//

#include "RestartGameEvent.h"

using namespace cugl::physics2::distrib;

/**
 * This method is used by the NetEventController to create a new event of using
 * a reference of the same type.
 *
 * Not that this method is not static, it differs from the static alloc() method
 * and all methods must implement this method.
 */
std::shared_ptr<NetEvent> RestartGameEvent::newEvent(){
    return std::make_shared<RestartGameEvent>();
}

std::shared_ptr<NetEvent> RestartGameEvent::allocRestartGameEvent(int d){
    auto event = std::make_shared<RestartGameEvent>();
    event->_difficulty = d;
    return event;
}

/**
 * Serialize any paramater that the event contains to a vector of bytes.
 */
std::vector<std::byte> RestartGameEvent::serialize(){
    _serializer.reset();
    _serializer.writeUint32(_difficulty);
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
void RestartGameEvent::deserialize(const std::vector<std::byte>& data){
    _deserializer.reset();
    _deserializer.receive(data);
    _difficulty = _deserializer.readUint32();
}

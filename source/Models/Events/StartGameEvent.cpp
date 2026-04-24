//
//  StartGameEvent.cpp
//  alpha_final
//
//  Created by Serena Duncan on 4/20/26.
//

#include "StartGameEvent.h"

using namespace cugl::physics2::distrib;

/**
 * This method is used by the NetEventController to create a new event of using
 * a reference of the same type.
 *
 * Not that this method is not static, it differs from the static alloc() method
 * and all methods must implement this method.
 */
std::shared_ptr<NetEvent> StartGameEvent::newEvent(){
    return std::make_shared<StartGameEvent>();
}

std::shared_ptr<NetEvent> StartGameEvent::allocStartGameEvent(){
    auto event = std::make_shared<StartGameEvent>();
    return event;
}

/**
 * Serialize any paramater that the event contains to a vector of bytes.
 */
std::vector<std::byte> StartGameEvent::serialize(){
    _serializer.reset();
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
void StartGameEvent::deserialize(const std::vector<std::byte>& data){
    _deserializer.reset();
    _deserializer.receive(data);
}

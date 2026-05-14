//States needed to be synced -> health, freeze, buff, #shield, mud, 
//Add isDirty check to playerObstacle
//
//  PlayerStateEvent.h
//  tech_final
//
//  Created by Alex Jankowich on 5/11/26.
//
#include "PlayerStateEvent.h"

using namespace cugl::physics2::distrib;

std::shared_ptr<NetEvent> PlayerStateEvent::newEvent() {
    return std::make_shared<PlayerStateEvent>();
}

std::shared_ptr<NetEvent> PlayerStateEvent::allocPlayerStateEvent(std::string pid, float hp, bool freeze, bool buff, bool mud,  int numShields) {
    auto event = std::make_shared<PlayerStateEvent>();
    event-> pid = pid;
    event -> hp = hp;
    event -> freeze = freeze;
    event -> buff = buff;
    event -> mud = mud;
    event -> numShields = numShields;
    
    return event;
}

std::vector<std::byte> PlayerStateEvent::serialize() {
    _serializer.reset();
    _serializer.writeString(pid);
    _serializer.writeFloat(hp);
    _serializer.writeBool(freeze);
    _serializer.writeBool(buff);
    _serializer.writeBool(mud);
    _serializer.writeSint32(numShields);
                                                  
    return _serializer.serialize();
}

void PlayerStateEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    pid = _deserializer.readString();
    hp = _deserializer.readFloat();
    freeze = _deserializer.readBool();
    buff = _deserializer.readBool();
    mud = _deserializer.readFloat();
    numShields = _deserializer.readSint32();
}

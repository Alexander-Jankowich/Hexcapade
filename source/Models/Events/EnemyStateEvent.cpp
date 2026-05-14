//
//  EnemyStateEvent.h
//  tech_final
//
//  Created by Kaz Tam on 5/13/26.
//
#include "EnemyStateEvent.h"

using namespace cugl::physics2::distrib;

std::shared_ptr<NetEvent> EnemyStateEvent::newEvent() {
    return std::make_shared<EnemyStateEvent>();
}

std::shared_ptr<NetEvent> EnemyStateEvent::alloc(std::shared_ptr<EnemyObstacle> en) {
    auto event = std::make_shared<EnemyStateEvent>();
    event->id = en->ID;
    event->freeze = en->isFroze();
    event->health = en->getHealth();
    event->state = en->getState();

    return event;
}

std::vector<std::byte> EnemyStateEvent::serialize() {
    _serializer.reset();
    _serializer.writeUint32(id);
    _serializer.writeBool(freeze);
    _serializer.writeFloat(health);
    _serializer.writeUint32((uint32) state);

    return _serializer.serialize();
}

void EnemyStateEvent::deserialize(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);

    id = _deserializer.readUint32();
    freeze = _deserializer.readBool();
    health = _deserializer.readFloat();
    state = (EnemyObstacle::EnemyState) _deserializer.readUint32();
}

//
//  EnemyStateEvent.h
//  tech_final
//
//  Created by Kaz Tam on 5/13/26.
//
#ifndef __ENEMY_STATE_EVENT_H__
#define __ENEMY_STATE_EVENT_H__

#include <cugl/cugl.h>
#include "../Enemies/EnemyObstacle.h"
using namespace cugl;
using namespace cugl::physics2::distrib;

class EnemyStateEvent : public NetEvent {

protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

public:
    uint32 id;
    bool freeze;
    float health;
    EnemyObstacle::EnemyState state;

    std::shared_ptr<NetEvent> newEvent() override;

    static std::shared_ptr<NetEvent> alloc(std::shared_ptr<EnemyObstacle> en);

    std::vector<std::byte> serialize() override;

    void deserialize(const std::vector<std::byte>& data) override;

};

#endif // !__ENEMY_STATE_EVENT_H__

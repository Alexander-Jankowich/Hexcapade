//
//  DeathEvent.h
//  alpha_final
//
//  Created by Alex Jankowich on 4/21/26.
//
//
#ifndef __DEATH_EVENT_H__
#define __DEATH_EVENT_H__

#include <cugl/cugl.h>
using namespace cugl;
using namespace cugl::physics2::distrib;

class DeathEvent : public NetEvent {
protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

public:
    std::string _pid;

    std::shared_ptr<NetEvent> newEvent() override;
    static std::shared_ptr<NetEvent> allocDeathEvent(const std::string& pid);

    std::vector<std::byte> serialize() override;
    void deserialize(const std::vector<std::byte>& data) override;

    std::string getPID() const { return _pid; }

};

#endif // !__DEATH_EVENT_H__

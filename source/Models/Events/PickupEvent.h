//
//  PickupEvent.h
//  tech_final
//
//  Created by Frank Xie on 3/27/26
//
#ifndef __NP_PICKUP_EVENT_H__
#define __NP_PICKUP_EVENT_H__

#include <cugl/cugl.h>
using namespace cugl;
using namespace cugl::physics2::distrib;

class PickupEvent : public NetEvent {
protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

public:
    std::string _pid;

    std::shared_ptr<NetEvent> newEvent() override;
    static std::shared_ptr<NetEvent> allocPickupEvent(const std::string& pid);

    std::vector<std::byte> serialize() override;
    void deserialize(const std::vector<std::byte>& data) override;

    std::string getPID() const { return _pid; }
};

#endif // !__NP_PICKUP_EVENT_H__

//
//  PlayerStateEvent.h
//  tech_final
//
//  Created by Alex Jankowich on 5/11/26.
//
#ifndef __PLAYER_STATE_EVENT_H__
#define __PLAYER_STATE_EVENT_H__

#include <cugl/cugl.h>
#include "../Players/PlayerObstacle.h"
using namespace cugl;
using namespace cugl::physics2::distrib;

class PlayerStateEvent : public NetEvent {

protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;
    
public:
    std::string pid;
    float hp;
    bool freeze;
    bool buff;
    bool mud;
    int numShields;
    bool host;
    
    std::shared_ptr<NetEvent> newEvent() override;

    static std::shared_ptr<NetEvent> allocPlayerStateEvent(std::string pid, float hp, bool freeze, bool buff, bool mud,  int numShields, bool host);

    std::vector<std::byte> serialize() override;

    void deserialize(const std::vector<std::byte>& data) override;

};

#endif // !__SET_SPELL_EVENT_H__

//
//  PlayerNameEvent.h
//  alpha_final
//
//  Created by Frank Xie on 4/27/26.
//

#ifndef __NP_PLAYER_NAME_EVENT_H__
#define __NP_PLAYER_NAME_EVENT_H__

#include <cugl/cugl.h>

using namespace cugl;
using namespace cugl::physics2::distrib;

class PlayerNameEvent : public NetEvent {

protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

private:
    std::string _pid;
    std::string _name;

public:
    std::shared_ptr<NetEvent> newEvent() override;

    static std::shared_ptr<NetEvent> allocPlayerNameEvent(
        const std::string& pid, const std::string& name);

    const std::string& getPID()  const { return _pid; }
    const std::string& getName() const { return _name; }

    std::vector<std::byte> serialize() override;
    void deserialize(const std::vector<std::byte>& data) override;
};

#endif // !__NP_PLAYER_NAME_EVENT_H__
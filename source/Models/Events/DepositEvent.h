//
//  DepositEvent.h
//  tech_final
//
//  Created by Frank Xie on 3/24/26
//

#ifndef __NP_DEPOSIT_EVENT_H__
#define __NP_DEPOSIT_EVENT_H__

#include <cugl/cugl.h>
using namespace cugl;
using namespace cugl::physics2::distrib;

class DepositEvent : public NetEvent {
protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

public:
    std::string _pid;
    int _amount;
    bool _sync;
    int _slotIndex;

    std::shared_ptr<NetEvent> newEvent() override;
    static std::shared_ptr<NetEvent> allocDepositEvent(const std::string& pid, int amount = 0, bool sync = false, int slotIndex = -1);

    std::vector<std::byte> serialize() override;
    void deserialize(const std::vector<std::byte>& data) override;

    std::string getPID() const { return _pid; }
    int getAmount() const { return _amount; }
    bool isSync() const { return _sync; }
    int getSlotIndex() const { return _slotIndex; }
};

#endif // !__NP_DEPOSIT_EVENT_H__

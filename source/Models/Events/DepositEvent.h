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
    LWSerializer _serializer;
    LWDeserializer _deserializer;

public:
    int _amount;

    std::shared_ptr<NetEvent> newEvent() override;
    static std::shared_ptr<NetEvent> allocDepositEvent(int amount);

    std::vector<std::byte> serialize() override;
    void deserialize(const std::vector<std::byte>& data) override;

    int getAmount() const { return _amount; }
};

#endif // !__NP_DEPOSIT_EVENT_H__

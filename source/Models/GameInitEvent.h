//
//  GameInitEvent.h
//  tech_final
//
//  Created by Serena Duncan on 3/21/26.
//

#ifndef __NP_GAME_INIT_EVENT_H__
#define __NP_GAME_INIT_EVENT_H__

#include <cugl/cugl.h>

using namespace cugl;
using namespace cugl::physics2::distrib;

class GameInitEvent : public NetEvent {
    
protected:
    LWSerializer _serializer;
    LWDeserializer _deserializer;
    
    
    
public:
    unsigned int _seed;

    /**
     * This method is used by the NetEventController to create a new event of
     * using a reference of the same type.
     *
     * Not that this method is not static, it differs from the static alloc()
     * method and all methods must implement this method.
     */
    std::shared_ptr<NetEvent> newEvent() override;
    
    static std::shared_ptr<NetEvent> allocGameInitEvent(unsigned int seed);
    
    /**
     * Serialize any paramater that the event contains to a vector of bytes.
     */
    std::vector<std::byte> serialize() override;
    
    /**
     * Deserialize a vector of bytes and set the corresponding parameters.
     *
     * @param data  a byte vector packed by serialize()
     *
     * This function should be the "reverse" of the serialize() function: it
     * should be able to recreate a serialized event entirely, setting all the
     * useful parameters of this class.
     */
    void deserialize(const std::vector<std::byte>& data) override;
    
    /** Gets the position of the event. */
    int getSeed() { return _seed; }

};


#endif // !__NP_GAME_INIT_EVENT_H__

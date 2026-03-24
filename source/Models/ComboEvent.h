//
//  CastSpellEvent.h
//  tech_final
//
//  Created by Serena Duncan on 3/20/26.
//

#ifndef __NP_CAST_SPELL_EVENT_H__
#define __NP_CAST_SPELL_EVENT_H__

#include <cugl/cugl.h>
#include "SpellFactory.h"
using namespace cugl;
using namespace cugl::physics2::distrib;

class CastSpellEvent : public NetEvent {
    
protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;
    

    
    
public:
    std::string _p1ID;
    std::string _p2ID;
    SpellType _combo;
    /**
     * This method is used by the NetEventController to create a new event of
     * using a reference of the same type.
     *
     * Not that this method is not static, it differs from the static alloc()
     * method and all methods must implement this method.
     */
    std::shared_ptr<NetEvent> newEvent() override;
    
    static std::shared_ptr<NetEvent> allocCastSpellEvent(std::string p1id, std::string p2id, SpellType c);
    
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
    std::string getP1() { return _p1ID; }
    std::string getP2() { return _p2ID; }
    SpellType getType() { return _combo; }

};


#endif // !__NP_CAST_SPELL_EVENT_H__

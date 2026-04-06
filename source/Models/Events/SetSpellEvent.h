//
//  SetSpellEvent.h
//  tech_final
//
//  Created by Alex Jankowich on 4/2/26.
//
#ifndef __SET_SPELL_EVENT_H__
#define __SET_SPELL_EVENT_H__

#include <cugl/cugl.h>
#include "../Spells.h"
using namespace cugl;
using namespace cugl::physics2::distrib;

class SetSpellEvent : public NetEvent {

protected:
    cugl::netcode::NetcodeSerializer _serializer;
    cugl::netcode::NetcodeDeserializer _deserializer;

public:
    std::string _pid;
    SpellType _spell;

    std::shared_ptr<NetEvent> newEvent() override;

    static std::shared_ptr<NetEvent> allocSetSpellEvent(std::string pid, SpellType spell);

    std::vector<std::byte> serialize() override;

    void deserialize(const std::vector<std::byte>& data) override;

    std::string getPID() { return _pid; }
    SpellType getSpell() { return _spell; }
};

#endif // !__SET_SPELL_EVENT_H__

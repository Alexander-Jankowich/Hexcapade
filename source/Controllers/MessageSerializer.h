#ifndef __MESSAGE_SERIALIZER_H__
#define __MESSAGE_SERIALIZER_H__

#include <cugl/cugl.h>

struct ClientInput {
    float   joystickAngle = 0.0f;
    int     spellType = 0;      // 0=NONE,1=FIREBALL,2=FREEZE
    float   spellAngle = 0.0f;
    bool    pickupPressed = false;
    float   posX = 0.0f;
    float   posY = 0.0f;
};

class MessageSerializer {
public:
    static std::vector<std::byte> encodeClientInput(const ClientInput& input) {
        cugl::netcode::NetcodeSerializer s;
        s.writeFloat(input.joystickAngle);
        s.writeSint32(input.spellType);
        s.writeFloat(input.spellAngle);
        s.writeBool(input.pickupPressed);
        s.writeFloat(input.posX);
        s.writeFloat(input.posY);
        return s.serialize();
    }

    static bool decodeClientInput(const std::vector<std::byte>& data, ClientInput& out) {
        if (data.empty()) return false;
        cugl::netcode::NetcodeDeserializer d;
        d.receive(data);
        out.joystickAngle = d.readFloat();
        out.spellType = d.readSint32();
        out.spellAngle = d.readFloat();
        out.pickupPressed = d.readBool();
        out.posX = d.readFloat();
        out.posY = d.readFloat();
        return true;
    }
};

#endif /* __MESSAGE_SERIALIZER_H__ */
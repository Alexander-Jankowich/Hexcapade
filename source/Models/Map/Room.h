// this model stores one generated room and the room-level spawn/modifier data

#ifndef __HEXCAPADE_ROOM_H__
#define __HEXCAPADE_ROOM_H__

#include <cugl/cugl.h>
#include <string>
#include <vector>

class Room {
public:
    enum class RoomModifier {NONE};
private:
   cugl::Vec2 _topLeftCornerPos;
    RoomModifier _modifier;
    cugl::Size _dimens;

public:
    Room();
    Room(cugl::Vec2 tlc, RoomModifier m, float w, float h);

    const RoomModifier getModifier() const { return _modifier; }
    bool hasModifier() const { return _modifier != RoomModifier::NONE; }
    cugl::Vec2 getTLCornerPos() {return _topLeftCornerPos;}
    cugl::Size getDimensions() {return _dimens;}

    bool contains(const cugl::Vec2& worldPos) const;

};

#endif // __HEXCAPADE_ROOM_H__

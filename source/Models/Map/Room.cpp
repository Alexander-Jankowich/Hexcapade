#include "Room.h"


Room::Room(cugl::Vec2 tlc, RoomModifier m, float w, float h){
    _topLeftCornerPos = tlc;
    _dimens = cugl::Size(w, h);
    _modifier = m;
}

bool Room::contains(const cugl::Vec2& pos) const {
    bool inH = _topLeftCornerPos.y-_dimens.height <= pos.y && _topLeftCornerPos.y >= pos.y;
    bool inW = _topLeftCornerPos.x-_dimens.width <= pos.x && _topLeftCornerPos.x >= pos.x;
    return inH && inW;
}

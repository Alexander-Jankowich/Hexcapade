// this controller exposes room queries off the generated map without making
// gamescene reach into map internals everywhere

#ifndef __HEXCAPADE_ROOM_CONTROLLER_H__
#define __HEXCAPADE_ROOM_CONTROLLER_H__

#include <cugl/cugl.h>

#include "../Models/Map/Room.h"

class RoomController {
private:
    std::vector<std::shared_ptr<Room>> _rooms;

public:
    RoomController() {};
    ~RoomController() {};

    void setRooms(std::vector<std::shared_ptr<Room>> r) {_rooms = r;}
    /**
     * @return shared pointer to Room pos is in, nullptr if pos is not in a room
     */
    std::shared_ptr<Room> getCurrentRoom(const cugl::Vec2& pos) const;



};

#endif // __HEXCAPADE_ROOM_CONTROLLER_H__

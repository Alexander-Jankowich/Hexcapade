// this controller exposes room queries off the generated map without making
// gamescene reach into map internals everywhere

#ifndef __HEXCAPADE_ROOM_CONTROLLER_H__
#define __HEXCAPADE_ROOM_CONTROLLER_H__

#include <cugl/cugl.h>
#include <memory>
#include <string>
#include <vector>

#include "../Models/Map.h"

class RoomController {
private:
    std::shared_ptr<Map> _map;

public:
    RoomController() = default;

    void setMap(const std::shared_ptr<Map>& map) { _map = map; }

    int getCurrentRoomIndex(const cugl::Vec2& worldPos) const;
    const std::vector<int>& getConnectedRooms(int roomIndex) const;

    bool currentRoomHasModifier(const cugl::Vec2& worldPos) const;
    std::string getCurrentRoomModifier(const cugl::Vec2& worldPos) const;

    const std::vector<cugl::Vec2>& getInitialItemSpawnPoints() const;
    const std::vector<cugl::Vec2>& getInitialEnemySpawnPoints() const;
};

#endif // __HEXCAPADE_ROOM_CONTROLLER_H__

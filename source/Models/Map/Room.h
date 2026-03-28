// this model stores one generated room and the room-level spawn/modifier data

#ifndef __HEXCAPADE_ROOM_H__
#define __HEXCAPADE_ROOM_H__

#include <cugl/cugl.h>
#include <string>
#include <vector>

class Room {
private:
    int _id;
    cugl::Rect _bounds;
    cugl::Color4 _debugColor;
    std::vector<int> _connectedRooms;
    std::string _activeModifier;
    std::vector<cugl::Vec2> _itemSpawns;
    std::vector<cugl::Vec2> _enemySpawns;

public:
    Room();
    Room(int id, const cugl::Rect& bounds, const cugl::Color4& debugColor,
         const std::string& activeModifier = "");

    int getId() const { return _id; }
    const cugl::Rect& getBounds() const { return _bounds; }
    const cugl::Color4& getDebugColor() const { return _debugColor; }
    const std::vector<int>& getConnectedRooms() const { return _connectedRooms; }
    const std::string& getActiveModifier() const { return _activeModifier; }
    bool hasActiveModifier() const { return !_activeModifier.empty() && _activeModifier != "none"; }

    bool contains(const cugl::Vec2& worldPos) const;

    void addConnection(int roomId);
    void addItemSpawn(const cugl::Vec2& worldPos);
    void addEnemySpawn(const cugl::Vec2& worldPos);

    const std::vector<cugl::Vec2>& getItemSpawns() const { return _itemSpawns; }
    const std::vector<cugl::Vec2>& getEnemySpawns() const { return _enemySpawns; }
};

#endif // __HEXCAPADE_ROOM_H__

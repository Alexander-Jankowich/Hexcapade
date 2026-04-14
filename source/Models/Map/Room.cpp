#include "Room.h"

Room::Room() :
_id(-1),
_bounds(cugl::Rect::ZERO) {
}

Room::Room(int id, const cugl::Rect& bounds, const cugl::Color4& debugColor,
           const std::string& activeModifier) :
_id(id),
_bounds(bounds),
_activeModifier(activeModifier) {
}

bool Room::contains(const cugl::Vec2& worldPos) const {
    return _bounds.contains(worldPos);
}

void Room::addConnection(int roomId) {
    for (int existing : _connectedRooms) {
        if (existing == roomId) {
            return;
        }
    }
    _connectedRooms.push_back(roomId);
}

void Room::addItemSpawn(const cugl::Vec2& worldPos) {
    _itemSpawns.push_back(worldPos);
}

void Room::addEnemySpawn(const cugl::Vec2& worldPos) {
    _enemySpawns.push_back(worldPos);
}

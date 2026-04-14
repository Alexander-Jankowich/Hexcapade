#include "RoomController.h"

//int RoomController::getCurrentRoomIndex(const cugl::Vec2& worldPos) const {
//    if (!_map) {
//        return -1;
//    }
//    return _map->findRoomIndexForWorld(worldPos);
//}
//
//const std::vector<cugl::Vec2>& RoomController::getInitialItemSpawnPoints() const {
//    static const std::vector<cugl::Vec2> EMPTY;
//    if (!_map) {
//        return EMPTY;
//    }
//    return _map->getItemSpawnPoints();
//}
//
//const std::vector<cugl::Vec2>& RoomController::getInitialEnemySpawnPoints() const {
//    static const std::vector<cugl::Vec2> EMPTY;
//    if (!_map) {
//        return EMPTY;
//    }
//    return _map->getEnemySpawnPoints();
//}
//
//const std::vector<int>& RoomController::getConnectedRooms(int roomIndex) const {
//    static const std::vector<int> EMPTY;
//    if (!_map) {
//        return EMPTY;
//    }
//    return _map->getConnectedRooms(roomIndex);
//}
//
//bool RoomController::currentRoomHasModifier(const cugl::Vec2& worldPos) const {
//    if (!_map) {
//        return false;
//    }
//
//    int roomIndex = _map->findRoomIndexForWorld(worldPos);
//    if (roomIndex < 0 || roomIndex >= (int)_map->getRooms().size()) {
//        return false;
//    }
//    return _map->getRooms()[(size_t)roomIndex].hasActiveModifier();
//}
//
//std::string RoomController::getCurrentRoomModifier(const cugl::Vec2& worldPos) const {
//    if (!_map) {
//        return "";
//    }
//
//    int roomIndex = _map->findRoomIndexForWorld(worldPos);
//    if (roomIndex < 0 || roomIndex >= (int)_map->getRooms().size()) {
//        return "";
//    }
//    return _map->getRooms()[(size_t)roomIndex].getActiveModifier();
//}

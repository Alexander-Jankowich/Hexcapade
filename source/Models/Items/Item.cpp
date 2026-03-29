//
//  Item.cpp
//  Hexcapade
//
//  Shared item constructor implementation.
//
#include "Item.h"

Item::Item(std::string id, std::string name, int value, Kind kind) :
_id(std::move(id)),
_name(std::move(name)),
_value(value),
_kind(kind) {
    // No extra work is required here yet. All item behavior is carried by the
    // concrete subclasses and the inventory/item-set systems above this class.
}

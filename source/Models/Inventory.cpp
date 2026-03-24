//
//  Inventory.cpp
//  Hexcapade
//
//  Per-player inventory lifecycle logic.
//
#include "Inventory.h"

Inventory::Inventory(size_t capacity) : _capacity(capacity) {
}

void Inventory::clear() {
    _items.clear();
}

bool Inventory::canAddItem() const {
    return _items.size() < _capacity;
}

bool Inventory::addItem(const std::shared_ptr<Item>& item) {
    if (!item || !canAddItem()) {
        return false;
    }
    _items.push_back(item);
    return true;
}

std::shared_ptr<Item> Inventory::dropLastItem() {
    if (_items.empty()) {
        return nullptr;
    }
    return dropItemAt(_items.size() - 1);
}

std::shared_ptr<Item> Inventory::dropItemAt(size_t index) {
    if (index >= _items.size()) {
        return nullptr;
    }

    std::shared_ptr<Item> item = _items[index];
    _items.erase(_items.begin() + (int)index);
    return item;
}

int Inventory::depositAllItems() {
    int total = getCarriedValue();
    _items.clear();
    return total;
}

int Inventory::getCarriedValue() const {
    int value = 0;
    for (const std::shared_ptr<Item>& item : _items) {
        value += item->getValue();
    }
    return value;
}

bool Inventory::hasSpell(SpellBook::Spell spell) const {
    for (const std::shared_ptr<Item>& item : _items) {
        if (item->getKind() != Item::Kind::SPELLBOOK) {
            continue;
        }

        std::shared_ptr<SpellBook> spellbook = std::static_pointer_cast<SpellBook>(item);
        if (spellbook->getSpell() == spell) {
            return true;
        }
    }
    return false;
}

bool Inventory::blocksSpellcasting() const {
    for (const std::shared_ptr<Item>& item : _items) {
        if (item->getKind() != Item::Kind::ARTIFACT) {
            continue;
        }

        std::shared_ptr<Artifact> artifact = std::static_pointer_cast<Artifact>(item);
        if (artifact->blocksSpellcasting()) {
            return true;
        }
    }
    return false;
}

float Inventory::getDamageMultiplier() const {
    float multiplier = 1.0f;
    for (const std::shared_ptr<Item>& item : _items) {
        if (item->getKind() != Item::Kind::ARTIFACT) {
            continue;
        }

        std::shared_ptr<Artifact> artifact = std::static_pointer_cast<Artifact>(item);
        multiplier += artifact->getDamageMultiplierModifier();
    }

    if (multiplier < 0.1f) {
        multiplier = 0.1f;
    }
    return multiplier;
}

bool Inventory::hasProtectionArtifact() const {
    for (const std::shared_ptr<Item>& item : _items) {
        if (item->getKind() != Item::Kind::ARTIFACT) {
            continue;
        }

        std::shared_ptr<Artifact> artifact = std::static_pointer_cast<Artifact>(item);
        if (artifact->hasProtectionTrigger()) {
            return true;
        }
    }
    return false;
}

std::vector<Artifact::Effect> Inventory::getActiveArtifactEffects() const {
    std::vector<Artifact::Effect> effects;
    effects.reserve(_items.size());

    for (const std::shared_ptr<Item>& item : _items) {
        if (item->getKind() != Item::Kind::ARTIFACT) {
            continue;
        }

        std::shared_ptr<Artifact> artifact = std::static_pointer_cast<Artifact>(item);
        effects.push_back(artifact->getEffect());
    }

    return effects;
}

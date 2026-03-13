//
//  ItemSet.cpp
//  Hexcapade
//
//  Temporary non-procgen world item container used by the networking item pass.
//
#include "ItemSet.h"

#include <algorithm>
#include <limits>
#include "Artifact.h"
#include "SpellBook.h"

using namespace cugl;

namespace {
    constexpr int GENERIC_ARTIFACT_VALUE = 40;
    constexpr int GENERIC_SPELLBOOK_VALUE = 25;
}

ItemSet::ItemSet() :
_spawnBounds(Rect::ZERO),
_defaultRadius(14.0f),
_nextArtifactIndex(1),
_nextSpellbookIndex(1) {
    std::random_device rd;
    _rng = std::mt19937(rd());
}

void ItemSet::reset() {
    _items.clear();
    _nextArtifactIndex = 1;
    _nextSpellbookIndex = 1;
}

void ItemSet::setSpawnBounds(const Rect& bounds) {
    _spawnBounds = bounds;
}

void ItemSet::spawnInitialPrototypeItems() {
    _items.clear();
    _items.push_back(createRandomItem(true, false));
    _items.push_back(createRandomItem(false, true));
}

void ItemSet::spawnRandomItem() {
    _items.push_back(createRandomItem(false, false));
}

void ItemSet::addDroppedItem(const std::shared_ptr<Item>& item, const Vec2& position, float radius) {
    if (!item) {
        return;
    }

    WorldItem world;
    world.item = item;
    world.position = clampToBounds(position);
    world.radius = radius > 0.0f ? radius : _defaultRadius;
    _items.push_back(world);
}

int ItemSet::findNearestPickupIndex(const Vec2& playerPos, const Size& playerSize) const {
    int bestIndex = -1;
    float bestDistance2 = std::numeric_limits<float>::max();

    for (size_t ii = 0; ii < _items.size(); ++ii) {
        const WorldItem& candidate = _items[ii];
        if (!overlapsRectCircle(playerPos, playerSize, candidate.position, candidate.radius)) {
            continue;
        }

        float dist2 = (candidate.position - playerPos).lengthSquared();
        if (dist2 < bestDistance2) {
            bestDistance2 = dist2;
            bestIndex = (int)ii;
        }
    }

    return bestIndex;
}

std::shared_ptr<Item> ItemSet::takeAt(size_t index, Vec2* outPosition) {
    if (index >= _items.size()) {
        return nullptr;
    }

    if (outPosition != nullptr) {
        *outPosition = _items[index].position;
    }

    std::shared_ptr<Item> picked = _items[index].item;
    _items.erase(_items.begin() + (int)index);
    return picked;
}

ItemSet::WorldItem ItemSet::createRandomItem(bool forceArtifact, bool forceSpellbook) {
    std::uniform_int_distribution<int> kindDist(0, 1);
    bool createArtifact = forceArtifact || (!forceSpellbook && kindDist(_rng) == 0);

    WorldItem world;
    world.position = createRandomPosition();
    world.radius = _defaultRadius;

    if (createArtifact) {
        std::string id = "artifact_" + std::to_string(_nextArtifactIndex);
        std::string name = "artifact" + std::to_string(_nextArtifactIndex);
        _nextArtifactIndex++;
        world.item = std::make_shared<Artifact>(
            id, name, GENERIC_ARTIFACT_VALUE, Artifact::Effect::ARTIFACT_EFFECT
        );
    } else {
        std::string id = "spellbook_" + std::to_string(_nextSpellbookIndex);
        std::string name = "spellbook" + std::to_string(_nextSpellbookIndex);
        _nextSpellbookIndex++;
        world.item = std::make_shared<SpellBook>(
            id, name, GENERIC_SPELLBOOK_VALUE, SpellBook::Spell::SPELL_EFFECT
        );
    }

    return world;
}

Vec2 ItemSet::createRandomPosition() {
    float margin = 32.0f;

    float minX = _spawnBounds.origin.x + margin;
    float minY = _spawnBounds.origin.y + margin;
    float maxX = _spawnBounds.origin.x + _spawnBounds.size.width - margin;
    float maxY = _spawnBounds.origin.y + _spawnBounds.size.height - margin;

    if (maxX < minX) {
        maxX = minX;
    }
    if (maxY < minY) {
        maxY = minY;
    }

    std::uniform_real_distribution<float> xdist(minX, maxX);
    std::uniform_real_distribution<float> ydist(minY, maxY);
    return Vec2(xdist(_rng), ydist(_rng));
}

Vec2 ItemSet::clampToBounds(const Vec2& pos) const {
    float minX = _spawnBounds.origin.x;
    float minY = _spawnBounds.origin.y;
    float maxX = _spawnBounds.origin.x + _spawnBounds.size.width;
    float maxY = _spawnBounds.origin.y + _spawnBounds.size.height;

    return Vec2(
        clampf(pos.x, minX, maxX),
        clampf(pos.y, minY, maxY)
    );
}

bool ItemSet::overlapsRectCircle(const Vec2& rectCenter, const Size& rectSize,
                                 const Vec2& circleCenter, float radius) {
    Vec2 half = rectSize / 2;
    Vec2 minCorner = rectCenter - half;
    Vec2 maxCorner = rectCenter + half;

    Vec2 closest(
        std::max(minCorner.x, std::min(circleCenter.x, maxCorner.x)),
        std::max(minCorner.y, std::min(circleCenter.y, maxCorner.y))
    );

    return (closest - circleCenter).lengthSquared() <= radius * radius;
}

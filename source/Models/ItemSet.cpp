// this file handles world item seeding, pickup targeting, and moving item data
// between world space and inventory space

#include "ItemSet.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include "Artifact.h"
#include "SpellBook.h"

using namespace cugl;

namespace {
    constexpr float PI_F = 3.14159265f;

    ItemCatalogEntryConfig makeFallbackEntry(const std::string& kind,
                                             const std::string& id,
                                             const std::string& name,
                                             int value,
                                             float weight) {
        ItemCatalogEntryConfig entry;
        entry.kind = kind;
        entry.id = id;
        entry.name = name;
        entry.value = value;
        entry.weight = weight;
        return entry;
    }
}

ItemSet::ItemSet() :
_spawnBounds(Rect::ZERO),
_defaultRadius(14.0f),
_spawnConfig(),
_nextArtifactIndex(1),
_nextSpellbookIndex(1) {
    std::random_device rd;
    _rng = std::mt19937(rd());

    _spawnConfig.entries = {
        makeFallbackEntry("artifact", "artifact_scroll", "Scroll", 60, 1.0f),
        makeFallbackEntry("artifact", "artifact_gem", "Gem", 80, 0.8f),
        makeFallbackEntry("spellbook", "spellbook_fire", "Fire Spellbook", 25, 1.0f),
        makeFallbackEntry("spellbook", "spellbook_freeze", "Freeze Spellbook", 25, 1.0f),
        makeFallbackEntry("spellbook", "spellbook_heal", "Heal Spellbook", 25, 1.0f)
    };
    rebuildCatalogCaches();
}

void ItemSet::reset() {
    _items.clear();
    _nextArtifactIndex = 1;
    _nextSpellbookIndex = 1;
}

void ItemSet::setSpawnBounds(const Rect& bounds) {
    _spawnBounds = bounds;
}

void ItemSet::setSpawnConfig(const ItemSpawnConfig& config) {
    _spawnConfig = config;
    _defaultRadius = std::max(6.0f, _spawnConfig.defaultPickupRadius);
    if (_spawnConfig.entries.empty()) {
        _spawnConfig.entries = {
            makeFallbackEntry("artifact", "artifact_scroll", "Scroll", 60, 1.0f),
            makeFallbackEntry("artifact", "artifact_gem", "Gem", 80, 0.8f),
            makeFallbackEntry("spellbook", "spellbook_fire", "Fire Spellbook", 25, 1.0f),
            makeFallbackEntry("spellbook", "spellbook_freeze", "Freeze Spellbook", 25, 1.0f),
            makeFallbackEntry("spellbook", "spellbook_heal", "Heal Spellbook", 25, 1.0f)
        };
    }
    rebuildCatalogCaches();
}

int ItemSet::getTotalWorldValue() const {
    int total = 0;
    for (const WorldItem& world : _items) {
        total += world.item ? world.item->getValue() : 0;
    }
    return total;
}

void ItemSet::spawnInitialPrototypeItems() {
    _items.clear();
    _items.push_back(createRandomItem(true, false));
    _items.push_back(createRandomItem(false, true));
}

void ItemSet::spawnRandomItem() {
    _items.push_back(createRandomItem(false, false));
}

void ItemSet::spawnFromRoomPositions(const std::vector<Vec2>& positions) {
    _items.clear();
    if (positions.empty()) {
        return;
    }

    int guaranteedArtifacts = std::max(0, _spawnConfig.guaranteedArtifacts);
    int guaranteedSpellbooks = std::max(0, _spawnConfig.guaranteedSpellbooks);

    for (size_t ii = 0; ii < positions.size(); ++ii) {
        bool forceArtifact = ((int)ii < guaranteedArtifacts);
        bool forceSpellbook = ((int)ii >= guaranteedArtifacts &&
                               (int)ii < guaranteedArtifacts + guaranteedSpellbooks);
        _items.push_back(createItemAtPosition(positions[ii], forceArtifact, forceSpellbook));
    }
}

void ItemSet::spawnUntilTotalValueAtLeast(int targetTotalValue, int maxAdditionalItems) {
    (void)spawnUntilTotalValueAtLeast(std::vector<Vec2>(), targetTotalValue, maxAdditionalItems);
}

int ItemSet::spawnUntilTotalValueAtLeast(const std::vector<Vec2>& positions,
                                         int targetTotalValue,
                                         int maxAdditionalItems) {
    int target = std::max(0, targetTotalValue);
    int cap = std::max(0, maxAdditionalItems);
    if (target <= 0 || cap <= 0) {
        return 0;
    }

    int runningTotal = getTotalWorldValue();
    if (runningTotal >= target) {
        return 0;
    }

    int added = 0;
    std::uniform_int_distribution<size_t> positionDist(0, positions.empty() ? 0 : positions.size() - 1);
    while (runningTotal < target && added < cap) {
        WorldItem world;
        if (positions.empty()) {
            world = createRandomItem(false, false);
        } else {
            world = createItemAtPosition(positions[positionDist(_rng)], false, false);
        }

        if (world.item) {
            runningTotal += std::max(0, world.item->getValue());
        }
        _items.push_back(world);
        added++;
    }
    return added;
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
    return createItemAtPosition(createRandomPosition(), forceArtifact, forceSpellbook);
}

ItemSet::WorldItem ItemSet::createItemAtPosition(const Vec2& position, bool forceArtifact, bool forceSpellbook) {
    std::uniform_int_distribution<int> kindDist(0, 1);
    bool createArtifact = forceArtifact;
    if (!forceArtifact && !forceSpellbook) {
        if (_artifactCatalog.empty() && !_spellbookCatalog.empty()) {
            createArtifact = false;
        } else if (!_artifactCatalog.empty() && _spellbookCatalog.empty()) {
            createArtifact = true;
        } else {
            createArtifact = kindDist(_rng) == 0;
        }
    }
    if (forceSpellbook) {
        createArtifact = false;
    }

    WorldItem world;
    world.radius = _defaultRadius;
    world.position = findOpenSpawnPosition(position, world.radius);

    if (createArtifact) {
        const ItemCatalogEntryConfig* entry = chooseCatalogEntry(_artifactCatalog);
        std::string id = "artifact_" + std::to_string(_nextArtifactIndex);
        std::string name = "artifact" + std::to_string(_nextArtifactIndex);
        int value = 60;
        if (entry) {
            if (!entry->id.empty()) {
                id = entry->id + "_" + std::to_string(_nextArtifactIndex);
            }
            if (!entry->name.empty()) {
                name = entry->name;
            }
            value = std::max(1, entry->value);
        }
        _nextArtifactIndex++;
        world.item = std::make_shared<Artifact>(id, name, value, Artifact::Effect::ARTIFACT_EFFECT);
    } else {
        const ItemCatalogEntryConfig* entry = chooseCatalogEntry(_spellbookCatalog);
        std::string id = "spellbook_" + std::to_string(_nextSpellbookIndex);
        std::string name = "spellbook" + std::to_string(_nextSpellbookIndex);
        int value = 25;
        if (entry) {
            if (!entry->id.empty()) {
                id = entry->id + "_" + std::to_string(_nextSpellbookIndex);
            }
            if (!entry->name.empty()) {
                name = entry->name;
            }
            value = std::max(1, entry->value);
        }
        _nextSpellbookIndex++;
        // ga: procnet later
        // spell-specific data is already in the config entry ids/names, but
        // the current networking item classes still use the placeholder enum
        world.item = std::make_shared<SpellBook>(id, name, value, SpellBook::Spell::SPELL_EFFECT);
    }

    return world;
}

const ItemCatalogEntryConfig* ItemSet::chooseCatalogEntry(const std::vector<ItemCatalogEntryConfig>& catalog) {
    if (catalog.empty()) {
        return nullptr;
    }

    float totalWeight = 0.0f;
    for (const ItemCatalogEntryConfig& entry : catalog) {
        totalWeight += std::max(0.0f, entry.weight);
    }

    if (totalWeight <= 0.0001f) {
        std::uniform_int_distribution<size_t> indexDist(0, catalog.size() - 1);
        return &catalog[indexDist(_rng)];
    }

    std::uniform_real_distribution<float> rollDist(0.0f, totalWeight);
    float roll = rollDist(_rng);
    float cursor = 0.0f;
    for (const ItemCatalogEntryConfig& entry : catalog) {
        cursor += std::max(0.0f, entry.weight);
        if (roll <= cursor) {
            return &entry;
        }
    }
    return &catalog.back();
}

void ItemSet::rebuildCatalogCaches() {
    _artifactCatalog.clear();
    _spellbookCatalog.clear();

    for (const ItemCatalogEntryConfig& entry : _spawnConfig.entries) {
        std::string kind = entry.kind;
        std::transform(kind.begin(), kind.end(), kind.begin(), [](unsigned char c) {
            return (char)std::tolower(c);
        });
        if (kind == "artifact") {
            _artifactCatalog.push_back(entry);
        } else if (kind == "spellbook") {
            _spellbookCatalog.push_back(entry);
        }
    }
}

bool ItemSet::overlapsExistingItems(const Vec2& position, float radius, float spacingPadding) const {
    float candidateRadius = std::max(1.0f, radius);
    float padding = std::max(0.0f, spacingPadding);
    for (const WorldItem& world : _items) {
        float otherRadius = std::max(1.0f, world.radius);
        float minDistance = candidateRadius + otherRadius + padding;
        if ((world.position - position).lengthSquared() < minDistance * minDistance) {
            return true;
        }
    }
    return false;
}

Vec2 ItemSet::findOpenSpawnPosition(const Vec2& preferred, float radius) {
    Vec2 clamped = clampToBounds(preferred);
    float safeRadius = std::max(1.0f, radius);
    float spacingPadding = std::max(6.0f, safeRadius * 0.55f);

    if (!overlapsExistingItems(clamped, safeRadius, spacingPadding)) {
        return clamped;
    }

    float minJitter = std::max(12.0f, safeRadius * 1.2f);
    float maxJitter = std::max(minJitter, safeRadius * 3.2f);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI_F);
    std::uniform_real_distribution<float> distanceDist(minJitter, maxJitter);

    for (int attempt = 0; attempt < 32; ++attempt) {
        float angle = angleDist(_rng);
        float distance = distanceDist(_rng);
        Vec2 offset(std::cos(angle) * distance, std::sin(angle) * distance);
        Vec2 candidate = clampToBounds(clamped + offset);
        if (!overlapsExistingItems(candidate, safeRadius, spacingPadding)) {
            return candidate;
        }
    }

    for (int attempt = 0; attempt < 24; ++attempt) {
        Vec2 candidate = createRandomPosition();
        if (!overlapsExistingItems(candidate, safeRadius, spacingPadding)) {
            return candidate;
        }
    }

    return clamped;
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

    return Vec2(clampf(pos.x, minX, maxX), clampf(pos.y, minY, maxY));
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

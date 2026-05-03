#include "ItemSpawning.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace hexcapade {
namespace itemspawn {

// picks one item from an already-filtered list using catalog weights
const ItemCatalogEntryConfig* chooseWeight(const std::vector<ItemCatalogEntryConfig>& weightedEntries,
                                           const std::shared_ptr<cugl::Random>& random) {
    float weightSum = 0.0f;
    for (const auto& entry : weightedEntries) {
        weightSum += entry.weight;
    }

    float remainingRoll = random->getClosedFloat(0.0f, weightSum);
    for (const auto& entry : weightedEntries) {
        if ((remainingRoll -= entry.weight) <= 0.0f) {
            return &entry;
        }
    }

    return &weightedEntries.back();
}

// checks if an item fits this zone's value band
bool isItemEligibleForZone(const ItemCatalogEntryConfig& item,
                           int maxItemValue,
                           float zoneMinRatio,
                           float zoneMaxRatio) {
    float zoneMinValue = zoneMinRatio * maxItemValue;
    float zoneMaxValue = zoneMaxRatio * maxItemValue;
    bool includesMaxValue = zoneMaxRatio >= 1.0f;
    return item.value >= zoneMinValue &&
        (includesMaxValue ? item.value <= zoneMaxValue : item.value < zoneMaxValue);
}

// rolls spellbook vs artifact from zone probabilities
std::string chooseItemCategory(float spellbookChance,
                               float artifactChance,
                               const std::shared_ptr<cugl::Random>& random) {
    float totalChance = spellbookChance + artifactChance;
    float categoryRoll = random->getClosedFloat(0.0f, totalChance);
    return categoryRoll <= spellbookChance ? "spellbook" : "artifact";
}

float getTypicalItemValue(const std::vector<ItemCatalogEntryConfig>& catalogEntries) {
    float weightSum = 0.0f;
    float weightedValueSum = 0.0f;
    for (const auto& entry : catalogEntries) {
        weightSum += entry.weight;
        weightedValueSum += entry.weight * entry.value;
    }

    return weightedValueSum / weightSum;
}

int getMaxItemValue(const std::vector<ItemCatalogEntryConfig>& catalogEntries) {
    int maxValue = 0;
    for (const auto& entry : catalogEntries) {
        maxValue = std::max(maxValue, entry.value);
    }
    return maxValue;
}

size_t chooseSpawnCount(size_t spawnPointCount,
                        size_t roomCount,
                        int quota,
                        float typicalItemValue,
                        float itemValueRatio) {
    float targetItemValue = (float)quota * itemValueRatio;
    size_t itemsNeeded = (size_t)std::ceil(targetItemValue / typicalItemValue);
    return std::min(spawnPointCount, std::max(roomCount, itemsNeeded));
}

ItemCatalogEntryConfig chooseItemForZone(const std::vector<ItemCatalogEntryConfig>& catalogEntries,
                                         int maxItemValue,
                                         float zoneMinRatio,
                                         float zoneMaxRatio,
                                         float spellbookChance,
                                         float artifactChance,
                                         const std::shared_ptr<cugl::Random>& random) {
    std::string itemCategory = chooseItemCategory(spellbookChance, artifactChance, random);
    std::vector<ItemCatalogEntryConfig> eligibleEntries;
    eligibleEntries.reserve(catalogEntries.size());
    for (const auto& entry : catalogEntries) {
        if (entry.kind == itemCategory &&
                isItemEligibleForZone(entry, maxItemValue, zoneMinRatio, zoneMaxRatio)) {
            eligibleEntries.push_back(entry);
        }
    }

    return *chooseWeight(eligibleEntries, random);
}

} // namespace itemspawn
} // namespace hexcapade

#ifndef __HEXCAPADE_ITEM_SPAWNING_H__
#define __HEXCAPADE_ITEM_SPAWNING_H__

#include <cugl/cugl.h>
#include <cstddef>
#include <string>
#include <vector>

#include "../GameplayConfig.h"

namespace hexcapade {
namespace itemspawn {

// keeping these (chooseWeight, isItemEligibleForZone, chooseItemCategory) in the h just in case another file needs them. we can move them into the cpp if we want.

// picks one item from an already-filtered list using catalog weights
const ItemCatalogEntryConfig* chooseWeight(const std::vector<ItemCatalogEntryConfig>& weightedEntries,
                                           const std::shared_ptr<cugl::Random>& random);

// estimates average item value from the full catalog
float getTypicalItemValue(const std::vector<ItemCatalogEntryConfig>& catalogEntries);

// finds the catalog max so zone bands can use ratios
int getMaxItemValue(const std::vector<ItemCatalogEntryConfig>& catalogEntries);

// checks if an item fits this zone's value band
bool isItemEligibleForZone(const ItemCatalogEntryConfig& item,
                           int maxItemValue,
                           float zoneMinRatio,
                           float zoneMaxRatio);

// rolls spellbook vs artifact from zone probabilities
std::string chooseItemCategory(float spellbookChance,
                               float artifactChance,
                               const std::shared_ptr<cugl::Random>& random);

// does: decides how many spawn item points to populate
size_t chooseSpawnCount(size_t spawnPointCount,
                        size_t roomCount,
                        int quota,
                        float typicalItemValue,
                        float itemValueRatio);

// full item choice for one selected map point: value band, category roll, then weight
ItemCatalogEntryConfig chooseItemForZone(const std::vector<ItemCatalogEntryConfig>& catalogEntries,
                                         int maxItemValue,
                                         float zoneMinRatio,
                                         float zoneMaxRatio,
                                         float spellbookChance,
                                         float artifactChance,
                                         const std::shared_ptr<cugl::Random>& random);

} // namespace itemspawn
} // namespace hexcapade

#endif // __HEXCAPADE_ITEM_SPAWNING_H__

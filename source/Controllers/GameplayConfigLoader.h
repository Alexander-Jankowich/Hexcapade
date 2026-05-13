// this file translates json tuning into typed gameplay config data so
// gamescene wiring can stay focused on runtime flow

#ifndef __HEXCAPADE_GAMEPLAY_CONFIG_LOADER_H__
#define __HEXCAPADE_GAMEPLAY_CONFIG_LOADER_H__

#include <cugl/cugl.h>
#include <cugl/core/io/CUJsonReader.h>
#include <algorithm>
#include <string>
#include <vector>

#include "GameplayConfig.h"

namespace hexcapade {
namespace gameplayconfig {

inline std::shared_ptr<cugl::JsonValue> loadAssetJson(const std::string& path) {
    std::shared_ptr<cugl::JsonReader> reader = cugl::JsonReader::allocWithAsset(path);
    if (!reader) {
        return nullptr;
    }
    return reader->readJson();
}

inline int getIntMin(const std::shared_ptr<cugl::JsonValue>& node,
                     const std::string& key,
                     int defaultValue,
                     int minValue) {
    if (!node) {
        return defaultValue;
    }
    return std::max(minValue, node->getInt(key, defaultValue));
}

inline float getFloatMin(const std::shared_ptr<cugl::JsonValue>& node,
                         const std::string& key,
                         float defaultValue,
                         float minValue) {
    if (!node) {
        return defaultValue;
    }
    return std::max(minValue, node->getFloat(key, defaultValue));
}

inline cugl::Color4 readColor4(const std::shared_ptr<cugl::JsonValue>& node,
                               const cugl::Color4& defaultValue) {
    if (!node || node->size() < 3) {
        return defaultValue;
    }

    int r = std::clamp(node->get(0)->asInt(defaultValue.r), 0, 255);
    int g = std::clamp(node->get(1)->asInt(defaultValue.g), 0, 255);
    int b = std::clamp(node->get(2)->asInt(defaultValue.b), 0, 255);
    int a = defaultValue.a;
    if (node->size() >= 4) {
        a = std::clamp(node->get(3)->asInt(defaultValue.a), 0, 255);
    }
    return cugl::Color4((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

inline void parseProceduralConfig(const std::shared_ptr<cugl::JsonValue>& root, RunBalanceConfig& config) {
    if (!root) {
        return;
    }

    config.quota = getIntMin(root, "quota", config.quota, 1);

    std::shared_ptr<cugl::JsonValue> mapNode = root->has("map") ? root->get("map") : root;
    if (!mapNode) {
        return;
    }

    config.map.roomCountMin = getIntMin(mapNode, "room_count_min", config.map.roomCountMin, 1);
    config.map.roomCountMax = getIntMin(mapNode, "room_count_max", config.map.roomCountMax, config.map.roomCountMin);
    config.map.roomPlacementAttempts = getIntMin(mapNode, "room_placement_attempts", config.map.roomPlacementAttempts, 1);

    config.map.tileSize = getFloatMin(mapNode, "tile_size", config.map.tileSize, 8.0f);
    config.map.worldMinGridWidth = getIntMin(mapNode, "world_min_grid_width", config.map.worldMinGridWidth, 16);
    config.map.worldMinGridHeight = getIntMin(mapNode, "world_min_grid_height", config.map.worldMinGridHeight, 16);

    config.map.roomWidthMin = getIntMin(mapNode, "room_width_min", config.map.roomWidthMin, 4);
    config.map.roomWidthMax = getIntMin(mapNode, "room_width_max", config.map.roomWidthMax, config.map.roomWidthMin);
    config.map.roomHeightMin = getIntMin(mapNode, "room_height_min", config.map.roomHeightMin, 4);
    config.map.roomHeightMax = getIntMin(mapNode, "room_height_max", config.map.roomHeightMax, config.map.roomHeightMin);
    config.map.roomBufferTiles = getIntMin(mapNode, "room_buffer_tiles", config.map.roomBufferTiles, 0);
    config.map.roomEdgePadding = getIntMin(mapNode, "room_edge_padding", config.map.roomEdgePadding, 1);

    config.map.hallThickness = getIntMin(mapNode, "hall_thickness", config.map.hallThickness, 1);
    config.map.maxHallwayLengthTiles = getIntMin(mapNode, "max_hallway_length_tiles",
                                                 config.map.maxHallwayLengthTiles, 1);
    config.map.extraHallConnectionStep = getIntMin(mapNode, "extra_hall_connection_step",
                                                   config.map.extraHallConnectionStep, 2);

    /*
    // old obstacle tuning loader kept as reference with the older obstacle path.
    config.map.obstacleCountMin = getIntMin(mapNode, "obstacle_count_min", config.map.obstacleCountMin, 0);
    config.map.obstacleCountMax = getIntMin(mapNode, "obstacle_count_max", config.map.obstacleCountMax, config.map.obstacleCountMin);
    config.map.obstacleSizeMin = getIntMin(mapNode, "obstacle_size_min", config.map.obstacleSizeMin, 1);
    config.map.obstacleSizeMax = getIntMin(mapNode, "obstacle_size_max", config.map.obstacleSizeMax, config.map.obstacleSizeMin);
    config.map.obstacleMarginTiles = getIntMin(mapNode, "obstacle_margin_tiles", config.map.obstacleMarginTiles, 1);
    config.map.obstacleCenterClearanceTiles = getIntMin(mapNode, "obstacle_center_clearance_tiles",
                                                        config.map.obstacleCenterClearanceTiles, 0);
    config.map.obstaclePlacementAttempts = getIntMin(mapNode, "obstacle_placement_attempts",
                                                     config.map.obstaclePlacementAttempts, 1);
    */

    config.map.largeRoomWidthThreshold = getIntMin(mapNode, "large_room_width_threshold",
                                                   config.map.largeRoomWidthThreshold, 1);
    config.map.largeRoomHeightThreshold = getIntMin(mapNode, "large_room_height_threshold",
                                                    config.map.largeRoomHeightThreshold, 1);
    config.map.primaryItemOffsetX = mapNode->getInt("primary_item_offset_x", config.map.primaryItemOffsetX);
    config.map.primaryItemOffsetY = mapNode->getInt("primary_item_offset_y", config.map.primaryItemOffsetY);
    config.map.secondaryItemOffsetX = mapNode->getInt("secondary_item_offset_x", config.map.secondaryItemOffsetX);
    config.map.secondaryItemOffsetY = mapNode->getInt("secondary_item_offset_y", config.map.secondaryItemOffsetY);
    config.map.enemyOffsetX = mapNode->getInt("enemy_offset_x", config.map.enemyOffsetX);
    config.map.enemyOffsetY = mapNode->getInt("enemy_offset_y", config.map.enemyOffsetY);
    config.map.skipEnemySpawnsInFirstRooms = getIntMin(mapNode, "skip_enemy_spawns_in_first_rooms",
                                                       config.map.skipEnemySpawnsInFirstRooms, 0);

    std::shared_ptr<cugl::JsonValue> colorsNode = mapNode->get("colors");
    if (colorsNode) {
        config.map.backdropColor = readColor4(colorsNode->get("backdrop"), config.map.backdropColor);
        config.map.hallFloorColor = readColor4(colorsNode->get("hall_floor"), config.map.hallFloorColor);
        config.map.wallFaceColor = readColor4(colorsNode->get("wall_face"), config.map.wallFaceColor);
        config.map.wallTopColor = readColor4(colorsNode->get("wall_top"), config.map.wallTopColor);
        /*
        // old obstacle colors kept with the older obstacle path.
        config.map.obstacleFaceColor = readColor4(colorsNode->get("obstacle_face"), config.map.obstacleFaceColor);
        config.map.obstacleTopColor = readColor4(colorsNode->get("obstacle_top"), config.map.obstacleTopColor);
        */

        std::shared_ptr<cugl::JsonValue> paletteNode = colorsNode->get("room_palette");
        if (paletteNode && paletteNode->size() > 0) {
            std::vector<cugl::Color4> palette;
            for (size_t ii = 0; ii < paletteNode->size(); ++ii) {
                palette.push_back(readColor4(paletteNode->get((int)ii), cugl::Color4(118, 146, 108, 255)));
            }
            if (!palette.empty()) {
                config.map.roomPalette = palette;
            }
        }
    }
}

inline void parseRoomTemplateConfig(const std::shared_ptr<cugl::JsonValue>& root, RunBalanceConfig& config) {
    if (!root) {
        return;
    }

    std::shared_ptr<cugl::JsonValue> templatesNode = root->has("room_templates") ? root->get("room_templates") : root;
    if (!templatesNode || templatesNode->size() == 0) {
        return;
    }

    std::vector<RoomTemplateConfig> loaded;
    loaded.reserve(templatesNode->size());
    for (size_t ii = 0; ii < templatesNode->size(); ++ii) {
        std::shared_ptr<cugl::JsonValue> entry = templatesNode->get((int)ii);
        if (!entry) {
            continue;
        }

        RoomTemplateConfig templ;
        templ.roomId = entry->getString("room_id", "room_" + std::to_string(ii));
        templ.width = getIntMin(entry, "width", config.map.roomWidthMin, 4);
        templ.height = getIntMin(entry, "height", config.map.roomHeightMin, 4);
        if (entry->has("color")) {
            templ.hasColor = true;
            templ.color = readColor4(entry->get("color"), templ.color);
        }

        std::shared_ptr<cugl::JsonValue> modifiersNode = entry->get("possible_modifiers");
        if (modifiersNode) {
            for (size_t jj = 0; jj < modifiersNode->size(); ++jj) {
                std::shared_ptr<cugl::JsonValue> modifierNode = modifiersNode->get((int)jj);
                if (!modifierNode) {
                    continue;
                }
                std::string modifier = modifierNode->asString("");
                if (!modifier.empty()) {
                    templ.possibleModifiers.push_back(modifier);
                }
            }
        }
        loaded.push_back(templ);
    }

    if (!loaded.empty()) {
        config.map.roomTemplates = loaded;
    }
}

inline void parseEnemyConfig(const std::shared_ptr<cugl::JsonValue>& root, RunBalanceConfig& config) {
    if (!root) {
        return;
    }

    // ga: procnet later
    // this parser is carried over as data only so later enemy-room work can
    // reuse the same config without changing the loader again
    std::shared_ptr<cugl::JsonValue> enemyNode = root->has("enemy") ? root->get("enemy") : root;
    if (!enemyNode) {
        return;
    }

    config.enemy.initialSpawnLimit = getIntMin(enemyNode, "initial_spawn_limit", config.enemy.initialSpawnLimit, 0);
    config.enemy.waveSpawnCount = getIntMin(enemyNode, "wave_spawn_count", config.enemy.waveSpawnCount, 0);
    config.enemy.waveIntervalSeconds = getFloatMin(enemyNode, "wave_interval_seconds",
                                                   config.enemy.waveIntervalSeconds, 0.1f);
    config.enemy.maxActiveEnemies = getIntMin(enemyNode, "max_active_enemies", config.enemy.maxActiveEnemies, 0);
    config.enemy.spawnHealth = getIntMin(enemyNode, "spawn_health", config.enemy.spawnHealth, 1);
    config.enemy.spawnSpeed = getFloatMin(enemyNode, "spawn_speed", config.enemy.spawnSpeed, 0.1f);
    config.enemy.waypointArrivalDistance = getFloatMin(enemyNode, "waypoint_arrival_distance",
                                                       config.enemy.waypointArrivalDistance, 0.1f);
    config.enemy.unstuckRadius = getFloatMin(enemyNode, "unstuck_radius", config.enemy.unstuckRadius, 1.0f);
}

inline void addCatalogEntries(const std::shared_ptr<cugl::JsonValue>& arrayNode,
                              const std::string& kind,
                              std::vector<ItemCatalogEntryConfig>& outEntries) {
    if (!arrayNode) {
        return;
    }

    for (size_t ii = 0; ii < arrayNode->size(); ++ii) {
        std::shared_ptr<cugl::JsonValue> item = arrayNode->get((int)ii);
        if (!item) {
            continue;
        }

        ItemCatalogEntryConfig entry;
        entry.kind = kind;
        entry.id = item->getString("id", kind + "_" + std::to_string(ii));
        entry.name = item->getString("name", entry.id);
        entry.textureAsset = item->getString("texture_asset", "");
        entry.value = getIntMin(item, "value", 1, 1);
        entry.weight = getFloatMin(item, "weight", 1.0f, 0.0f);
        outEntries.push_back(entry);
    }
}

inline void parseItemSpawnConfig(const std::shared_ptr<cugl::JsonValue>& root, RunBalanceConfig& config) {
    if (!root) {
        return;
    }

    std::shared_ptr<cugl::JsonValue> spawnNode = root->has("item_spawning") ? root->get("item_spawning") : root;
    if (!spawnNode) {
        return;
    }

    config.item.defaultPickupRadius = getFloatMin(spawnNode, "default_pickup_radius", config.item.defaultPickupRadius, 4.0f);
    config.item.guaranteedArtifacts = getIntMin(spawnNode, "guaranteed_artifacts", config.item.guaranteedArtifacts, 0);
    config.item.guaranteedSpellbooks = getIntMin(spawnNode, "guaranteed_spellbooks", config.item.guaranteedSpellbooks, 0);
    config.item.quotaCoverageRatio = getFloatMin(spawnNode, "quota_coverage_ratio", config.item.quotaCoverageRatio, 1.0f);
    config.item.quotaCoverageMaxExtraItems = getIntMin(spawnNode, "quota_coverage_max_extra_items",
                                                       config.item.quotaCoverageMaxExtraItems, 0);
}

inline void parseArtifactCatalogConfig(const std::shared_ptr<cugl::JsonValue>& root, RunBalanceConfig& config) {
    if (!root) {
        return;
    }

    std::shared_ptr<cugl::JsonValue> catalogNode = root->has("catalog") ? root->get("catalog") : root;
    if (!catalogNode) {
        catalogNode = root;
    }

    std::vector<ItemCatalogEntryConfig> loadedEntries;
    addCatalogEntries(catalogNode->get("artifacts"), "artifact", loadedEntries);
    addCatalogEntries(catalogNode->get("spellbooks"), "spellbook", loadedEntries);
    if (!loadedEntries.empty()) {
        config.item.entries = loadedEntries;
    }
}

inline RunBalanceConfig loadRunBalanceConfigFromAssets() {
    RunBalanceConfig config;

    std::shared_ptr<cugl::JsonValue> procedural = loadAssetJson("json/procedural_config.json");
    parseProceduralConfig(procedural, config);

    std::shared_ptr<cugl::JsonValue> roomTemplates = loadAssetJson("json/room_templates.json");
    parseRoomTemplateConfig(roomTemplates, config);

    std::shared_ptr<cugl::JsonValue> enemies = loadAssetJson("json/enemy_config.json");
    parseEnemyConfig(enemies, config);

    std::shared_ptr<cugl::JsonValue> itemSpawn = loadAssetJson("json/item_spawning_config.json");
    parseItemSpawnConfig(itemSpawn, config);

    std::shared_ptr<cugl::JsonValue> artifacts = loadAssetJson("json/artifact_config.json");
    parseArtifactCatalogConfig(artifacts, config);

    return config;
}

inline void logRunBalanceSummary(const RunBalanceConfig& config) {
    CULog("config summary quota=%d rooms=[%d,%d] max_hallway=%d templates=%llu enemy_limit=%d wave_count=%d wave_interval=%.2f max_active=%d item_entries=%llu quota_ratio=%.2f",
          config.quota,
          config.map.roomCountMin,
          config.map.roomCountMax,
          config.map.maxHallwayLengthTiles,
          (unsigned long long)config.map.roomTemplates.size(),
          config.enemy.initialSpawnLimit,
          config.enemy.waveSpawnCount,
          config.enemy.waveIntervalSeconds,
          config.enemy.maxActiveEnemies,
          (unsigned long long)config.item.entries.size(),
          config.item.quotaCoverageRatio);
}

} // namespace gameplayconfig
} // namespace hexcapade

#endif // __HEXCAPADE_GAMEPLAY_CONFIG_LOADER_H__

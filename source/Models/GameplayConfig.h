// this file stores editable runtime config data for map, room, item, and
// future artifact/modifier hooks loaded from json

#ifndef __HEXCAPADE_GAMEPLAY_CONFIG_H__
#define __HEXCAPADE_GAMEPLAY_CONFIG_H__

#include <cugl/cugl.h>
#include <string>
#include <vector>

struct RoomTemplateConfig {
    std::string roomId;
    int width = 12;
    int height = 10;
    bool hasColor = false;
    cugl::Color4 color = cugl::Color4(118, 146, 108, 255);
    std::vector<std::string> possibleModifiers;
};

struct MapGenerationConfig {
    int roomCountMin = 6;
    int roomCountMax = 10;
    int roomPlacementAttempts = 1200;

    float tileSize = 48.0f;
    float worldScaleX = 4.0f;
    float worldScaleY = 4.0f;
    int worldMinGridWidth = 84;
    int worldMinGridHeight = 84;

    int roomWidthMin = 10;
    int roomWidthMax = 16;
    int roomHeightMin = 8;
    int roomHeightMax = 14;
    int roomBufferTiles = 3;
    int roomEdgePadding = 2;

    int hallThickness = 2;
    int maxHallwayLengthTiles = 26;
    int extraHallConnectionStep = 3;

    int obstacleCountMin = 2;
    int obstacleCountMax = 4;
    int obstacleSizeMin = 1;
    int obstacleSizeMax = 3;
    int obstacleMarginTiles = 2;
    int obstacleCenterClearanceTiles = 2;
    int obstaclePlacementAttempts = 90;

    int largeRoomWidthThreshold = 12;
    int largeRoomHeightThreshold = 10;
    int primaryItemOffsetX = -2;
    int primaryItemOffsetY = 0;
    int secondaryItemOffsetX = 2;
    int secondaryItemOffsetY = -1;
    int enemyOffsetX = 0;
    int enemyOffsetY = 2;
    int skipEnemySpawnsInFirstRooms = 1;

    cugl::Color4 backdropColor = cugl::Color4(16, 20, 24, 255);
    cugl::Color4 hallFloorColor = cugl::Color4(146, 132, 98, 255);
    cugl::Color4 wallFaceColor = cugl::Color4(42, 50, 58, 255);
    cugl::Color4 wallTopColor = cugl::Color4(69, 78, 88, 255);
    cugl::Color4 obstacleFaceColor = cugl::Color4(96, 96, 96, 255);
    cugl::Color4 obstacleTopColor = cugl::Color4(152, 152, 152, 255);

    std::vector<cugl::Color4> roomPalette = {
        cugl::Color4(98, 142, 184, 255),
        cugl::Color4(112, 162, 110, 255),
        cugl::Color4(168, 135, 92, 255),
        cugl::Color4(145, 112, 170, 255),
        cugl::Color4(172, 114, 114, 255),
        cugl::Color4(111, 169, 166, 255)
    };

    std::vector<RoomTemplateConfig> roomTemplates;
};

struct EnemySpawnConfig {
    // ga: procnet later
    // this stays as data only for now so later enemy-room work can reuse the
    // same config shape without reshaping the loader again
    int initialSpawnLimit = 6;
    int waveSpawnCount = 0;
    float waveIntervalSeconds = 5.0f;
    int maxActiveEnemies = 0;
    int spawnHealth = 4;
    float spawnSpeed = 2.5f;
    float waypointArrivalDistance = 3.0f;
    float unstuckRadius = 8.0f;
};

struct ItemCatalogEntryConfig {
    std::string kind;
    std::string id;
    std::string name;
    std::string textureAsset;
    int value = 0;
    float weight = 1.0f;
};

struct ItemSpawnConfig {
    float defaultPickupRadius = 14.0f;
    int guaranteedArtifacts = 1;
    int guaranteedSpellbooks = 1;
    float quotaCoverageRatio = 1.15f;
    int quotaCoverageMaxExtraItems = 24;
    std::vector<ItemCatalogEntryConfig> entries;
};

struct RunBalanceConfig {
    int quota = 100;
    MapGenerationConfig map;
    EnemySpawnConfig enemy;
    ItemSpawnConfig item;
};

#endif // __HEXCAPADE_GAMEPLAY_CONFIG_H__

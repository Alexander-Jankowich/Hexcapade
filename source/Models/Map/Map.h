
// this model generates and stores the shared room/hall map for one run

#ifndef __HEXCAPADE_MAP_H__
#define __HEXCAPADE_MAP_H__

#include <array> 
#include <cugl/cugl.h>

#include "Room.h"
#include "LevelLoader.h"
#include "../Enemies/EnemyObstacle.h"
#include "../Items/ItemTypeClass.h"

//#include "../../Controllers/NetworkController.h"

#define MAP_HEIGHT 120
#define MAP_WIDTH 120
#define TILE_SIZE 40
#define PHYSICS_SCALE 50 //TODO: find a way to like. not redefine it lol

// GID defs for hallway gen
#define FLOOR_GID 67
#define WEST_GID 100
#define EAST_GID 101
#define NORTH_TOP_GID 10
#define NORTH_MID_GID 23
#define NORTH_LOW_GID 36
#define SOUTH_TOP_GID 103
#define SOUTH_MID_GID 116
#define SOUTH_LOW_GID 129

using namespace cugl;
using namespace cugl::graphics;

class Map {
private:
    struct LevelConfig {
        int _mapSize;
        int quota;
        int maxEnemies;
        int enemSpawnInterval;
        
        std::vector<EnemyType> _enemyTypes;
        std::vector<float> _zone1WeightsEn;
        std::vector<float> _zone2WeightsEn;
        std::vector<float> _zone3WeightsEn;
        
        float _zone1EnemyProb;
        float _zone2EnemyProb;
        float _zone3EnemyProb;
        
        // item stuff
        std::vector<ItemTypeClass::ItemType> _spellbookTypes;
        std::vector<float> _zone1WeightsSp;
        std::vector<float> _zone2WeightsSp;
        std::vector<float> _zone3WeightsSp;

        std::vector<ItemTypeClass::ItemType> _artifactTypes;
        std::vector<float> _zone1WeightsArt;
        std::vector<float> _zone2WeightsArt;
        std::vector<float> _zone3WeightsArt;
        
        // spellbook vs artifact chance
        std::vector<float> _zoneSpellbookProb;
        float _goldChance;
    };
    struct MapDoor {
        Vec2 pos;
        Size size;
        Door::Direction dir;
        MapDoor(Vec2 p, Size s, Door::Direction d) {
            pos = p;
            size = s;
            dir = d;
        }
    };

    //std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT> _mapGrid;
    std::vector<std::vector<int>> _mapGrid;
    std::shared_ptr<AssetManager> _assets;
    
    int _maxRoomW;
    int _maxRoomH;
    
    cugl::Vec2 _origin;
    cugl::Size _sceneSize;
    SimpleExtruder _extruder;
    std::shared_ptr<cugl::scene2::SceneNode> lw_node;
    std::vector<std::shared_ptr<Room>> _rooms;

    std::vector<std::pair<Vec2, ItemTypeClass::ItemType>> _itemSpawns;
    
    std::vector<cugl::Vec2> _z1itemSpawnPoints;
    std::vector<cugl::Vec2> _z2itemSpawnPoints;
    std::vector<cugl::Vec2> _z3itemSpawnPoints;
    
    std::vector<cugl::Vec2> _z1enemySpawnPoints;
    std::vector<cugl::Vec2> _z2enemySpawnPoints;
    std::vector<cugl::Vec2> _z3enemySpawnPoints;

    cugl::Vec2 _playerSpawn;
    LevelConfig _config;

    cugl::Random _rng;

public:
    Map();
    
    void reset();
    /** Procedurally generates the map for this run. Adds all created rooms to a SceneNode and generates collision obstacles accordingly
     * @param seed the random seed used to initialize this runs random number generator
     * @param assets the assetManager, which contains all loaded tiled data tied to proper room keys
     * @param rooms the list of all string keys used to access each room in the AssetManager
     * @return a pair, the world node and the lower walls node
     */
    std::shared_ptr<cugl::scene2::SceneNode> generate(int level, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms, const Size& sceneSize, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world, int numPlayers);
    
    /**
     * called by GLGameScene to roll die for each zone and generate an enemy spawn if the zone die roll succeeds.
     */
    std::vector<std::pair<Vec2, EnemyType>> generateEnemySpawns(const std::vector<Vec2> playersPos, const std::vector<Vec2> enemsPos);
    
    /**
     * called by GLGameScene to populate item spawns for the run
     */
     void generateItemSpawns();
    
    std::shared_ptr<cugl::scene2::SceneNode> getLowerWallNode() {return lw_node;}
    Vec2 getPlayerSpawn() {return _playerSpawn;}
    const std::vector<std::pair<Vec2, ItemTypeClass::ItemType>>& getItemSpawns() const {return _itemSpawns;}
    const std::vector<std::shared_ptr<Room>> getRooms() const {return _rooms;}
    int getNumRooms() { return (int)_rooms.size(); }
    int getQuota() {return _config.quota;}
    int getEnemInterval() {return _config.enemSpawnInterval;}
private:
    /** Helper function to add all sceneNodes for a room (given tiled data) in the right place.
     * Side effect:  fills in the mapGrid with 1s for each space this room covers
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in mapGrid coordinates
     * @param world the base sceneNode to add all created nodes to
     * @param lwalls the sceneNode to add all the lower walls to
     */
    void addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world, std::shared_ptr<cugl::scene2::SceneNode> lwalls);
    
    /** Helper function to add all obstacles for a room
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in mapGrid coordinates
     * @param world the physics networld to add all created obstacles to
     */
    void addRoomObstacles(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::physics2::distrib::NetWorld> world);
    
    /** Helper function that attempts to fit a room branching off of the desired door.
     * @param d the door the algorithm is trying to branch off of
     * @param rooms a list of rooms that could possibly connect (have door in correct wall)
     * @param connDir the direction of the connection (from view of room)
     * @return a tuple of the tiled data for a room that fits, the # of hallway units it needs, and the coords for the rooms top left corner. nullptr if no room was found.
     */
    std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> attemptToFitRoom(MapDoor& d, std::vector<std::string>& rooms, Door::Direction connDir);
    
    void addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world);
    
    void addWallObstacle(Poly2& poly, std::shared_ptr<cugl::physics2::distrib::NetWorld>& world);
    
    void loadLevelConfig(int level, int numPlayers, std::shared_ptr<cugl::JsonValue> config);
   
    
    
    std::pair<Vec2, EnemyType> chooseEnemySpawnPoint(const std::vector<Vec2> zonePoints, const std::vector<Vec2> playersPos, const std::vector<Vec2> enemsPos, const std::vector<float> zoneRates);
    /** Picks an item type for a single spawn point using the zone's spellbook/artifact split + zoneIndex is 0-indexed so 0/1/2 for zone 1/2/3. */
    ItemTypeClass::ItemType chooseItemType(const std::vector<float> zoneSpellbookWeights, const std::vector<float> zoneArtifactWeights, float spellBookProb);
};

#endif // __HEXCAPADE_MAP_H__

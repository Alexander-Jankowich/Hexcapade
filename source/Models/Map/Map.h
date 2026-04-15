
// this model generates and stores the shared room/hall map for one run

#ifndef __HEXCAPADE_MAP_H__
#define __HEXCAPADE_MAP_H__

#include <cugl/cugl.h>

#include "../GameplayConfig.h"
#include "Room.h"
#include "LevelLoader.h"
#include "../../Controllers/NetworkController.h"

#define MAP_HEIGHT 100
#define MAP_WIDTH 100
#define TILE_SIZE 40

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
    struct GridRoom {
        int x;
        int y;
        int w;
        int h;
        std::shared_ptr<LevelLoader> _tiledData;
    };

    std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT> _mapGrid;
    std::shared_ptr<AssetManager> _assets;
    
    cugl::Vec2 _origin;
    cugl::Size _sceneSize;

    //std::vector<Room> _rooms; IGNORING FOR NOW

    std::vector<cugl::Vec2> _itemSpawnPoints;
    std::vector<cugl::Vec2> _enemySpawnPoints;
    cugl::Vec2 _playerSpawn;
    MapGenerationConfig _config;
    int _numRooms; //TEMP CHANGE LATER

    cugl::Random _rng;

public:
    Map();
    /** Procedurally generates the map for this run. Adds all created rooms to a SceneNode and generates collision obstacles accordingly
     * @param seed the random seed used to initialize this runs random number generator
     * @param assets the assetManager, which contains all loaded tiled data tied to proper room keys
     * @param rooms the list of all string keys used to access each room in the AssetManager
     */
    std::shared_ptr<cugl::scene2::SceneNode> generate(const MapGenerationConfig& config, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms, const Size& sceneSize);
    
    Vec2 getPlayerSpawn() {return _playerSpawn;}
    std::vector<Vec2> getEnemySpawnPoints() {return _enemySpawnPoints;}
    std::vector<Vec2> getItemSpawnPoints() {return _enemySpawnPoints;}
    int getNumRooms() {return _numRooms;}


private:
    /** Helper function to add all sceneNodes for a room (given tiled data) in the right place.
     * Side effect:  fills in the mapGrid with 1s for each space this room covers
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in mapGrid coordinates
     * @param world the base sceneNode to add all created nodes to
     */
    void addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world);
    
    /** Helper function that attempts to fit a room branching off of the desired door.
     * @param d the door the algorithm is trying to branch off of
     * @param rooms a list of rooms that could possibly connect (have door in correct wall)
     * @param connDir the direction of the connection (from view of room)
     * @return a tuple of the tiled data for a room that fits, the # of hallway units it needs, and the coords for the rooms top left corner. nullptr if no room was found.
     */
    std::tuple<std::shared_ptr<LevelLoader>, int, Vec2> attemptToFitRoom(MapDoor& d, std::vector<std::string>& rooms, Door::Direction connDir);
    
    void addSingleNode(std::shared_ptr<Texture> t, Vec2 pos, std::shared_ptr<cugl::scene2::SceneNode> world);
};

#endif // __HEXCAPADE_MAP_H__

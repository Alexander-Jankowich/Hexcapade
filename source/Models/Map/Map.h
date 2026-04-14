
// this model generates and stores the shared room/hall map for one run

#ifndef __HEXCAPADE_MAP_H__
#define __HEXCAPADE_MAP_H__

#include <cugl/cugl.h>
#include <memory>
#include <vector>

#include "../GameplayConfig.h"
#include "Room.h"
#include "LevelLoader.h"
#include "../../Controllers/NetworkController.h"
#include "RectFactory.h"

#define MAP_HEIGHT 100
#define MAP_WIDTH 100


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
    }
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

    std::vector<Room> _rooms;

    std::vector<cugl::Vec2> _itemSpawnPoints;
    std::vector<cugl::Vec2> _enemySpawnPoints;
    cugl::Vec2 _playerSpawn;
    MapGenerationConfig _config;

    cugl::Random _rng;

public:
    Map();
    /** Procedurally generates the map for this run. Adds all created rooms to a SceneNode and generates collision obstacles accordingly
     * @param seed the random seed used to initialize this runs random number generator
     * @param assets the assetManager, which contains all loaded tiled data tied to proper room keys
     * @param rooms the list of all string keys used to access each room in the AssetManager
     */
    std::shared_ptr<cugl::scene2::SceneNode> generate(const MapGenerationConfig& config, unsigned int seed,  std::shared_ptr<AssetManager> assets, std::shared_ptr<std::vector<std::string>> rooms);

private:
    /** Helper function to add all sceneNodes for a room (given tiled data) in the right place.
     * Side effect:  fills in the mapGrid with 1s for each space this room covers
     * @param tiledRmData the Tiled Data for this room
     * @param tlCorner the top left corner of the room in tiled coordinates
     * @param world the base sceneNode to add all created nodes to
     */
    void addRoomSceneNodes(std::shared_ptr<LevelLoader> tiledRmData, Vec2 tlCorner, std::shared_ptr<cugl::scene2::SceneNode> world);
    
    /** Helper function that attempts to fit a room branching off of the desired door.
     * @param d the door the algorithm is trying to branch off of
     * @param rooms a list of rooms that could possibly connect (have door in correct wall)
     * @param connDir the direction of the connection (from view of room)
     * @return a pair of the tiled data for a room that fits and the # of hallway units it needs. nullptr if no room was found.
     */
    std::pair<std::shared_ptr<LevelLoader>, int> attemptToFitRoom(MapDoor d, std::shared_ptr<std::vector<std::string>> rooms, Door::Direction connDir);
};

#endif // __HEXCAPADE_MAP_H__

//
//  LevelLoader.h
//  tech_final
//
//  Created by Serena Duncan on 3/24/26.
//
#ifndef __TD_LEVEL_MODEL_H__
#define __TD_LEVEL_MODEL_H__
#include <array> 
#include <cugl/cugl.h>

using namespace std;
using namespace cugl;
struct Door {
        enum Direction : int {
            NORTH=0,
            EAST=1,
            SOUTH=2,
            WEST=3
        };
    Direction _direction;
    Vec2 _tlCorner;
    Size _dimens;
    Door(Direction d, int x, int y, int h, int w) {
        _direction = d;
        _tlCorner = Vec2(x, y);
        _dimens = Size(w, h);
    }
};
class SpawnPoint {
public:
    enum SpawnType : int {
        PLAYER=0,
        ENEMY=1,
        ITEM=2
    };
    SpawnType _type;
    Vec2 _spawnPoint;
    SpawnPoint(SpawnType t, float x, float y){
        _type = t;
        _spawnPoint = Vec2(x, y);
    }
};

class LevelLoader : public Asset {
    
protected:
    map<int, std::shared_ptr<graphics::Texture>> _tiles;
    Uint32 _tileWidth;
    Uint32 _tileHeight;
    int _mapHeight;
    int _mapWidth;
    vector<vector<int>> _floor;
    vector<vector<int>> _walls;
    vector<vector<int>> _lowerWalls;
    vector<vector<int>> _deco;
    vector<vector<int>> _visfix;
    vector<vector<int>> _mask;
    std::array<std::shared_ptr<Door>, 4> _doors;
    vector<Vec2> _itemSpawnPoints;
    vector<Vec2> _enemSpawnPoints;
    Vec2 _playerSpawn;
    vector<cugl::Path2> _colliders;


    // hash map for doors? direction -> vec door objects
    string _imgPath;
    int _firstID;
    
private:
    bool loadTilesFromImage(std::shared_ptr<graphics::Texture> img, int firstID);

public:
    LevelLoader();
    ~LevelLoader();
    
    Uint32 getTileW() {return _tileWidth;}
    Uint32 getTileH() {return _tileHeight;}
    
    /**
     * Creates a loader with no source file.
     *
     * The source file can be set at any time via the setFile() method. This method
     * does NOT load the asset.  You must call the load() method to do that.
     *
     * @return  an autoreleased level file
     */
    static std::shared_ptr<LevelLoader> alloc() {
        std::shared_ptr<LevelLoader> result = std::make_shared<LevelLoader>();
        return (result->init("") ? result : nullptr);
    }

    /**
     * Creates a new game level with the given source file.
     *
     * This method does NOT load the level. You must call the load() method to do that.
     * This method returns false if file does not exist.
     *
     * @return  an autoreleased level file
     */
    static std::shared_ptr<LevelLoader> alloc(std::string file) {
        std::shared_ptr<LevelLoader> result = std::make_shared<LevelLoader>();
        return (result->init(file) ? result : nullptr);
    }
    
    virtual bool preload (const std::string& file);
    virtual bool preload (const std::shared_ptr< JsonValue > &json) override;
    
    virtual bool materialize() override;
    
    /** This is a helper method to change the source, as room effects are displayed in the UI and have
     * the same layout (wrt GIDs) but are tied to a different source tileset. */
    bool changeTiledSource(std::string& newImgPath);
    
#pragma mark Getters
    std::shared_ptr<graphics::Texture> getTileForGid(int gid) {return _tiles.at(gid);}
    const vector<vector<int>>& getFloorLayer() const {return _floor;}
    const vector<vector<int>>& getWallLayer() const {return _walls;}
    const vector<vector<int>>& getBottomWallLayer() const {return _lowerWalls;}
    const vector<vector<int>>& getDecoLayer() const {return _deco;}
    const vector<vector<int>>& getFixLayer() const {return _visfix;}
    const vector<vector<int>>& getMaskLayer() const {return _mask;}
    
    std::vector<std::shared_ptr<Door>> getDoors() {
        std::vector<std::shared_ptr<Door>> res;
        for (auto d: _doors) if(d!=nullptr) res.push_back(d);
        return res;
    }
    

    const vector<Vec2>& getItemSpawnPoints() const {return _itemSpawnPoints;}
    const vector<Vec2>& getEnemSpawnPoints() const {return _enemSpawnPoints;}
    const vector<cugl::Path2>& getColliders() const {return _colliders;}
    Vec2 getPlayerSpawn() {return _playerSpawn;}


    int getHeight() {return _mapHeight;}
    int getWidth() {return _mapWidth;}
    bool hasNorthDoor() {return _doors[0] != nullptr;}
    bool hasEastDoor() {return _doors[1] != nullptr;}
    bool hasSouthDoor() {return _doors[2] != nullptr;}
    bool hasWestDoor() {return _doors[3] != nullptr;}
    
    std::shared_ptr<Door> getNorthDoor() {return _doors[0];}
    std::shared_ptr<Door> getEastDoor() {return _doors[1];}
    std::shared_ptr<Door> getSouthDoor() {return _doors[2];}
    std::shared_ptr<Door> getWestDoor() {return _doors[3];}
};

#endif // !__TD_LEVEL_MODEL_H__

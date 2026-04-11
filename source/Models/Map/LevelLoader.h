//
//  LevelLoader.h
//  tech_final
//
//  Created by Serena Duncan on 3/24/26.
//
#ifndef __TD_LEVEL_MODEL_H__
#define __TD_LEVEL_MODEL_H__
#include <cugl/cugl.h>

using namespace std;
using namespace cugl;
class Door {
    public:
        enum Direction : int {
            NORTH=0,
            EAST=1,
            SOUTH=2,
            WEST=3
        };
    Direction _direction;
    Vec2 _llCorner;
    Door(Direction d, float x, float y) {
        _direction = d;
        _llCorner = Vec2(x, y);
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
    vector<vector<int>> _floor;
    vector<vector<int>> _walls;
    vector<vector<int>> _deco;
    vector<vector<int>> _visfix;
    vector<Door> _doors;
    vector<SpawnPoint> _spawnPoints;
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
    
#pragma mark Getters
    map<int, std::shared_ptr<graphics::Texture>> getTiles() {return _tiles;}
    vector<vector<int>> getFloorLayer() {return _floor;}
    vector<vector<int>> getWallLayer() {return _walls;}
    vector<vector<int>> getDecoLayer() {return _deco;}
    vector<vector<int>> getFixLayer() {return _visfix;}
    
    vector<Door> getDoors() {return _doors;}
    

    vector<SpawnPoint> getSpawnPoints() {return _spawnPoints;}
};

#endif // !__TD_LEVEL_MODEL_H__

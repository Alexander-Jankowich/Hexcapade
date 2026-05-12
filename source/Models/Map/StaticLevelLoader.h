//
//  StaticLevelLoader.h
//  hexcapade
//
//  Created by Serena Duncan on 5/7/26.
//

#ifndef __TD_STATIC_LEVEL_MODEL_H__
#define __TD_STATIC_LEVEL_MODEL_H__
#include <array>
#include <cugl/cugl.h>

using namespace std;
using namespace cugl;


class StaticLevelLoader : public Asset {
public:
    struct Event {
        Rect location;
        std::string png; //TODO: this will be a texture soon...
        Event(Rect r, string t) {
            location = r;
            png = t;
        }
    };
    struct ItemSpawn {
        Vec2 location;
        int item;
        
        ItemSpawn(Vec2 l, int i) {
            location = l;
            item =  i;
        }
    };
    struct EnemySpawn {
        Vec2 location;
        int enem;
        
        EnemySpawn(Vec2 l, int i) {
            location = l;
            enem =  i;
        }
    };
    struct BotSpawn {
        Vec2 location;
        int spell; // these get cast in tutorialscene when creating object
        
        BotSpawn(Vec2 l, int i) {
            location = l;
            spell = i;
        }
    };
    
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
    vector<ItemSpawn> _itemSpawnPoints;
    vector<EnemySpawn> _enemSpawnPoints;
    vector<BotSpawn> _botSpawnPoints;
    Vec2 _playerSpawn;
    Vec2 _depositSpawn;
    vector<Path2> _colliders;
    vector<Event> _events;


    string _imgPath;
    int _firstID;
    
private:
    bool loadTilesFromImage(std::shared_ptr<graphics::Texture> img, int firstID);

public:
    StaticLevelLoader();
    ~StaticLevelLoader();
    
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
    static std::shared_ptr<StaticLevelLoader> alloc() {
        std::shared_ptr<StaticLevelLoader> result = std::make_shared<StaticLevelLoader>();
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
    static std::shared_ptr<StaticLevelLoader> alloc(std::string file) {
        std::shared_ptr<StaticLevelLoader> result = std::make_shared<StaticLevelLoader>();
        return (result->init(file) ? result : nullptr);
    }
    
    virtual bool preload (const std::string& file);
    virtual bool preload (const std::shared_ptr< JsonValue > &json) override;
    
    virtual bool materialize() override;
    
#pragma mark Getters
    std::shared_ptr<graphics::Texture> getTileForGid(int gid) {return _tiles.at(gid);}
    const vector<vector<int>>& getFloorLayer() const {return _floor;}
    const vector<vector<int>>& getWallLayer() const {return _walls;}
    const vector<vector<int>>& getBottomWallLayer() const {return _lowerWalls;}
    const vector<vector<int>>& getDecoLayer() const {return _deco;}
    const vector<vector<int>>& getFixLayer() const {return _visfix;}
    

    const vector<ItemSpawn>& getItemSpawnPoints() const {return _itemSpawnPoints;}
    const vector<EnemySpawn>& getEnemSpawnPoints() const {return _enemSpawnPoints;}
    const vector<BotSpawn>& getBotSpawnPoints() const {return _botSpawnPoints;}

    const vector<cugl::Path2>& getColliders() const {return _colliders;}
    const vector<Event>& getEvents() const {return _events;}

    Vec2 getPlayerSpawn() {return _playerSpawn;}
    Vec2 getDepositSpawn() {return _depositSpawn;}


    int getHeight() {return _mapHeight;}
    int getWidth() {return _mapWidth;}
};

#endif // !__TD_STATIC_LEVEL_MODEL_H__

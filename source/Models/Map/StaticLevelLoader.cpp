//
//  StaticLevelLoader.cpp
//  hexcapade
//
//  Created by Serena Duncan on 5/8/26.
//

#include "StaticLevelLoader.h"

StaticLevelLoader::StaticLevelLoader() :
    _tileWidth(0),
    _tileHeight(0) {
}

StaticLevelLoader::~StaticLevelLoader() {
}

/**
 * Loads individual tiles in tileset and assigns them to proper ID.
 * @param image img to load tileset from
 * @param firstID the first GID in the tileset,
 * @return true if loading tiles was successful
 */
bool StaticLevelLoader::loadTilesFromImage(std::shared_ptr<graphics::Texture> img, int firstID) {
    // iterate over image tile by tile
    int currID = firstID;
    for (int y_start = img->getHeight() - _tileHeight; y_start >= 0; y_start-=_tileHeight) {
        for (int x_start = 0; x_start < img->getWidth(); x_start+=_tileWidth) {
            auto tiletex = img->getSubTexture(x_start, y_start, _tileWidth, _tileHeight);
            _tiles.insert(pair<int, std::shared_ptr<graphics::Texture>>(currID, tiletex));
            currID++;
        }
    }
    return true;
}

/**
 * Loads this game level from the source file
 * @return true if successfully loaded the asset from a file
 */
bool StaticLevelLoader::preload(const std::string& file) {
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
    return preload(reader->readJson());
}

bool StaticLevelLoader::preload (const std::shared_ptr< JsonValue > &json) {
    // load the textures into _tiles
    _tileWidth = json->getInt("tilewidth");
    _tileHeight = json->getInt("tileheight");
    _imgPath = json->get("tilesets")->get(0)->getString("source");
    _firstID = json->get("tilesets")->get(0)->getInt("firstgid");
    
    // init 2D layer vectors
    _mapHeight = json->getInt("height");
    _mapWidth = json->getInt("width");
    vector<vector<int>> fl(_mapHeight, vector<int>(_mapWidth, 0));
    vector<vector<int>> wl(_mapHeight, vector<int>(_mapWidth, 0));
    vector<vector<int>> vf(_mapHeight, vector<int>(_mapWidth, 0));
    vector<vector<int>> dc(_mapHeight, vector<int>(_mapWidth, 0));
    vector<vector<int>> bwl(_mapHeight, vector<int>(_mapWidth, 0));
    _floor = fl;
    _walls = wl;
    _visfix = vf;
    _deco = dc;
    _lowerWalls = bwl;
    
    // generate the flat map
    auto fl_layer = json->get("layers")->get(0)->get("layers")->get(0);
    auto wl_layer = json->get("layers")->get(0)->get("layers")->get(1);
    auto vf_layer = json->get("layers")->get(0)->get("layers")->get(2);
    auto dc_layer = json->get("layers")->get(0)->get("layers")->get(3);
    auto bwl_layer = json->get("layers")->get(0)->get("layers")->get(4);

    auto fl_tiledData = fl_layer->get("data")->asIntArray();
    auto wl_tiledData = wl_layer->get("data")->asIntArray();
    auto vf_tiledData = vf_layer->get("data")->asIntArray();
    auto dc_tiledData = dc_layer->get("data")->asIntArray();
    auto bwl_tiledData = bwl_layer->get("data")->asIntArray();

    // iterate over the data set in parallel lol
    for (int row=0; row<_mapHeight; row++){
        for(int col=0; col<_mapWidth; col++){
            _floor.at(row).at(col) = fl_tiledData[row*_mapWidth + col];
            _walls.at(row).at(col) = wl_tiledData[row*_mapWidth + col];
            _visfix.at(row).at(col) = vf_tiledData[row*_mapWidth + col];
            _deco.at(row).at(col) = dc_tiledData[row*_mapWidth + col];
            _lowerWalls.at(row).at(col) = bwl_tiledData[row * _mapWidth + col];
        }
    }
    
    // generate the spawn points. order: item, enemies, player/bot/deposit, events, colliders
    // TODO: IGNORE FOR NOW UNTIL DECIDING ON ENUM
//    for (auto item_spawn : json->get("layers")->get(1)->get("layers")->get(0)->get("objects")->children()){
//        auto sp = Vec2(item_spawn->getFloat("x") / _tileWidth, item_spawn->getFloat("y") / _tileHeight);
//        _itemSpawnPoints.push_back(sp);
//    }
    
    // in SLL we convert into world coords while loading bc we know map height
    // TODO: change all y to me tw * height - y
    for (auto enem_spawn : json->get("layers")->get(1)->get("layers")->get(1)->get("objects")->children()){
        auto sp = Vec2(enem_spawn->getFloat("x"), _mapHeight * _tileWidth - enem_spawn->getFloat("y"));
        _enemSpawnPoints.push_back(EnemySpawn(sp, enem_spawn->get("properties")->children()[0]->getInt("value")));
    }
    
    for (auto spawn : json->get("layers")->get(1)->get("layers")->get(2)->get("objects")->children()) {
        auto t = spawn->getString("name");
        auto sp = Vec2(spawn->getFloat("x"), _mapHeight * _tileWidth - spawn->getFloat("y"));
        
        if(t == "player")
            _playerSpawn = sp;
        
        if(t=="deposit")
                _depositSpawn = sp;
        
        if (t=="bot")
                _botSpawnPoints.push_back(BotSpawn(sp, spawn->get("properties")->children()[0]->getInt("value")));
    }
    // collect events
    for (auto e_data : json->get("layers")->get(1)->get("layers")->get(3)->get("objects")->children()) {
        Rect bounds = Rect(e_data->getFloat("x") /_tileWidth, e_data->getFloat("y") /_tileHeight, e_data->getInt("width") /_tileWidth, e_data->getInt("height") /_tileHeight);
        std::string t = &"tut_" [ e_data->get("properties")->children()[0]->getInt("value")];
        _events.push_back(Event(bounds, t));
    }
    // collect colliders
    for (auto col_data : json->get("layers")->get(1)->get("layers")->get(4)->get("objects")->children()) {
        Vec2 origin = Vec2(col_data->getFloat("x"), col_data->getFloat("y"));
        vector<Vec2> points;
        for (auto p : col_data->get("polygon")->children()) {
            points.push_back(Vec2((origin.x + p->getFloat("x"))/_tileWidth, (origin.y + p->getFloat("y"))/_tileHeight));
        }
        _colliders.push_back(Path2(points));
    }
    
    
    return true;
}

bool StaticLevelLoader::materialize() {
    shared_ptr<graphics::Image> image = graphics::Image::allocWithFile(_imgPath);
    auto tex = graphics::Texture::allocWithImage(image); // acc to spec this will work
    loadTilesFromImage(tex, _firstID);
    return true;
}

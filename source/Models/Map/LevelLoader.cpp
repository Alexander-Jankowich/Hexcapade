//
//  LevelLoader.cpp
//  tech_final
//
//  Created by Serena Duncan on 3/24/26.
//
#include "LevelLoader.h"

LevelLoader::LevelLoader() :
    _tileWidth(0),
    _tileHeight(0) {
}

LevelLoader::~LevelLoader() {
}

/**
 * Loads individual tiles in tileset and assigns them to proper ID.
 * @param image img to load tileset from
 * @param firstID the first GID in the tileset,
 * @return true if loading tiles was successful
 */
bool LevelLoader::loadTilesFromImage(std::shared_ptr<graphics::Texture> img, int firstID) {
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
bool LevelLoader::preload(const std::string& file) {
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
    return preload(reader->readJson());
}

bool LevelLoader::preload (const std::shared_ptr< JsonValue > &json) {
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
    vector<vector<int>> mk(_mapHeight, vector<int>(_mapWidth, 0));
    _floor = fl;
    _walls = wl;
    _visfix = vf;
    _deco = dc;
    _mask = mk;
    
    // generate the flat map
    auto fl_layer = json->get("layers")->get(0)->get("layers")->get(0);
    auto wl_layer = json->get("layers")->get(0)->get("layers")->get(1);
    auto vf_layer = json->get("layers")->get(0)->get("layers")->get(2);
    auto dc_layer = json->get("layers")->get(0)->get("layers")->get(3);
    auto mk_layer = json->get("layers")->get(0)->get("layers")->get(4);

    auto fl_tiledData = fl_layer->get("data")->asIntArray();
    auto wl_tiledData = wl_layer->get("data")->asIntArray();
    auto vf_tiledData = vf_layer->get("data")->asIntArray();
    auto dc_tiledData = dc_layer->get("data")->asIntArray();
    auto mk_tiledData = mk_layer->get("data")->asIntArray();

    // iterate over the data set in parallel lol
    for (int row=0; row<_mapHeight; row++){
        for(int col=0; col<_mapWidth; col++){
            _floor.at(row).at(col) = fl_tiledData[row*_mapWidth + col];
            _walls.at(row).at(col) = wl_tiledData[row*_mapWidth + col];
            _visfix.at(row).at(col) = vf_tiledData[row*_mapWidth + col];
            _deco.at(row).at(col) = dc_tiledData[row*_mapWidth + col];
            _mask.at(row).at(col) = mk_tiledData[row * _mapWidth + col];
        }
    }
    
    // generate the spawn points
    // TODO: figure out if should be shared_ptr or not...
    for (auto item_spawn : json->get("layers")->get(1)->get("layers")->get(0)->get("objects")->children()){
        auto sp = Vec2(item_spawn->getFloat("x") / _tileWidth, item_spawn->getFloat("y") / _tileHeight);
        _itemSpawnPoints.push_back(sp);
    }
    
    auto pl_spawn = json->get("layers")->get(1)->get("layers")->get(1)->get("objects")->get(0);
    _playerSpawn = Vec2(pl_spawn->getFloat("x") / _tileWidth, pl_spawn->getFloat("y") / _tileHeight);
    
    for (auto enem_spawn : json->get("layers")->get(1)->get("layers")->get(2)->get("objects")->children()){
        auto sp = Vec2(enem_spawn->getFloat("x") / _tileWidth, enem_spawn->getFloat("y") / _tileHeight);
        _enemSpawnPoints.push_back(sp);
    }
    _doors.fill(nullptr);
    // DOORS ARE IN TILE BASED COORDS, divide by 40
    for (auto door_sp : json->get("layers")->get(1)->get("layers")->get(3)->get("objects")->children()){
        auto d = std::make_shared<Door>(Door(static_cast<Door::Direction>(door_sp->get("properties")->get(0)->getInt("value")), door_sp->getInt("x") / _tileWidth, door_sp->getInt("y") / _tileHeight, door_sp->getInt("width") / _tileWidth, door_sp->getInt("height") / _tileHeight));
        _doors[door_sp->get("properties")->get(0)->getInt("value")] = d;
    }
    return true;
}

bool LevelLoader::materialize() {
    shared_ptr<graphics::Image> image = graphics::Image::allocWithFile(_imgPath);
    auto tex = graphics::Texture::allocWithImage(image); // acc to spec this will work
    loadTilesFromImage(tex, _firstID);
    return true;
}

bool LevelLoader::changeTiledSource(std::string &newImgPath) {
    _imgPath = newImgPath;
    shared_ptr<graphics::Image> image = graphics::Image::allocWithFile(_imgPath);
    auto tex = graphics::Texture::allocWithImage(image); // acc to spec this will work
    loadTilesFromImage(tex, _firstID);
    return true;
}

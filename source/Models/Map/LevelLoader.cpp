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
    // TODO: debug
    
    
    int mapHeight = json->getInt("height");
    int mapWidth = json->getInt("width");
    for (int i=0; i<mapHeight; i++) {
        vector<int> v(mapWidth);
        _floor.push_back(v);
        _walls.push_back(v);
        _visfix.push_back(v);
        _deco.push_back(v);
    }
    // generate the flat map
    auto fl_layer = json->get("layers")->get(0)->get("layers")->get(0);
    auto wl_layer = json->get("layers")->get(0)->get("layers")->get(1);
    auto vf_layer = json->get("layers")->get(0)->get("layers")->get(2);
    auto dc_layer = json->get("layers")->get(0)->get("layers")->get(3);

    auto fl_tiledData = fl_layer->get("data")->asIntArray();
    auto wl_tiledData = wl_layer->get("data")->asIntArray();
    auto vf_tiledData = vf_layer->get("data")->asIntArray();
    auto dc_tiledData = dc_layer->get("data")->asIntArray();
    // iterate over the data set in parallel lol
    for (int row=0; row<mapHeight; row++){
        for(int col=0; col<mapWidth; col++){
            _floor.at(row).at(col) = fl_tiledData[row*mapWidth + col];
            _walls.at(row).at(col) = wl_tiledData[row*mapWidth + col];
            _visfix.at(row).at(col) = vf_tiledData[row*mapWidth + col];
            _deco.at(row).at(col) = dc_tiledData[row*mapWidth + col];

        }
    }
    
    // generate the spawn points
    for (auto item_spawn : json->get("layers")->get(1)->get("layers")->get(0)->get("objects")->children()){
        SpawnPoint sp = SpawnPoint(SpawnPoint::SpawnType::ITEM, item_spawn->getFloat("x"), item_spawn->getFloat("y"));
        _spawnPoints.push_back(sp);
    }
    
    auto pl_spawn = json->get("layers")->get(1)->get("layers")->get(1)->get("objects")->get(0);
    SpawnPoint sp = SpawnPoint(SpawnPoint::SpawnType::PLAYER, pl_spawn->getFloat("x"), pl_spawn->getFloat("y"));
    _spawnPoints.push_back(sp);
    
    for (auto enem_spawn : json->get("layers")->get(1)->get("layers")->get(2)->get("objects")->children()){
        SpawnPoint sp = SpawnPoint(SpawnPoint::SpawnType::ENEMY, enem_spawn->getFloat("x"), enem_spawn->getFloat("y"));
        _spawnPoints.push_back(sp);
    }
    
    // generate door info
    for (auto door_sp : json->get("layers")->get(1)->get("layers")->get(2)->get("objects")->children()){
        
    }
    return true;
}

bool LevelLoader::materialize() {
    shared_ptr<graphics::Image> image = graphics::Image::allocWithFile(_imgPath);
    auto tex = graphics::Texture::allocWithImage(image); // acc to spec this will work
    loadTilesFromImage(tex, _firstID);
    return true;
}

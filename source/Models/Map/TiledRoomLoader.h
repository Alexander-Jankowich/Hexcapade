// this file keeps the typed tiled room data and the loader together
// the goal is that map code can ask for a room in gameplay terms
// without needing to know how tmj or tsx files are structured on disk

#ifndef __HEXCAPADE_TILED_ROOM_LOADER_H__
#define __HEXCAPADE_TILED_ROOM_LOADER_H__

#include <cugl/cugl.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// this is the typed result of reading one tiled room file
// once the loader finishes we want the rest of the game to use this object
// instead of reaching back into raw json or tsx parsing details
class TiledRoomData {
    friend class TiledRoomLoader;

public:
    // this stores one tile layer exactly the way tiled authored it
    // we intentionally keep the raw gids here because tiled hides flip state inside those values
    // later draw code can then honor the artist's flips instead of losing them during load
    struct TileLayerData {
        std::string name;
        int width = 0;
        int height = 0;
        std::vector<unsigned int> gids;

        // this gives us the untouched gid that came from tiled
        // that matters because the raw value still contains any authored flip bits
        unsigned int getRawTileAt(int x, int y) const;

        // this gives gameplay code the cleaned-up tile id in our room-local coordinates
        // y 0 means the bottom row for us even though tiled stores rows top-down in the file
        unsigned int getTileAt(int x, int y) const;

        // these are small convenience helpers for the renderer
        // they let the draw path ask what flip state tiled authored for this cell
        bool isFlipHorizontalAt(int x, int y) const;
        bool isFlipVerticalAt(int x, int y) const;
        bool isFlipDiagonalAt(int x, int y) const;

        // this is the quick answer to whether the layer actually placed any tile here
        bool hasTileAt(int x, int y) const;

        // this is mostly for debugging and sanity checks when we want to confirm a layer loaded real data
        size_t countFilledTiles() const;
    };

    // this stores one marker from a tiled object layer
    // we keep both tiled-space and converted local-space positions because they help with debugging
    struct ObjectMarker {
        std::string name;
        std::string type;
        cugl::Vec2 tiledPosition = cugl::Vec2::ZERO;
        cugl::Vec2 localPosition = cugl::Vec2::ZERO;
        cugl::Size size = cugl::Size::ZERO;
    };

    // this stores the tileset information we need after following the tmj -> tsx -> png chain
    // once this is filled we can turn gids back into real textures on screen
    struct TilesetData {
        unsigned int firstGid = 1;
        int tileWidth = 0;
        int tileHeight = 0;
        int tileCount = 0;
        int columns = 0;
        int margin = 0;
        int spacing = 0;
        int imageWidth = 0;
        int imageHeight = 0;
        std::string tsxAssetPath;
        std::string imageAssetPath;
        std::string transparentColorHex;
        std::shared_ptr<cugl::graphics::Texture> texture;
        std::vector<std::shared_ptr<cugl::graphics::Texture>> subTextures;

        // this looks up the subtexture for one gid after we strip off any tiled flip bits
        std::shared_ptr<cugl::graphics::Texture> getTextureForGid(unsigned int gid) const;
    };

private:
    std::string _assetPath;
    int _width;
    int _height;
    int _tileWidth;
    int _tileHeight;

    TileLayerData _floorLayer;
    TileLayerData _wallLayer;
    TileLayerData _decoLayer;
    TileLayerData _visFixLayer;
    std::vector<uint8_t> _walkableFloorMask;

    std::vector<ObjectMarker> _itemSpawns;
    std::vector<ObjectMarker> _enemySpawns;
    std::vector<ObjectMarker> _bounds;
    ObjectMarker _playerSpawn;
    bool _hasPlayerSpawn;

    TilesetData _tileset;

public:
    // this starts empty and the loader fills in the authored room data
    TiledRoomData();

    const std::string& getAssetPath() const { return _assetPath; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    int getTileWidth() const { return _tileWidth; }
    int getTileHeight() const { return _tileHeight; }
    float getPixelWidth() const { return (float)(_width * _tileWidth); }
    float getPixelHeight() const { return (float)(_height * _tileHeight); }

    const TileLayerData& getFloorLayer() const { return _floorLayer; }
    const TileLayerData& getWallLayer() const { return _wallLayer; }
    const TileLayerData& getDecoLayer() const { return _decoLayer; }
    const TileLayerData& getVisFixLayer() const { return _visFixLayer; }

    const std::vector<ObjectMarker>& getItemSpawns() const { return _itemSpawns; }
    const std::vector<ObjectMarker>& getEnemySpawns() const { return _enemySpawns; }
    const std::vector<ObjectMarker>& getBounds() const { return _bounds; }
    bool hasPlayerSpawn() const { return _hasPlayerSpawn; }
    const ObjectMarker& getPlayerSpawn() const { return _playerSpawn; }
    const TilesetData& getTileset() const { return _tileset; }

    // these helpers keep map code simple when it needs quick questions about the room layout
    bool hasFloorAt(int x, int y) const;
    bool hasWallAt(int x, int y) const;
    // this is the important one for gameplay
    // it marks the connected playable interior floor area instead of trusting every floor-looking tile
    // that matters because some border art also lives on the floor layer and should not become walkable
    bool hasWalkableFloorAt(int x, int y) const;

    // this turns one gid back into the tileset art for rendering if that gid belongs to this room's tileset
    std::shared_ptr<cugl::graphics::Texture> getTextureForGid(unsigned int gid) const;

    // this gives us one stable floor gid we can reuse for hall visuals
    // the hall code uses this so halls still look like room floor instead of debug art
    unsigned int getPrimaryFloorGid() const;

    // this is just a compact debug string for logs and teammate walkthroughs
    std::string getDebugSummary() const;
};

// this class owns the actual tiled file reading work
// map generation should ask it for TiledRoomData instead of reopening tmj files itself
class TiledRoomLoader {
public:
    // this is the only entry point the rest of the game should call
    // give it a tmj asset path and it returns the fully interpreted room data
    static std::shared_ptr<TiledRoomData> loadFromAsset(const std::string& assetPath);

private:
    // this derives the connected playable interior from the authored art and player spawn
    // it is private because the loader should be the one place responsible for building that mask
    static void buildWalkableFloorMask(TiledRoomData& room);
};

#endif // __HEXCAPADE_TILED_ROOM_LOADER_H__

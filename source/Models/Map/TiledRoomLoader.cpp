// this file is the bridges between tiled files and our runtime room data
// so all of the tmj and tsx parsing details stay in one place

#include "TiledRoomLoader.h"

#include <algorithm>
#include <array>
#include <cugl/core/io/CUJsonReader.h>
#include <cugl/core/io/CUTextReader.h>
#include <queue>
#include <sstream>
#include <unordered_map>

using namespace cugl;
using namespace cugl::graphics;

namespace {
    // tiled stores each tile cell as one gid integer in the tmj data
    // that one integer is doing two jobs at once
    // the low bits tell us which tile image to draw from the tileset
    // and the top bits tell us whether the artist flipped or rotated that tile in tiled
    // these constants come from tiled's gid format so we can pull those authored flags back out
    constexpr unsigned int kTiledHorizontalFlipFlag = 0x80000000u;
    constexpr unsigned int kTiledVerticalFlipFlag = 0x40000000u;
    constexpr unsigned int kTiledDiagonalFlipFlag = 0x20000000u;
    constexpr unsigned int kTiledFlipMask = 0xE0000000u;

    // when we want the actual tile art we have to remove the flip bits first
    // otherwise the gid would be a huge mixed-together number instead of a real tileset index
    // this gives us the clean base tile id that matches a subtexture in the tsx image
    unsigned int stripTiledFlipFlags(unsigned int rawGid) {
        return rawGid & ~kTiledFlipMask;
    }

    // after we keep the raw gid; yes or no checks for drawing
    // these helpers keep the draw code readable instead of repeating raw bit math everywhere
    bool hasTiledHorizontalFlip(unsigned int rawGid) {
        return (rawGid & kTiledHorizontalFlipFlag) != 0;
    }

    bool hasTiledVerticalFlip(unsigned int rawGid) {
        return (rawGid & kTiledVerticalFlipFlag) != 0;
    }

    bool hasTiledDiagonalFlip(unsigned int rawGid) {
        return (rawGid & kTiledDiagonalFlipFlag) != 0;
    }

    // tiled usually saves paths relative to the file that is referencing them (?)
    // for example the tmj points to the tsx relative to the tmj folder
    // and then the tsx points to the png relative to the tsx folder
    // this helper gives us the folder piece so we can rebuild those next paths correctly
    std::string getAssetDirectory(const std::string& assetPath) {
        size_t slash = assetPath.find_last_of('/');
        if (slash == std::string::npos) {
            return "";
        }
        return assetPath.substr(0, slash + 1);
    }

    // this is the other half of the relative path story
    // once we know the current file's folder we can attach tiled's relative path onto it
    // that lets the same loader work even when the tmj and tsx are nested in asset folders
    std::string joinAssetPath(const std::string& baseDir, const std::string& relativePath) {
        if (relativePath.empty()) {
            return relativePath;
        }
        if (relativePath[0] == '/') {
            return relativePath;
        }
        return baseDir + relativePath;
    }

    // tiled gives us lots of named children inside arrays like layers and object groups
    // we use names such as floor wall art and objects as our contract with the room files
    // so this helper is just the repeated search for one child by its authored name
    std::shared_ptr<JsonValue> findNamedChild(const std::shared_ptr<JsonValue>& parentArray,
                                              const std::string& name) {
        if (!parentArray) {
            return nullptr;
        }

        for (size_t ii = 0; ii < parentArray->size(); ++ii) {
            std::shared_ptr<JsonValue> child = parentArray->get((int)ii);
            if (child && child->getString("name", "") == name) {
                return child;
            }
        }
        return nullptr;
    }

    // the tsx file is tiny for this project and we only need a few attributes from it
    // instead of pulling in a full xml parser we read the text and peel out the fields we care about
    // this helper grabs one attribute value from a tag string like source tilewidth or trans
    std::string extractXmlAttribute(const std::string& tagText, const std::string& key) {
        std::string pattern = key + "=\"";
        size_t start = tagText.find(pattern);
        if (start == std::string::npos) {
            return "";
        }
        start += pattern.size();
        size_t end = tagText.find('"', start);
        if (end == std::string::npos) {
            return "";
        }
        return tagText.substr(start, end - start);
    }

    // this is the same attribute read but for integer fields
    // we keep a default so missing optional tsx fields do not explode the whole load
    int extractXmlIntAttribute(const std::string& tagText, const std::string& key, int defaultValue) {
        std::string value = extractXmlAttribute(tagText, key);
        if (value.empty()) {
            return defaultValue;
        }
        return std::stoi(value);
    }

    // this pulls out a whole tag header so the attribute helpers above can read from it
    // for our current files we only need simple tags like tileset and image
    std::string extractXmlTag(const std::string& xmlText, const std::string& tagName) {
        std::string startPattern = "<" + tagName;
        size_t start = xmlText.find(startPattern);
        if (start == std::string::npos) {
            return "";
        }
        size_t end = xmlText.find('>', start);
        if (end == std::string::npos) {
            return "";
        }
        return xmlText.substr(start, end - start + 1);
    }

    // this copies one tile layer exactly as tiled wrote it into the tmj
    // we intentionally keep the raw gids here instead of cleaning them up too early
    // because later steps still need the flip bits for rendering
    bool readTileLayer(const std::shared_ptr<JsonValue>& layerNode,
                       TiledRoomData::TileLayerData& outLayer) {
        if (!layerNode || layerNode->getString("type", "") != "tilelayer") {
            return false;
        }

        outLayer.name = layerNode->getString("name", "");
        outLayer.width = layerNode->getInt("width", 0);
        outLayer.height = layerNode->getInt("height", 0);
        if (outLayer.width <= 0 || outLayer.height <= 0) {
            return false;
        }

        std::shared_ptr<JsonValue> dataNode = layerNode->get("data");
        if (!dataNode) {
            return false;
        }

        outLayer.gids.clear();
        outLayer.gids.reserve(dataNode->size());
        for (size_t ii = 0; ii < dataNode->size(); ++ii) {
            unsigned int rawGid = (unsigned int)dataNode->get((int)ii)->asInt(0);
            outLayer.gids.push_back(rawGid);
        }
        return true;
    }

    // older tiled tilesets can mark one rgb color as transparent in the tsx
    // our room art used that format at one point so we still understand it here
    // this helper turns something like ffffff into real rgb numbers we can compare against
    bool parseRgbHex(const std::string& value, Uint8& outR, Uint8& outG, Uint8& outB) {
        if (value.size() != 6) {
            return false;
        }

        unsigned int packed = 0;
        std::stringstream stream;
        stream << std::hex << value;
        stream >> packed;
        if (stream.fail()) {
            return false;
        }

        outR = (Uint8)((packed >> 16) & 0xFFu);
        outG = (Uint8)((packed >> 8) & 0xFFu);
        outB = (Uint8)(packed & 0xFFu);
        return true;
    }

    // if the tsx says one color should really be transparent we fix that before making the texture
    // without this step desktop would draw white boxes anywhere the artist expected transparency
    // mobile uses a preprocessed png now so both platforms can end up looking the same
    void applyTransparentColorKey(const std::shared_ptr<Image>& image,
                                  const std::string& transparentColorHex) {
        if (!image || transparentColorHex.empty()) {
            return;
        }

        Uint8 keyR = 0;
        Uint8 keyG = 0;
        Uint8 keyB = 0;
        if (!parseRgbHex(transparentColorHex, keyR, keyG, keyB)) {
            return;
        }

        std::vector<std::byte> pixels;
        image->get(pixels);
        if (pixels.empty()) {
            return;
        }

        for (size_t ii = 0; ii + 3 < pixels.size(); ii += 4) {
            Uint8 r = (Uint8)pixels[ii + 0];
            Uint8 g = (Uint8)pixels[ii + 1];
            Uint8 b = (Uint8)pixels[ii + 2];
            if (r == keyR && g == keyG && b == keyB) {
                pixels[ii + 0] = std::byte{0};
                pixels[ii + 1] = std::byte{0};
                pixels[ii + 2] = std::byte{0};
                pixels[ii + 3] = std::byte{0};
            }
        }

        image->set(pixels);
    }

    // tiled stores object markers in its own coordinate system with y going downward
    // our gameplay code treats the room's local space as y going upward from the bottom
    // we convert once here so every later system can use one consistent local coordinate space
    TiledRoomData::ObjectMarker readObjectMarker(const std::shared_ptr<JsonValue>& objectNode,
                                                 float mapPixelHeight) {
        TiledRoomData::ObjectMarker marker;
        if (!objectNode) {
            return marker;
        }

        marker.name = objectNode->getString("name", "");
        marker.type = objectNode->getString("type", "");
        marker.tiledPosition = Vec2(objectNode->getFloat("x", 0.0f),
                                    objectNode->getFloat("y", 0.0f));
        marker.localPosition = Vec2(marker.tiledPosition.x,
                                    mapPixelHeight - marker.tiledPosition.y);
        marker.size = Size(objectNode->getFloat("width", 0.0f),
                           objectNode->getFloat("height", 0.0f));
        return marker;
    }

    // this reads one whole object layer such as item_spawn or enemy_spawn
    // each marker gets converted into our local room coordinates right away
    // so later code does not need to remember how tiled's coordinate system works
    bool readObjectLayer(const std::shared_ptr<JsonValue>& layerNode,
                         float mapPixelHeight,
                         std::vector<TiledRoomData::ObjectMarker>& outMarkers) {
        if (!layerNode || layerNode->getString("type", "") != "objectgroup") {
            return false;
        }

        std::shared_ptr<JsonValue> objectsNode = layerNode->get("objects");
        if (!objectsNode) {
            return false;
        }

        outMarkers.clear();
        for (size_t ii = 0; ii < objectsNode->size(); ++ii) {
            outMarkers.push_back(readObjectMarker(objectsNode->get((int)ii), mapPixelHeight));
        }
        return true;
    }

    // this is where we follow the tmj -> tsx -> png chain
    // the tmj tells us which tileset file it uses
    // the tsx tells us how big each tile is and which image contains the art
    // and then we slice that image into the subtextures the room renderer will ask for later
bool loadTilesetData(const std::string& tmjAssetPath,
                     const std::shared_ptr<JsonValue>& root,
                     TiledRoomData::TilesetData& outTileset) {

    std::shared_ptr<JsonValue> tilesetsNode = root ? root->get("tilesets") : nullptr;
    if (!tilesetsNode || tilesetsNode->size() == 0) {
        CULog("FAIL: no tilesets node");
        return false;
    }

    std::shared_ptr<JsonValue> tilesetRef = tilesetsNode->get(0);
    if (!tilesetRef) {
        CULog("FAIL: tilesetRef null");
        return false;
    }

    outTileset.firstGid = (unsigned int)tilesetRef->getInt("firstgid", 1);

    std::string tmjDir = getAssetDirectory(tmjAssetPath);
    std::string source = tilesetRef->getString("source", "");
    CULog("TSX source: %s", source.c_str());

    outTileset.tsxAssetPath = joinAssetPath(tmjDir, source);
    CULog("Resolved TSX path: %s", outTileset.tsxAssetPath.c_str());

    if (outTileset.tsxAssetPath.empty()) {
        CULog("FAIL: empty tsx path");
        return false;
    }

    std::shared_ptr<TextReader> reader = TextReader::allocWithAsset(outTileset.tsxAssetPath);
    CULog("This is da path %s",outTileset.tsxAssetPath.c_str());
    if (!reader) {
        CULog("FAIL: could not open TSX file");
        return false;
    }

    std::string tsxText = reader->readAll();
    CULog("TSX loaded, size=%zu", tsxText.size());

    std::string tilesetTag = extractXmlTag(tsxText, "tileset");
    std::string imageTag = extractXmlTag(tsxText, "image");

    if (tilesetTag.empty()) {
        CULog("FAIL: no <tileset> tag");
        return false;
    }
    if (imageTag.empty()) {
        CULog("FAIL: no <image> tag");
        return false;
    }


        outTileset.tileWidth = extractXmlIntAttribute(tilesetTag, "tilewidth", 0);
        outTileset.tileHeight = extractXmlIntAttribute(tilesetTag, "tileheight", 0);
        outTileset.tileCount = extractXmlIntAttribute(tilesetTag, "tilecount", 0);
        outTileset.columns = extractXmlIntAttribute(tilesetTag, "columns", 1);
        outTileset.margin = extractXmlIntAttribute(tilesetTag, "margin", 0);
        outTileset.spacing = extractXmlIntAttribute(tilesetTag, "spacing", 0);
        outTileset.imageWidth = extractXmlIntAttribute(imageTag, "width", 0);
        outTileset.imageHeight = extractXmlIntAttribute(imageTag, "height", 0);
        outTileset.transparentColorHex = extractXmlAttribute(imageTag, "trans");
        if (outTileset.tileWidth <= 0 || outTileset.tileHeight <= 0 ||
            outTileset.tileCount <= 0 || outTileset.columns <= 0 ||
            outTileset.imageWidth <= 0 || outTileset.imageHeight <= 0) {
            return false;
        }

        std::string tsxDir = getAssetDirectory(outTileset.tsxAssetPath);
        outTileset.imageAssetPath = joinAssetPath(tsxDir, extractXmlAttribute(imageTag, "source"));
        if (outTileset.imageAssetPath.empty()) {
            return false;
        }

        // if the png already has alpha baked in we can load it directly everywhere
        // if the tsx still uses an old color key then desktop can still clean that up here
        // ios and opengles should use the preprocessed alpha png instead because image readback is limited there...
        if (outTileset.transparentColorHex.empty()) {
            outTileset.texture = Texture::allocWithFile(outTileset.imageAssetPath);
        } else {
#if CU_GRAPHICS_API == CU_OPENGLES
            outTileset.texture = Texture::allocWithFile(outTileset.imageAssetPath);
#else
            std::shared_ptr<Image> image = Image::allocWithFile(outTileset.imageAssetPath);
            if (!image) {
                return false;
            }
            applyTransparentColorKey(image, outTileset.transparentColorHex);
            outTileset.texture = Texture::allocWithImage(image);
#endif
        }
        if (!outTileset.texture) {
            return false;
        }

        outTileset.subTextures.clear();
        outTileset.subTextures.resize((size_t)(outTileset.firstGid + outTileset.tileCount));

        // tiled numbers tiles from the top-left of the image
        // cugl subtextures want pixel rectangles in the actual image
        // so this loop converts tiled's logical tile index into the source rect for each tile
        for (int ii = 0; ii < outTileset.tileCount; ++ii) {
            int column = ii % std::max(1, outTileset.columns);
            int rowFromTop = ii / std::max(1, outTileset.columns);
            int sx = outTileset.margin + column * (outTileset.tileWidth + outTileset.spacing);
            int topY = outTileset.margin + rowFromTop * (outTileset.tileHeight + outTileset.spacing);
            int sy = outTileset.imageHeight - topY - outTileset.tileHeight;
            if (sx < 0 || sy < 0 ||
                sx + outTileset.tileWidth > outTileset.imageWidth ||
                sy + outTileset.tileHeight > outTileset.imageHeight) {
                continue;
            }

            size_t gid = (size_t)(outTileset.firstGid + ii);
            outTileset.subTextures[gid] = outTileset.texture->getSubTexture(
                (Uint32)sx, (Uint32)sy, (Uint32)outTileset.tileWidth, (Uint32)outTileset.tileHeight
            );
        }

        return true;
    }

}

unsigned int TiledRoomData::TileLayerData::getRawTileAt(int x, int y) const {
    // tiled writes tile rows from top to bottom in the json array
    // our map code thinks of y 0 as the bottom row of the room
    // this flips the row lookup once so the rest of the game can stay in its usual coordinate system
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return 0;
    }

    int flippedY = (height - 1) - y;
    size_t index = (size_t)(flippedY * width + x);
    if (index >= gids.size()) {
        return 0;
    }
    return gids[index];
}

unsigned int TiledRoomData::TileLayerData::getTileAt(int x, int y) const {
    // most gameplay code only wants the base tile id and does not care about flip state
    // so this is the common lookup that returns the cleaned gid
    return stripTiledFlipFlags(getRawTileAt(x, y));
}

bool TiledRoomData::TileLayerData::isFlipHorizontalAt(int x, int y) const {
    return hasTiledHorizontalFlip(getRawTileAt(x, y));
}

bool TiledRoomData::TileLayerData::isFlipVerticalAt(int x, int y) const {
    return hasTiledVerticalFlip(getRawTileAt(x, y));
}

bool TiledRoomData::TileLayerData::isFlipDiagonalAt(int x, int y) const {
    return hasTiledDiagonalFlip(getRawTileAt(x, y));
}

bool TiledRoomData::TileLayerData::hasTileAt(int x, int y) const {
    return getTileAt(x, y) != 0;
}

size_t TiledRoomData::TileLayerData::countFilledTiles() const {
    // this is mostly a debug helper so we can log whether the room actually loaded meaningful art
    return (size_t)std::count_if(gids.begin(), gids.end(), [](unsigned int gid) {
        return stripTiledFlipFlags(gid) != 0;
    });
}

std::shared_ptr<Texture> TiledRoomData::TilesetData::getTextureForGid(unsigned int gid) const {
    // callers may hand us the raw gid straight from tiled data
    // we strip the flip bits first so we index into the actual subtexture array correctly
    gid = stripTiledFlipFlags(gid);
    if (gid >= subTextures.size()) {
        return nullptr;
    }
    return subTextures[(size_t)gid];
}

TiledRoomData::TiledRoomData() :
_width(0),
_height(0),
_tileWidth(0),
_tileHeight(0),
_hasPlayerSpawn(false) {
}

bool TiledRoomData::hasFloorAt(int x, int y) const {
    return _floorLayer.hasTileAt(x, y);
}

bool TiledRoomData::hasWallAt(int x, int y) const {
    return _wallLayer.hasTileAt(x, y);
}

bool TiledRoomData::hasWalkableFloorAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= _width || y >= _height) {
        return false;
    }
    size_t index = (size_t)(y * _width + x);
    return index < _walkableFloorMask.size() && _walkableFloorMask[index] != 0;
}

std::shared_ptr<Texture> TiledRoomData::getTextureForGid(unsigned int gid) const {
    return _tileset.getTextureForGid(gid);
}

unsigned int TiledRoomData::getPrimaryFloorGid() const {
    // halls need one floor-looking tile so they blend into the room art
    // this picks the most common authored floor tile from the room instead of hardcoding one
    std::unordered_map<unsigned int, size_t> counts;
    unsigned int bestGid = 0;
    size_t bestCount = 0;

    for (unsigned int gid : _floorLayer.gids) {
        if (gid == 0) {
            continue;
        }
        size_t nextCount = ++counts[gid];
        if (nextCount > bestCount) {
            bestCount = nextCount;
            bestGid = gid;
        }
    }

    return bestGid;
}

std::string TiledRoomData::getDebugSummary() const {
    std::ostringstream out;
    out << "tiled room " << _assetPath
        << " size=" << _width << "x" << _height
        << " tile=" << _tileWidth << "x" << _tileHeight
        << " floor=" << _floorLayer.countFilledTiles()
        << " wall=" << _wallLayer.countFilledTiles()
        << " deco=" << _decoLayer.countFilledTiles()
        << " item_spawns=" << _itemSpawns.size()
        << " enemy_spawns=" << _enemySpawns.size()
        << " player_spawn=" << (_hasPlayerSpawn ? "yes" : "no")
        << " tileset=" << _tileset.imageAssetPath;
    return out.str();
}

void TiledRoomLoader::buildWalkableFloorMask(TiledRoomData& room) {
    // this is one of the most important translation steps in the file
    // the room art has some floor-looking tiles that are really just border or frame detail
    // if we treated every floor tile as walkable then hallways would try to attach to fake openings
    // so we start from the player spawn and flood-fill only the connected interior floor region
    // that connected region becomes the real playable floor mask for gameplay and hallway code
    size_t gridSize = (size_t)(room._width * room._height);
    room._walkableFloorMask.assign(gridSize, 0);
    if (room._width <= 0 || room._height <= 0) {
        return;
    }

    // small local helpers keep the flood-fill readable
    auto inBounds = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < room._width && y < room._height;
    };

    // a tile only counts as open room floor if it has floor art and no wall art over it
    auto isOpenFloorTile = [&](int x, int y) {
        return room._floorLayer.hasTileAt(x, y) && !room._wallLayer.hasTileAt(x, y);
    };

    // the mask is stored as a flat array so this converts x y back into one index
    auto toIndex = [&](int x, int y) {
        return y * room._width + x;
    };

    // we want the walkable region that contains the player spawn
    // so we first turn the player marker into a tile coordinate
    // if the marker lands between cells or on a decorative spot we fall back to the nearest open tile
    int seedX = room._width / 2;
    int seedY = room._height / 2;
    if (room._hasPlayerSpawn && room._tileWidth > 0 && room._tileHeight > 0) {
        seedX = std::clamp((int)std::floor(room._playerSpawn.localPosition.x / room._tileWidth), 0, room._width - 1);
        seedY = std::clamp((int)std::floor(room._playerSpawn.localPosition.y / room._tileHeight), 0, room._height - 1);
    }

    if (!isOpenFloorTile(seedX, seedY)) {
        int bestDistance = std::numeric_limits<int>::max();
        int bestX = -1;
        int bestY = -1;
        for (int y = 0; y < room._height; ++y) {
            for (int x = 0; x < room._width; ++x) {
                if (!isOpenFloorTile(x, y)) {
                    continue;
                }
                int distance = std::abs(x - seedX) + std::abs(y - seedY);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestX = x;
                    bestY = y;
                }
            }
        }
        seedX = bestX;
        seedY = bestY;
    }

    if (!inBounds(seedX, seedY) || !isOpenFloorTile(seedX, seedY)) {
        return;
    }

    std::queue<cugl::Vec2> frontier;
    frontier.push(cugl::Vec2((float)seedX, (float)seedY));
    room._walkableFloorMask[(size_t)toIndex(seedX, seedY)] = 1;

    static const std::array<cugl::Vec2, 4> DIRS = {
        cugl::Vec2(1, 0), cugl::Vec2(-1, 0), cugl::Vec2(0, 1), cugl::Vec2(0, -1)
    };

    // this flood-fill walks through the connected playable interior and marks it once
    while (!frontier.empty()) {
        cugl::Vec2 current = frontier.front();
        frontier.pop();

        for (const cugl::Vec2& dir : DIRS) {
            int nx = (int)current.x + (int)dir.x;
            int ny = (int)current.y + (int)dir.y;
            if (!inBounds(nx, ny) || !isOpenFloorTile(nx, ny)) {
                continue;
            }
            size_t nextIndex = (size_t)toIndex(nx, ny);
            if (room._walkableFloorMask[nextIndex] != 0) {
                continue;
            }
            room._walkableFloorMask[nextIndex] = 1;
            frontier.push(cugl::Vec2((float)nx, (float)ny));
        }
    }
}

std::shared_ptr<TiledRoomData> TiledRoomLoader::loadFromAsset(const std::string& assetPath) {
    // step 1 reads the tmj itself which gives us the room structure and layer layout
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(assetPath);
    if (!reader) {
        CULog("no reader");
        return nullptr;
    }

    std::shared_ptr<JsonValue> root = reader->readJson();
    if (!root) {
        CULog("no json?");
        return nullptr;
    }

    // step 2 creates the typed room object that the rest of the game will actually use
    // after this point we try to leave raw json behind as quickly as possible
    std::shared_ptr<TiledRoomData> room = std::make_shared<TiledRoomData>();
    room->_assetPath = assetPath;
    room->_width = root->getInt("width", 0);
    room->_height = root->getInt("height", 0);
    room->_tileWidth = root->getInt("tilewidth", 0);
    room->_tileHeight = root->getInt("tileheight", 0);
    if (room->_width <= 0 || room->_height <= 0 || room->_tileWidth <= 0 || room->_tileHeight <= 0) {
        CULog("room dimension check");
        return nullptr;
    }

    // step 3 follows the tileset reference so the renderer can draw the real authored art
    // this is where the tmj stops being just ids and starts being tied to the actual png
    if (!loadTilesetData(assetPath, root, room->_tileset)) {
        CULog("613 failed");
        return nullptr;
    }

    // step 4 finds the layer groups we decided to use as our room authoring contract
    // for now we expect one art group and one objects group because that matches the provided files
    std::shared_ptr<JsonValue> rootLayers = root->get("layers");
    if (!rootLayers) {
        CULog("no root layers?");
        return nullptr;
    }

    std::shared_ptr<JsonValue> artGroup = findNamedChild(rootLayers, "art");
    std::shared_ptr<JsonValue> objectGroup = findNamedChild(rootLayers, "objects");
    if (!artGroup || !objectGroup) {
        CULog("no art or object?");
        return nullptr;
    }

    std::shared_ptr<JsonValue> artLayers = artGroup->get("layers");
    std::shared_ptr<JsonValue> objectLayers = objectGroup->get("layers");
    if (!artLayers || !objectLayers) {
        CULog("no art or object layers?");
        return nullptr;
    }

    // step 5 reads the room art layers
    // floor is required because it defines the base playable surface
    // the others are optional overlays that add walls and decoration
    if (!readTileLayer(findNamedChild(artLayers, "floor"), room->_floorLayer)) {
        CULog("no floor?");
        return nullptr;
    }
    readTileLayer(findNamedChild(artLayers, "wall"), room->_wallLayer);
    readTileLayer(findNamedChild(artLayers, "deco"), room->_decoLayer);
    readTileLayer(findNamedChild(artLayers, "vis_fix_dont_worry"), room->_visFixLayer);

    // step 6 reads the object markers like item spawns enemy spawns and the player start
    // we convert them into room-local coordinates here so later map code can place the room cleanly
    float mapPixelHeight = room->getPixelHeight();
    readObjectLayer(findNamedChild(objectLayers, "item_spawn"), mapPixelHeight, room->_itemSpawns);
    readObjectLayer(findNamedChild(objectLayers, "enemy_spawn"), mapPixelHeight, room->_enemySpawns);
    readObjectLayer(findNamedChild(objectLayers, "bounds"), mapPixelHeight, room->_bounds);

    std::vector<TiledRoomData::ObjectMarker> playerMarkers;
    readObjectLayer(findNamedChild(objectLayers, "player_spawn"), mapPixelHeight, playerMarkers);
    room->_hasPlayerSpawn = !playerMarkers.empty();
    if (room->_hasPlayerSpawn) {
        room->_playerSpawn = playerMarkers.front();
    }

    // step 7 derives the actual walkable room interior from the authored art and player spawn
    // this is the mask the map code trusts when it decides where the player can move and where halls can connect
    buildWalkableFloorMask(*room);
    CULog("room is null: %d", room==nullptr);
    return room;
}

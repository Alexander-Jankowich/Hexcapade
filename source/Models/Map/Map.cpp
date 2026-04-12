// this file generates and renders a full run layout made of rooms, halls,
// walls, and room-based spawn points

#include "Map.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>
#include <utility>

#define PHYSICS_SCALE 100
#define FLOOR_GID 66
#define EMPTY_TILE 0


using namespace cugl;
using namespace cugl::graphics;


Map::Map() :
_gridWidth(0),
_gridHeight(0),
_tileSize(0.0f),
_origin(Vec2::ZERO),
_worldBounds(Rect::ZERO),
_playerSpawn(Vec2::ZERO),
_config() {
}


bool Map::generate(const cugl::Size &viewSize, const MapGenerationConfig &config, unsigned int seed, std::shared_ptr<LevelLoader> tileData) {
    
}

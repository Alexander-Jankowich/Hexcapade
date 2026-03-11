//
//  EnemySet.h
//
#ifndef __ENEMY_SET_H__
#define __ENEMY_SET_H__
#include <cugl/cugl.h>
#include "Enemy.h"

class EnemySet {
public:
    /** Hash map of enemyIds to enemy objects*/
    std::unordered_map<int, std::shared_ptr<Enemy>> current;
    
    int _totalCount;
    
    EnemySet();

    void dispose();

    void reset();
    
#pragma mark Graphics
    void setTexture(const std::shared_ptr<cugl::graphics::Texture>& value);

    void spawnEnemy(const cugl::Vec2 position, int health);
    
    void clientSpawnEnemy(int eid, const cugl::Vec2 position, int health);
    
    void updateEnemyAsClient(int eid, const cugl::Vec2 position, int health);

    void update(std::vector<cugl::Vec2> targets);

    void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

    float getRadius() { return _radius; }

private:
    float _radius;
    std::shared_ptr<cugl::graphics::Texture> _texture;
};
#endif // !__ENEMY_SET_H__

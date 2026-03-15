//
//  PlayerSet.h
//  Hexcapade
//
//  This class tracks all other players in the current game
//
//  Author: Kaz Tam
//  Version: 3/05/26
//

#ifndef __PLAYER_SET_H__
#define __PLAYER_SET_H__
#include <cugl/cugl.h>
#include "Player.h"

/**
 * Model class which holds a set of players.
 */
class PlayerSet {
protected:

public:

	PlayerSet();

    std::unordered_set<std::shared_ptr<Player>> current;

	void addPlayer(std::shared_ptr<Player>);
    
    int numPlayers() {return current.size();}

	//void update();
    std::shared_ptr<Player> getPlayerByPID(std::string pid) {
        for (auto p : current) {
            if (p->getPID()==pid) return p;
        }
        return nullptr; // should NOT get here
    }
    
    void reset() {
        for (auto p : current) {
            p->reset();
        }
    }

	void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

};

#endif // !__PLAYER_SET_H__

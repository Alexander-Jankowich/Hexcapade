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
	std::unordered_set<std::shared_ptr<Player>> current;

	PlayerSet();

	void addPlayer(std::shared_ptr<Player>);

	//void update();

	void draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);

};

#endif
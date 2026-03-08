//
//  PlayerSet.cpp
//  Hexcapade
//
//  This class tracks all other players in the current game
//
//  Author: Kaz Tam
//  Version: 3/05/26
//

#include "PlayerSet.h"

using namespace cugl;

void PlayerSet::addPlayer(std::shared_ptr<Player> p) {
	current.emplace(p);
}

void PlayerSet::draw(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
    for (auto it = current.begin(); it != current.end(); ++it) {
        (*it)->draw(batch);
    }
}
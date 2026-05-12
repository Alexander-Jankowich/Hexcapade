//
//  EndScene.h
//  Hexcapade
//
#ifndef __END_SCENE_H__
#define __END_SCENE_H__
#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class EndScene : public cugl::scene2::Scene2 {
public:
    enum class State { NONE, WIN, LOSE };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<NetworkController> _networkCtrl;
    std::shared_ptr<cugl::scene2::Button> _replayButton;
    std::shared_ptr<cugl::scene2::Button> _exitButton;
    std::shared_ptr<cugl::scene2::Label> _resultLabel;
    std::vector<std::shared_ptr<cugl::scene2::SceneNode>> _playerSlots;
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _playerPortraits;
    std::vector<std::shared_ptr<cugl::scene2::Label>> _playerNames;
    std::vector<std::shared_ptr<cugl::scene2::Label>> _playerDeposits;
    std::vector<std::string> _endPlayerIds;
    State _state;
    bool  _replayPressed;
    // EXIT BUTTON
    bool  _exitPressed;

    void updatePlayerStats();

public:
    EndScene() : cugl::scene2::Scene2(), _state(State::NONE), _replayPressed(false), _exitPressed(false) {}
    ~EndScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    void setNetwork(std::shared_ptr<NetworkController> n) {_networkCtrl=n;}
    void show(bool won);
    

    bool isVisible()      const { return _state != State::NONE; }
    bool isHost() const {return _networkCtrl->isHost();}
    std::shared_ptr<NetworkController> getNetwork() const {return _networkCtrl;}
    void setActive(bool active) override;

    void release() {_networkCtrl = nullptr;}
    bool didPressReplay() const { return (_replayPressed && _networkCtrl->isHost()) || (_networkCtrl->gameState()==NetworkController::NCGameState::UNSTARTED); }
    // EXIT BUTTON
    bool didPressExit() const { return _exitPressed; }

    void update(float dt) override;
    void render() override;
};

#endif /* __END_SCENE_H__ */

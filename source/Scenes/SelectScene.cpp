//
//  HostSelectScene.cpp
//  alpha_final
//
//  Created by Serena Duncan on 4/20/26.
//
#include "SelectScene.h"
using namespace cugl;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

#pragma mark Init / Dispose

bool SelectScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::initWithHint(Size(0,SCENE_HEIGHT))) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    
    Size dimen = getSize();
    
    // Acquire the scene built by the asset loader and resize it the scene
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("char-select");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("char-select.center.start"));

    _startgame->addListener([this](const std::string& name, bool down) {
        if (down) {
            if (!_networkCtrl->isHost()){
                _status = SSStatus::HANDSHAKE;
            }
            else { // host starting game
                _networkCtrl->pushOutEvent(StartGameEvent::allocStartGameEvent());
            }
            _startPressed = true;
            CULog("start pressed, is Host: %d", _networkCtrl->isHost());
        }
    });
    
    addChild(scene);
    setActive(false);
    return true;
}

void SelectScene::dispose() {
    if (_active) {
        removeAllChildren();
        _networkCtrl=nullptr;
        _active = false;
    }
}

#pragma mark Update / Render

void SelectScene::update(float dt) {
    _networkCtrl->updateNet();
    _networkCtrl->customEvents();
    configureStartButton();
    // if host and all connected AND map init, broadcast startgame
    if (_networkCtrl->canHostStart() && _networkCtrl->getRandSeed()!=0) {
        _status=SSStatus::HANDSHAKE;
    }
    // as client, mark ready once player has selected and map was init
    // have to use pushOutEvent bc of checks in markReady()
    if (!_networkCtrl->isHost() && _startPressed && _networkCtrl->getRandSeed()!=0) {
        CULog("client is ready");
        _networkCtrl->pushOutEvent(GameStateEvent::allocReady());
    }
    // on recieve start game, transition to game
    if (_networkCtrl->gameState()==NetworkController::NCGameState::RUNNING) _status = SSStatus::START;
}

void SelectScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            if (_networkCtrl->isHost()) {
                _networkCtrl->pushOutEvent(InitMapEvent::allocInitMapEvent(_networkCtrl->getRandSeed()));
                _startgame->deactivate();
                // mark players unready
                _networkCtrl->unreadyPlayers();
                CULog("host set active");
            }
            _status = SSStatus::SELECT;
            _startPressed = false;
            _startgame->setDown(false);
            configureStartButton();
        } else {
            _startgame->deactivate();
            // If any were pressed, reset them
            _startPressed = false;
            _startgame->setDown(false);
        }
    }
}

void SelectScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("up")->getChildByName("label"));
    label->setText(text);

}

void SelectScene::configureStartButton() {
    if(_status==SSStatus::SELECT) { // players choose their character, ONLY MULTIPLAYER
        auto text = _networkCtrl->isHost() ? "Waiting for Players" : "Ready";
        updateText(_startgame, text);
        if (!_networkCtrl->isHost()) _startgame->activate();
    }
    else if (_status==SSStatus::HANDSHAKE) {
        auto text = _networkCtrl->isHost() ? "Start Game" : "Awaiting Host Start";
        updateText(_startgame, text);
        if (_networkCtrl->isHost()) _startgame->activate();
    }
    else {
        updateText(_startgame, "Starting Game");
        _startgame->deactivate();
    }
}

//
//  HostScene.cpp
//  Hexcapade
//
#include "HostScene.h"

using namespace cugl;
using namespace cugl::audio;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

#pragma mark Init / Dispose

bool HostScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<NetworkController>& network) {
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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("host");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD

    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host.host_panel.buttons.join"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("host.return"));
    _gameid = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host.host_panel.buttons.code.numpad_code"));
    _player = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("host.host_panel.buttons.players.party_size"));
    
    // Program the buttons
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            // back is a ui tap so play click before leaving the room
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            _backPressed = true;
            disconnect();
        }
    });

    _startgame->addListener([this](const std::string& name, bool down) {
        if (down) {
            // start game click plays locally before the network starts the game
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            _networkCtrl->startGame();
        }
    });
    
    _networkCtrl = network;
    addChild(scene);
    setActive(false);
    return true;
}

void HostScene::dispose() {
    if (_active) {
        removeAllChildren();
        _networkCtrl=nullptr;
        _active = false;
    }
}

#pragma mark Update / Render

void HostScene::update(float dt) {
    _networkCtrl->updateNet();
    _player->setText(std::to_string(_networkCtrl->getNumPlayers()));
    if (_networkCtrl->getStatus() == NetworkController::Status::CONNECTED && _networkCtrl->hasNetwork()) {
        //_gameid->setText(_networkCtrl.getRoomCode());
        std::string code = _networkCtrl->getRoomCode();
        if (!code.empty()) {              // ← add this guard
            _gameid->setText(code);
        }
    }
    configureStartButton();
    _networkCtrl -> markReady();
}

void HostScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _backPressed = false;
            configureStartButton();
            _backout->activate();
            _startgame->activate();
            _networkCtrl->connectHost();
        } else {
            _startgame->deactivate();
            _backout->deactivate();
            // If any were pressed, reset them
            _startgame->setDown(false);
            _backout->setDown(false);
        }
    }
}

void HostScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("Start"));
    label->setText(text);

}

void HostScene::configureStartButton() {
    if(_networkCtrl->getStatus()<=NetworkController::Status::CONNECTING) {
        updateText(_startgame,"Waiting");
        _startgame->deactivate();

    }
    else if(_networkCtrl->getStatus()==NetworkController::Status::CONNECTED) {
        updateText(_startgame, "Start");
        _startgame->activate();
    }
    else if (_networkCtrl->getStatus()==NetworkController::Status::HANDSHAKE) {
        updateText(_startgame, "Starting Game");
        _startgame->deactivate();
    }
}

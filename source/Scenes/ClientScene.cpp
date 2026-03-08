//
//  ClientScene.cpp
//  Hexcapade
//
#include "ClientScene.h"

using namespace cugl;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

#pragma mark Init / Dispose

bool ClientScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("client");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    
    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client.center.start"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client.back"));
    _gameid = std::dynamic_pointer_cast<scene2::TextField>(_assets->get<scene2::SceneNode>("client.center.game.field.text"));
    _player = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client.center.players.field.text"));
    
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            disconnect();
        }
    });

    _startgame->addListener([=,this](const std::string& name, bool down) {
        if (down) {
            // This will call the _gameid listener
            _gameid->releaseFocus();
        }
    });
    
    _gameid->addExitListener([this](const std::string& name, const std::string& value) {
        if (!value.empty()) {
            _networkCtrl.connect(false, value);
        }
    });

    // Create the server configuration
    auto json = _assets->get<JsonValue>("server");
    _networkCtrl.init(json, false);
    
    addChild(scene);
    setActive(false);
    return true;
}

void ClientScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
        _networkCtrl.disconnect();
        _assets = nullptr;
        _font = nullptr;
    }

}


void ClientScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("up")->getChildByName("label"));
    label->setText(text);

}

void ClientScene::update(float dt) {
    _networkCtrl.update();
    _player->setText(std::to_string(_networkCtrl.playerCount()));
    configureStartButton();

    // TODO outsource this to network controller
//    if (_networkCtrl.getNetwork()) {
//        _networkCtrl.getNetwork()->receive([this](const std::string& source,
//            const std::vector<std::byte>& data) {
//                // Any message from the host means the game is starting
//                if (!data.empty()) {
//                    _status = Status::START;
//                }
//            });
//        checkConnection();
//    }
}

void ClientScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _gameid->activate();
            _startgame->activate();
            _backout->activate();
            _player->setText("1");
            configureStartButton();
            // Don't reset the room id
        } else {
            _gameid->deactivate();
            _startgame->deactivate();
            _backout->deactivate();
            // If any were pressed, reset them
            _startgame->setDown(false);
            _backout->setDown(false);
        }
    }
}

/**
 * Reconfigures the start button for this scene
 *
 * This is necessary because what the buttons do depends on the state of the
 * networking.
 */
void ClientScene::configureStartButton() {
    switch (_networkCtrl.getStatus()) {
        case NetworkController::Status::IDLE:
            updateText(_startgame, "Join Game");
            break;
        case NetworkController::Status::JOIN:
            updateText(_startgame, "Connecting");
            _startgame->deactivate();
            break;
        case NetworkController::Status::WAIT:
            updateText(_startgame, "Waiting");
            break;
        default:
            break;
    }
}

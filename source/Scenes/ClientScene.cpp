//
//  ClientScene.cpp
//  Hexcapade
//
#include "ClientScene.h"

using namespace cugl;
using namespace cugl::audio;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

#pragma mark Init / Dispose

bool ClientScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<NetworkController>& network) {
    canInitGame=false;
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
    
    _startgame = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client.client_left_panel.buttons.join"));
    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client.client_left_panel.buttons.return"));
    _deletebutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("client.client_keypad_panel.keypad.del"));
    _gameid = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client.client_left_panel.buttons.code.numpad_code"));
    _player = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("client.client_left_panel.buttons.players.party_size"));
    for (int ii = 0; ii < 10; ii++) {
        std::string key = "client.client_keypad_panel.keypad." + std::to_string(ii);
        _keypad[ii] = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>(key));
    }
    
    _backout->addListener([this](const std::string& name, bool down) {
        if (down) {
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            _backPressed = true;
            disconnect();
        }
    });

    _startgame->addListener([=,this](const std::string& name, bool down) {
        if (down) {
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            if (!_roomInput.empty()) {
                _networkCtrl->connectClient(_roomInput);
            }
        }
    });

    _deletebutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            // delete is still a button tap even when there is nothing to remove
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            if (!_roomInput.empty()) {
                _roomInput.pop_back();
                _gameid->setText(_roomInput);
            }
        }
    });
    
    for (int ii = 0; ii < 10; ii++) {
        _keypad[ii]->addListener([this, ii](const std::string& name, bool down) {
            if (down) {
                // play the tap sound before checking the code length
                // this keeps number buttons clicking even after five digits
                auto sound = _assets->get<Sound>("general_click");
                AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
                if (_roomInput.size() < 5) {
                    _roomInput.push_back((char)('0' + ii));
                    _gameid->setText(_roomInput);
                }
            }
        });
    }

    // Create the server configuration
    _networkCtrl = network;
    
    addChild(scene);
    setActive(false);
    return true;
}

void ClientScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
        _networkCtrl=nullptr;
        _assets = nullptr;
    }

}


void ClientScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("Start"));
    label->setText(text);

}

void ClientScene::update(float dt) {
    _networkCtrl->updateNet();
    
    _player->setText(std::to_string(_networkCtrl->getNumPlayers()));
    configureStartButton();
    // always mark ready once connected so we can move to INGAME
    _networkCtrl->markReady();
}

void ClientScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _backPressed = false;
            _startgame->activate();
            _backout->activate();
            _deletebutton->activate();
            for (auto& button : _keypad) {
                button->activate();
            }
            _player->setText("1");
            configureStartButton();
            // Don't reset the room id
        } else {
            _startgame->deactivate();
            _backout->deactivate();
            _deletebutton->deactivate();
            for (auto& button : _keypad) {
                button->deactivate();
                button->setDown(false);
            }
            // If any were pressed, reset them
            _startgame->setDown(false);
            _backout->setDown(false);
            _deletebutton->setDown(false);
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
    switch (_networkCtrl->getStatus()) {
        case NetworkController::Status::IDLE:
            updateText(_startgame, "Join");
            break;
        case NetworkController::Status::CONNECTING:
            updateText(_startgame, "Connecting");
            _startgame->deactivate();
            break;
        case NetworkController::Status::CONNECTED:
            updateText(_startgame, "Waiting for Host");
            break;
        default:
            break;
    }
}

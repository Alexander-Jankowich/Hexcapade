//
//  EndScene.cpp
//  Hexcapade
//
#include "EndScene.h"

using namespace cugl;
using namespace cugl::audio;
using namespace cugl::graphics;
#define SCENE_HEIGHT 720

static std::string endPortraitKey(const std::string& character) {
    if (character == "player_cat") return "cat_pr";
    if (character == "player_duck") return "duck_pr";
    if (character == "player_bunny") return "bunny_pr";
    return "fox_pr";
}

bool EndScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<NetworkController>& network) {
    if (!assets) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;

    _assets = assets;
    _networkCtrl = network;
    _state = State::NONE;
    _replayPressed = false;
    _exitPressed = false;

    Size dimen = getSize();
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("end");
    scene->setContentSize(dimen);
    scene->doLayout();

    _replayButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("end.next_round"));
    _exitButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("end.quit"));
    _resultLabel = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("end.result.result_text"));
    
    auto addPlayerSlot = [this](const std::string& player) {
        std::string slot = "end.player_stats." + player;

        _playerSlots.push_back(_assets->get<scene2::SceneNode>(slot));
        _playerPortraits.push_back(std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>(slot + "." + player + "_texture")));
        _playerNames.push_back(std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>(slot + "." + player + "_name")));
        _playerDeposits.push_back(std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>(slot + "." + player + "_text")));
    };

    addPlayerSlot("player_1");
    addPlayerSlot("player_2");
    addPlayerSlot("player_3");
    addPlayerSlot("player_4");

    _replayButton->addListener([this](const std::string& name, bool down) {
        if (down && _networkCtrl->isHost()) {
            // replay click plays locally before the host sends restart
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            _replayPressed = true;
            CULog("host sending restart");
            _networkCtrl->pushOutEvent(RestartGameEvent::allocRestartGameEvent(0));
            _networkCtrl->updateNet();
        }
    });

    // EXIT BUTTON
    _exitButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            // exit click plays locally before returning to menu
            auto sound = _assets->get<Sound>("general_click");
            AudioEngine::get()->play("general_click", sound, false, 1.0f, true);
            // exit button placeholder
            _exitPressed = true;
        }
    });

    addChild(scene);
    setActive(false);
    return true;
}

void EndScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
    _assets = nullptr;
    _networkCtrl = nullptr;
    _replayButton = nullptr;
    _exitButton = nullptr;
    _resultLabel = nullptr;
    _playerSlots.clear();
    _playerPortraits.clear();
    _playerNames.clear();
    _playerDeposits.clear();
    _endPlayerIds.clear();
}

void EndScene::show(bool won) {
    _state = won ? State::WIN : State::LOSE;
    _resultLabel->setText(won ? "Success" : "Failure");
    
    // get snapshot of connected player ids for this end screen
    // each id is used to look up its name, character, and deposit total - better way?
    _endPlayerIds.clear();
    for (const auto& pid : _networkCtrl->getPlayers()) {
        _endPlayerIds.push_back(pid);
    }
    updatePlayerStats();
}

void EndScene::updatePlayerStats() {
    // hide every slot first (stale data)
    for (auto& slot : _playerSlots) {
        slot->setVisible(false);
    }

    // fill one endgame slot for each player who finished the round
    int index = 0;
    for (const auto& pid : _endPlayerIds) {
        if (index == (int)_playerSlots.size()) break;

        Size portraitSize = _playerPortraits[index]->getContentSize();

        // swap to the portrait that matches the character chosen this round
        auto portrait = _assets->get<Texture>(endPortraitKey(_networkCtrl->getPlayerCharacter(pid)));
        _playerPortraits[index]->setTexture(portrait);
        
        // resize portrait to fit inside image section
        _playerPortraits[index]->setContentSize(portraitSize);

        // player name, player deposit amount
        _playerNames[index]->setText(_networkCtrl->getPlayerName(pid));
        _playerDeposits[index]->setText("Deposited: " + std::to_string(_networkCtrl->getPlayerDeposit(pid)));
        _playerSlots[index]->setVisible(true);
        index++;
    }
}

void EndScene::update(float dt) {
    _networkCtrl->updateNet();
    _networkCtrl->customEvents();
}

void EndScene::render() {
    if (_state == State::NONE) return;
    Scene2::render();
}

void EndScene::setActive(bool active) {
    if (isActive() != active) {
        Scene2::setActive(active);
        if (active) {
            _replayPressed = false;
            _exitPressed = false;
            _exitButton->activate();
            
            if (!_networkCtrl->isHost()) {
                _replayButton->deactivate();
                updateText(_replayButton, "Awaiting Host");
            }
            else {
                _replayButton->activate();
                auto text = _networkCtrl->gameState()==NetworkController::NCGameState::WON ? "Next Round" : "Try Again";
                updateText(_replayButton, text);
            }
        } else {
            _state = State::NONE;
            _replayPressed = false;
            _exitPressed = false;
            _replayButton->deactivate();
            _exitButton->deactivate();
            _replayButton->setDown(false);
            _exitButton->setDown(false);
        }
    }
}

void EndScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("action_text"));
    label->setText(text);

}

#include "SelectScene.h"
using namespace cugl;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

bool SelectScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;

    _assets = assets;
    Size dimen = getSize();

    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("char-select");
    if (!scene) return false;

    scene->setContentSize(dimen);
    scene->doLayout();

    _startgame = std::dynamic_pointer_cast<scene2::Button>(scene->getChildByName("start"));
    if (!_startgame) return false;

    _portrait = std::dynamic_pointer_cast<scene2::PolygonNode>(scene->getChildByName("selected_char"));
    if (!_portrait) return false;

    auto namingNode = scene->getChildByName("naming");
    if (namingNode) {
        _nameField = std::dynamic_pointer_cast<scene2::TextField>(namingNode->getChildByName("player_name"));
    }

    auto charButtonsNode = scene->getChildByName("char_buttons");
    if (!charButtonsNode) return false;

    _catBtn = std::dynamic_pointer_cast<scene2::Button>(charButtonsNode->getChildByName("cat"));
    _foxBtn = std::dynamic_pointer_cast<scene2::Button>(charButtonsNode->getChildByName("fox"));
    _duckBtn = std::dynamic_pointer_cast<scene2::Button>(charButtonsNode->getChildByName("duck"));
    _bunnyBtn = std::dynamic_pointer_cast<scene2::Button>(charButtonsNode->getChildByName("bunny"));

    if (!_catBtn || !_foxBtn || !_duckBtn || !_bunnyBtn) return false;

    _startgame->addListener([this](const std::string& name, bool down) {
        if (!down) return;
        if (_networkCtrl->isHost()) {
            _networkCtrl->pushOutEvent(StartGameEvent::allocStartGameEvent());
        }
        else {
            _status = SSStatus::HANDSHAKE;
        }
        _networkCtrl->setPlayerName(_networkCtrl->getPID(), _playerName);
        _networkCtrl->pushOutEvent(PlayerNameEvent::allocPlayerNameEvent(_networkCtrl->getPID(), _playerName));
        _startPressed = true;
        _nameField->releaseFocus();
        });
        
    _nameField->addTypeListener([this](const std::string&, const std::string& value) {
        std::string clamped = value.size() > 12 ? value.substr(0, 12) : value; // max 12 characters for now
        if (clamped != value && _nameField) {
            _nameField->setText(clamped);
        }
        _playerName = clamped;
    });

    _catBtn->addListener([this](const std::string& name, bool down) {
        if (down) onCharacterSelected(CharacterType::CAT);
        });
    _foxBtn->addListener([this](const std::string& name, bool down) {
        if (down) onCharacterSelected(CharacterType::FOX);
        });
    _duckBtn->addListener([this](const std::string& name, bool down) {
        if (down) onCharacterSelected(CharacterType::DUCK);
        });
    _bunnyBtn->addListener([this](const std::string& name, bool down) {
        if (down) onCharacterSelected(CharacterType::BUNNY);
        });

    addChild(scene);
    setActive(false);
    return true;
}

void SelectScene::dispose() {
    if (_active) {
        removeAllChildren();
        _networkCtrl = nullptr;
        _active = false;
    }
}

void SelectScene::onCharacterSelected(CharacterType c) {
    _selectedCharacter = c;
    _characterChosen = true;

    auto tex = _assets->get<graphics::Texture>(portraitKey(c));
    if (tex) {
        _portrait->setTexture(tex);
        _portrait->setContentSize(Size(396, 475));
        _portrait->setVisible(true);
    }

    configureStartButton();
}

void SelectScene::update(float dt) {
    if (!_networkCtrl) return;

    _networkCtrl->updateNet();
    _networkCtrl->customEvents();
    configureStartButton();

    if (_networkCtrl->isHost() && _networkCtrl->canHostStart() && _networkCtrl->getRandSeed() != 0) {
        _status = SSStatus::HANDSHAKE;
    }

    if (!_networkCtrl->isHost() && _startPressed && !_readySent && _networkCtrl->getPhysEnabled()) {
        _networkCtrl->pushOutEvent(GameStateEvent::allocReady());
        _readySent = true;
    }

    if (_networkCtrl->gameState() == NetworkController::NCGameState::RUNNING) {
        _status = SSStatus::START;
    }
}

void SelectScene::setActive(bool value) {
    if (isActive() == value) return;
    Scene2::setActive(value);

    if (value) {
        _selectedCharacter = CharacterType::FOX;
        _characterChosen = false;
        _startPressed = false;
        _readySent = false;
        _status = SSStatus::SELECT;

        _portrait->setVisible(false);

        _playerName = "PLAYER";
        if (_nameField) {
            _nameField->setText("PLAYER");
            _nameField->activate();
        }

        _catBtn->activate();
        _foxBtn->activate();
        _duckBtn->activate();
        _bunnyBtn->activate();

        if (_networkCtrl && _networkCtrl->isHost()) {
            _networkCtrl->unreadyPlayers();
            _networkCtrl->rerollRandSeed();
            CULog("send map init");
            _networkCtrl->pushOutEvent(InitMapEvent::allocInitMapEvent(_networkCtrl->getRandSeed()));
            _startgame->deactivate();
        }

        configureStartButton();

    }
    else {
        _startgame->deactivate();
        _startgame->setDown(false);
        _catBtn->deactivate();
        _foxBtn->deactivate();
        _duckBtn->deactivate();
        _bunnyBtn->deactivate();
        if (_nameField) {
            _nameField->deactivate();
        }
        _startPressed = false;
        _readySent = false;
    }
}

void SelectScene::configureStartButton() {
    if (_status == SSStatus::SELECT) {
        auto text = _networkCtrl->isHost() ? "Waiting for Players" : "Ready";
        updateText(_startgame, text);
        if (!_networkCtrl->isHost() && _characterChosen) _startgame->activate();
    }
    else if (_status == SSStatus::HANDSHAKE) {
        auto text = _networkCtrl->isHost() ? "Start Game" : "Awaiting Host Start";
        updateText(_startgame, text);
        if (_networkCtrl->isHost() && _characterChosen) _startgame->activate();
    }
    else {
        updateText(_startgame, "Starting Game");
        _startgame->deactivate();
    }
}

void SelectScene::updateText(const std::shared_ptr<scene2::Button>& button, const std::string text) {
    auto label = std::dynamic_pointer_cast<scene2::Label>(button->getChildByName("Start"));
    label->setText(text);

}

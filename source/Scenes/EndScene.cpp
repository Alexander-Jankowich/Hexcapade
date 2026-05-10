//
//  EndScene.cpp
//  Hexcapade
//
#include "EndScene.h"

using namespace cugl;
using namespace cugl::graphics;
#define SCENE_HEIGHT 720
bool EndScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (!assets) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;

    _assets = assets;
    _state = State::NONE;
    _replayPressed = false;

    Size dimen = getSize();
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("end");
    scene->setContentSize(dimen);
    scene->doLayout();

    _replayButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("end.next_round"));
    _exitButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("end.quit"));
    _resultLabel = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("end.result.result_text"));

    _replayButton->addListener([this](const std::string& name, bool down) {
        if (down && _networkCtrl->isHost()) {
            _replayPressed = true;
            CULog("host sending restart");
            _networkCtrl->pushOutEvent(RestartGameEvent::allocRestartGameEvent(0));
            _networkCtrl->updateNet();
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
}

void EndScene::show(bool won) {
    _state = won ? State::WIN : State::LOSE;
    _resultLabel->setText(won ? "Success" : "Failure");
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
            _replayButton->activate();
            _exitButton->deactivate();
            if (!_networkCtrl->isHost()) {
                _replayButton->deactivate();
            }
        } else {
            _state = State::NONE;
            _replayPressed = false;
            _replayButton->deactivate();
            _exitButton->deactivate();
            _replayButton->setDown(false);
            _exitButton->setDown(false);
        }
    }
}

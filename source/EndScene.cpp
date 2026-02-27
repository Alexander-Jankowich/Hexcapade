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
    _font = assets->get<Font>("font");
    _state = State::NONE;
    _replayPressed = false;

    // Centre the replay button on screen
    Size s = getSize();
    float bw = 300.0f, bh = 60.0f;
    _buttonRect = Rect(s.width / 2 - bw / 2, s.height / 2 - 80, bw, bh);

#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        touch->addEndListener(LISTENER_KEY, [this](const TouchEvent& event, bool focus) {
            this->touchEndCB(event, focus);
            });
    }
#else
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->addReleaseListener(LISTENER_KEY, [this](const MouseEvent& event, Uint8 clicks, bool focus) {
            this->mouseReleaseCB(event, clicks, focus);
            });
    }
#endif

    return true;
}

void EndScene::dispose() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) touch->removeEndListener(LISTENER_KEY);
#else
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) mouse->removeReleaseListener(LISTENER_KEY);
#endif
    _assets = nullptr;
    _font = nullptr;
}

void EndScene::show(bool won) {
    _state = won ? State::WIN : State::LOSE;
    _replayPressed = false;
}

void EndScene::update(float dt) {
    // Input is handled via listeners; nothing to poll here.
    // Reset _replayPressed each frame so it only fires for one frame.
    // (Caller should read it before calling update again.)
}

#ifdef CU_TOUCH_SCREEN
void EndScene::touchEndCB(const TouchEvent& event, bool focus) {
    if (_state == State::NONE) return;

    // Mirror the /3 scaling used in InputController::touch2Screen
    Vec2 pos;
    pos.x = event.position.x / 2.0f + 100;
    pos.y = event.position.y / 3.0f + 60;

    if (_buttonRect.contains(pos)) {
        _replayPressed = true;
    }
}
#else
void EndScene::mouseReleaseCB(const MouseEvent& event, Uint8 clicks, bool focus) {
    if (_state == State::NONE) return;

    // Only care about the left button being released
    if (!event.buttons.hasLeft()) return;

    // Mouse position is in screen coords; scene y runs bottom-up
    Vec2 pos = event.position;
    pos.y = getSize().height - pos.y;

    if (_buttonRect.contains(pos)) {
        _replayPressed = true;
    }
}
#endif

void EndScene::render() {
    if (_state == State::NONE) return;

    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();

    Size s = getSize();

    // Dimmed full-screen overlay
    _batch->setColor(Color4(0, 0, 0, 180));
    _batch->fill(Rect(0, 0, s.width, s.height));

    // Coloured panel
    bool won = (_state == State::WIN);
    Color4 panelColor = won ? Color4(30, 160, 30, 230) : Color4(180, 30, 30, 230);
    float pw = 480.0f, ph = 240.0f;
    Rect panel(s.width / 2 - pw / 2, s.height / 2 - ph / 2, pw, ph);
    _batch->setColor(panelColor);
    _batch->fill(panel);

    // Replay button background
    _batch->setColor(Color4(60, 60, 60, 255));
    _batch->fill(_buttonRect);

    if (_font) {
        std::string msg = won ? "QUOTA MET!  YOU WIN!" : "YOU DIED!";
        auto label = TextLayout::allocWithText(msg, _font);
        label->layout();
        Affine2 msgTrans;
        msgTrans.translate(s.width / 2 - label->getBounds().size.width / 2,
            s.height / 2 + 40);
        _batch->setColor(Color4::WHITE);
        _batch->drawText(label, msgTrans);

        auto btnLabel = TextLayout::allocWithText("Play Again", _font);
        btnLabel->layout();
        Affine2 btnTrans;
        btnTrans.translate(
            _buttonRect.getMidX() - btnLabel->getBounds().size.width / 2,
            _buttonRect.getMidY() - btnLabel->getBounds().size.height / 2);
        _batch->setColor(Color4::WHITE);
        _batch->drawText(btnLabel, btnTrans);
    }

    _batch->end();
}

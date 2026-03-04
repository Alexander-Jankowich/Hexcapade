//
//  MenuScene.cpp
//  Hexcapade
//
#include "MenuScene.h"
using namespace cugl;
using namespace cugl::graphics;
#define SCENE_HEIGHT 720
bool MenuScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (!assets) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;
    _assets = assets;
    _font = assets->get<Font>("font");
    _choice = Choice::NONE;
    Size s = getSize();
    float bw = 300.0f, bh = 70.0f;
    float cx = s.width / 2.0f;
    float cy = s.height / 2.0f;
    _hostRect = Rect(cx - bw / 2, cy + 20.0f, bw, bh);
    _joinRect = Rect(cx - bw / 2, cy - 70.0f, bw, bh);

#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        touch->addBeginListener(LISTENER_KEY,
            [this](const cugl::TouchEvent& event, bool focus) {
                this->touchBeganCB(event, focus);
            });
    }
#else
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->addPressListener(LISTENER_KEY,
            [this](const MouseEvent& event, Uint8 clicks, bool focus) {
                this->mousePressCB(event, clicks, focus);
            });
    }
#endif
    return true;
}
void MenuScene::dispose() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) touch->removeBeginListener(LISTENER_KEY);
#else
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) mouse->removePressListener(LISTENER_KEY);
#endif
    _assets = nullptr;
    _font = nullptr;
}

#ifdef CU_TOUCH_SCREEN
void MenuScene::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
    Vec2 pos = event.position;
    pos.y = getSize().height - pos.y;
    if (_hostRect.contains(pos)) _choice = Choice::HOST;
    else if (_joinRect.contains(pos)) _choice = Choice::JOIN;
}
#else
void MenuScene::mousePressCB(const MouseEvent& event, Uint8 clicks, bool focus) {
    Vec2 pos = event.position;
    pos.y = getSize().height - pos.y;
    if (_hostRect.contains(pos)) _choice = Choice::HOST;
    else if (_joinRect.contains(pos)) _choice = Choice::JOIN;
}
#endif
void MenuScene::render() {
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();
    Size s = getSize();
    float cx = s.width / 2.0f;
    float cy = s.height / 2.0f;
    // Background
    _batch->setColor(Color4(20, 20, 40, 255));
    _batch->fill(Rect(0, 0, s.width, s.height));
    // Panel
    _batch->setColor(Color4(40, 40, 80, 240));
    _batch->fill(Rect(cx - 260, cy - 160, 520, 320));
    // Host button
    _batch->setColor(Color4(40, 100, 160, 255));
    _batch->fill(_hostRect);
    // Join button
    _batch->setColor(Color4(40, 140, 80, 255));
    _batch->fill(_joinRect);
    if (_font) {
        _batch->setColor(Color4::WHITE);
        auto title = TextLayout::allocWithText("Hexcapade", _font);
        title->layout();
        Affine2 tt;
        tt.translate(cx - title->getBounds().size.width / 2, cy + 120);
        _batch->drawText(title, tt);
        auto hostLabel = TextLayout::allocWithText("Host Game", _font);
        hostLabel->layout();
        Affine2 ht;
        ht.translate(_hostRect.getMidX() - hostLabel->getBounds().size.width / 2,
            _hostRect.getMidY() - hostLabel->getBounds().size.height / 2);
        _batch->drawText(hostLabel, ht);
        auto joinLabel = TextLayout::allocWithText("Join Game", _font);
        joinLabel->layout();
        Affine2 jt;
        jt.translate(_joinRect.getMidX() - joinLabel->getBounds().size.width / 2,
            _joinRect.getMidY() - joinLabel->getBounds().size.height / 2);
        _batch->drawText(joinLabel, jt);
    }
    _batch->end();
}
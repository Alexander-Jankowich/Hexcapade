//
//  HostScene.cpp
//  Hexcapade
//
#include "HostScene.h"

using namespace cugl;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720


// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Init / Dispose
// ──────────────────────────────────────────────────────────────────────────────

bool HostScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (!assets) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;

    _assets = assets;
    _font = assets->get<Font>("font");
    _roomCode = "-----";

    Size s = getSize();
    float bw = 300.0f, bh = 60.0f;
    float cx = s.width / 2.0f;

    _startRect = Rect(cx - bw / 2, s.height / 2.0f - 60.0f, bw, bh);
    _backRect = Rect(cx - bw / 2, s.height / 2.0f - 140.0f, bw, bh);

    // Load network config from the "server" JSON asset
    auto json = _assets->get<JsonValue>("server");
    if (!json) {
        CULog("ERROR: server.json not found in assets!");
        return false;
    }
    _networkCtrl.init(json, true);

    // Register mouse listener
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->addPressListener(LISTENER_KEY,
            [this](const MouseEvent& event, Uint8 clicks, bool focus) {
                this->mousePressCB(event, clicks, focus);
            });
    }

    // Immediately attempt to open a host connection
    _networkCtrl.connect(true);
    return true;
}

void HostScene::dispose() {
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) mouse->removePressListener(LISTENER_KEY);
    _networkCtrl.disconnect();
    _assets = nullptr;
    _font = nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Input
// ──────────────────────────────────────────────────────────────────────────────

void HostScene::mousePressCB(const MouseEvent& event, Uint8 clicks, bool focus) {
    Vec2 pos = event.position;
    pos.y = getSize().height - pos.y;   // flip y to match scene coords

    if (_backRect.contains(pos)) {
        _networkCtrl.disconnect();
        return;
    }

    // Start button is only active when we have a room code (IDLE state)
    if (_networkCtrl.getStatus() == NetworkController::Status::IDLE && _startRect.contains(pos)) {
        _networkCtrl.startGame();
    }
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Update / Render
// ──────────────────────────────────────────────────────────────────────────────

void HostScene::update(float dt) {
    // TODO refactor
//    if (_network) {
//        _network->receive([](const std::string&, const std::vector<std::byte>&) {
//            // Hosts don't need to process incoming data during the lobby
//            });
//        checkConnection();
//    }
}

void HostScene::render() {
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();

    Size s = getSize();

    // Dark background panel
    _batch->setColor(Color4(20, 20, 40, 255));
    _batch->fill(Rect(0, 0, s.width, s.height));

    float cx = s.width / 2.0f;
    float cy = s.height / 2.0f;
    float pw = 520.0f, ph = 340.0f;
    Rect panel(cx - pw / 2, cy - ph / 2, pw, ph);
    _batch->setColor(Color4(40, 40, 80, 240));
    _batch->fill(panel);

    // Back button (always available)
    _batch->setColor(Color4(100, 40, 40, 255));
    _batch->fill(_backRect);

    // Start button — greyed out while waiting for a room code
    bool canStart = (_networkCtrl.getStatus() == NetworkController::Status::IDLE);
    _batch->setColor(canStart ? Color4(40, 140, 40, 255) : Color4(70, 70, 70, 255));
    _batch->fill(_startRect);

    if (_font) {
        _batch->setColor(Color4::WHITE);

        // Title
        auto title = TextLayout::allocWithText("Host Lobby", _font);
        title->layout();
        Affine2 t;
        t.translate(cx - title->getBounds().size.width / 2, cy + 120);
        _batch->drawText(title, t);

        // Room code
        auto codeLabel = TextLayout::allocWithText("Room Code: " + _roomCode, _font);
        codeLabel->layout();
        Affine2 ct;
        ct.translate(cx - codeLabel->getBounds().size.width / 2, cy + 60);
        _batch->drawText(codeLabel, ct);

        // Player count
        auto playerLabel = TextLayout::allocWithText(
            "Players: " + strtool::to_string(_networkCtrl.playerCount()), _font);
        playerLabel->layout();
        Affine2 pt;
        pt.translate(cx - playerLabel->getBounds().size.width / 2, cy + 10);
        _batch->drawText(playerLabel, pt);

        // Back button label
        auto backLabel = TextLayout::allocWithText("Back", _font);
        backLabel->layout();
        Affine2 bt;
        bt.translate(_backRect.getMidX() - backLabel->getBounds().size.width / 2,
            _backRect.getMidY() - backLabel->getBounds().size.height / 2);
        _batch->drawText(backLabel, bt);

        // Start button label
        std::string startText = canStart ? "Start Game" : "Waiting for server...";
        auto startLabel = TextLayout::allocWithText(startText, _font);
        startLabel->layout();
        Affine2 st;
        st.translate(_startRect.getMidX() - startLabel->getBounds().size.width / 2,
            _startRect.getMidY() - startLabel->getBounds().size.height / 2);
        _batch->drawText(startLabel, st);
    }

    _batch->end();
}

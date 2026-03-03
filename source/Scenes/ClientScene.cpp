//
//  ClientScene.cpp
//  Hexcapade
//
#include "ClientScene.h"

using namespace cugl;
using namespace cugl::netcode;
using namespace cugl::graphics;

#define SCENE_HEIGHT 720

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Init / Dispose
// ──────────────────────────────────────────────────────────────────────────────

bool ClientScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (!assets) return false;
    if (!Scene2::initWithHint(Size(0, SCENE_HEIGHT))) return false;

    _assets = assets;
    _font = assets->get<Font>("font");
    _status = Status::IDLE;
    _roomInput = "";

    Size s = getSize();
    float bw = 300.0f, bh = 60.0f;
    float cx = s.width / 2.0f;
    float cy = s.height / 2.0f;

    // Layout: input box in the middle, Join below, Back below that
    _inputRect = Rect(cx - bw / 2, cy + 20.0f, bw, bh);
    _joinRect = Rect(cx - bw / 2, cy - 60.0f, bw, bh);
    _backRect = Rect(cx - bw / 2, cy - 140.0f, bw, bh);

    auto json = _assets->get<JsonValue>("server");
    if (!json) {
        CULog("ERROR: server.json not found in assets!");
        return false;
    }
    _networkCtrl.init(json);

    // Mouse listener for button clicks
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->addPressListener(LISTENER_KEY,
            [this](const MouseEvent& event, Uint8 clicks, bool focus) {
                this->mousePressCB(event, clicks, focus);
            });
    }

    // Keyboard listener so the player can type the room code
    Keyboard* kb = Input::get<Keyboard>();
    if (kb) {
        kb->addKeyDownListener(KB_LISTEN_KEY,
            [this](const KeyEvent& event, bool focus) {
                this->keyPressCB(event, focus);
            });
    }

    return true;
}

void ClientScene::dispose() {
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) mouse->removePressListener(LISTENER_KEY);

    Keyboard* kb = Input::get<Keyboard>();
    if (kb) kb->removeKeyDownListener(KB_LISTEN_KEY);

    _networkCtrl.disconnect();
    _assets = nullptr;
    _font = nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Networking
// ──────────────────────────────────────────────────────────────────────────────

bool ClientScene::checkConnection() {
    auto state = _networkCtrl.checkConnection();
    switch (state) {
    case NetcodeConnection::State::NEGOTIATING:
        _status=Status::JOIN;
            break;
    case NetcodeConnection::State::CONNECTED:
        if (_status != Status::START) {
            _status = Status::WAIT;
        }
        //_playerCount = (int)_network->getNumPlayers();
        break;

    case NetcodeConnection::State::DENIED:
    case NetcodeConnection::State::MISMATCHED:
    case NetcodeConnection::State::INVALID:
    case NetcodeConnection::State::FAILED:
    case NetcodeConnection::State::DISCONNECTED:
        _networkCtrl.disconnect();
        _status = Status::IDLE;
        return false;

    default:
        break;
    }
    return true;
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Input
// ──────────────────────────────────────────────────────────────────────────────

void ClientScene::keyPressCB(const KeyEvent& event, bool focus) {
    // Only accept typing when idle (not yet connected)
    if (_status != Status::IDLE) return;

    KeyCode key = event.keycode;

    if (key == KeyCode::BACKSPACE || key == KeyCode::DEL) {
        if (!_roomInput.empty()) _roomInput.pop_back();
        return;
    }

    // Accept digits 0-9, limit to 5 characters
    if (_roomInput.size() < 5) {
        char c = 0;
        if (key >= KeyCode::NUM_0 && key <= KeyCode::NUM_9) {
            c = '0' + (int)(key)-(int)(KeyCode::NUM_0);
        }
        else if (key >= KeyCode::KEYPAD_0 && key <= KeyCode::KEYPAD_9) {
            c = '0' + (int)(key)-(int)(KeyCode::KEYPAD_0);
        }
        if (c) _roomInput += c;
    }

    // Enter key triggers join if we have 5 digits
    if ((key == KeyCode::RETURN || key == KeyCode::KEYPAD_ENTER)
        && _roomInput.size() == 5) {
        _networkCtrl.connect(false, _roomInput);
        checkConnection();
    }
}

void ClientScene::mousePressCB(const MouseEvent& event, Uint8 clicks, bool focus) {
    Vec2 pos = event.position;
    pos.y = getSize().height - pos.y;

    if (_backRect.contains(pos)) {
        _networkCtrl.disconnect();
        _roomInput = "";
        _status = Status::ABORT;
        return;
    }

    // Join button: only works when IDLE and 5 digits entered
    if (_status == Status::IDLE && _joinRect.contains(pos) && _roomInput.size() == 5) {
        _networkCtrl.connect(false, _roomInput);
        checkConnection();
    }
}

// ──────────────────────────────────────────────────────────────────────────────
#pragma mark Update / Render
// ──────────────────────────────────────────────────────────────────────────────

void ClientScene::update(float dt) {
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

void ClientScene::render() {
    _batch->setPerspective(getCamera()->getCombined());
    _batch->begin();

    Size s = getSize();
    float cx = s.width / 2.0f;
    float cy = s.height / 2.0f;

    // Background
    _batch->setColor(Color4(20, 20, 40, 255));
    _batch->fill(Rect(0, 0, s.width, s.height));

    // Panel
    float pw = 520.0f, ph = 340.0f;
    _batch->setColor(Color4(40, 40, 80, 240));
    _batch->fill(Rect(cx - pw / 2, cy - ph / 2, pw, ph));

    // Room code input box — highlighted when focused
    _batch->setColor(Color4(60, 60, 100, 255));
    _batch->fill(_inputRect);

    // Back button
    _batch->setColor(Color4(100, 40, 40, 255));
    _batch->fill(_backRect);

    // Join / status button
    bool canJoin = (_status == Status::IDLE && _roomInput.size() == 5);
    Color4 joinColor;
    if (_status == Status::IDLE)  joinColor = canJoin ? Color4(40, 140, 40, 255) : Color4(70, 70, 70, 255);
    else                          joinColor = Color4(70, 70, 70, 255);
    _batch->setColor(joinColor);
    _batch->fill(_joinRect);

    if (_font) {
        _batch->setColor(Color4::WHITE);

        // Title
        auto title = TextLayout::allocWithText("Join Game", _font);
        title->layout();
        Affine2 tt;
        tt.translate(cx - title->getBounds().size.width / 2, cy + 120);
        _batch->drawText(title, tt);

        // Instruction
        auto instr = TextLayout::allocWithText("Enter 5-digit room code:", _font);
        instr->layout();
        Affine2 it;
        it.translate(cx - instr->getBounds().size.width / 2, cy + 95);
        _batch->drawText(instr, it);

        // Room code input text (show cursor placeholder when empty)
        std::string displayCode = _roomInput.empty() ? "_____" : _roomInput;
        // Pad with underscores to show remaining digits
        while (displayCode.size() < 5 && _status == Status::IDLE) displayCode += '_';
        auto codeText = TextLayout::allocWithText(displayCode, _font);
        codeText->layout();
        Affine2 ct;
        ct.translate(_inputRect.getMidX() - codeText->getBounds().size.width / 2,
            _inputRect.getMidY() - codeText->getBounds().size.height / 2);
        _batch->drawText(codeText, ct);

        // Player count (visible once connected)
        if (_status == Status::WAIT || _status == Status::START) {
            auto playerLabel = TextLayout::allocWithText(
                "Players: " + strtool::to_string(_networkCtrl.playerCount()), _font);
            playerLabel->layout();
            Affine2 pl;
            pl.translate(cx - playerLabel->getBounds().size.width / 2, cy - 20);
            _batch->drawText(playerLabel, pl);
        }

        // Back button label
        auto backLabel = TextLayout::allocWithText("Back", _font);
        backLabel->layout();
        Affine2 bl;
        bl.translate(_backRect.getMidX() - backLabel->getBounds().size.width / 2,
            _backRect.getMidY() - backLabel->getBounds().size.height / 2);
        _batch->drawText(backLabel, bl);

        // Join button label
        std::string joinText;
        if (_status == Status::IDLE) joinText = "Join";
        else if (_status == Status::JOIN) joinText = "Connecting...";
        else                              joinText = "Waiting for host...";
        auto joinLabel = TextLayout::allocWithText(joinText, _font);
        joinLabel->layout();
        Affine2 jl;
        jl.translate(_joinRect.getMidX() - joinLabel->getBounds().size.width / 2,
            _joinRect.getMidY() - joinLabel->getBounds().size.height / 2);
        _batch->drawText(joinLabel, jl);
    }

    _batch->end();
}   

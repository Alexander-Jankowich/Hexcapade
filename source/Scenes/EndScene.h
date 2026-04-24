//
//  EndScene.h
//  Hexcapade
//
#ifndef __END_SCENE_H__
#define __END_SCENE_H__
#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class EndScene : public cugl::scene2::Scene2 {
public:
    enum class State { NONE, WIN, LOSE };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;
    std::shared_ptr<NetworkController> _networkCtrl;
    State _state;
    bool  _replayPressed;
    /** Screen-space rect for the replay button */
    cugl::Rect _buttonRect;

#ifdef CU_TOUCH_SCREEN
    static constexpr Uint32 LISTENER_KEY = 2;
    void touchEndCB(const cugl::TouchEvent& event, bool focus);
#else
    static constexpr Uint32 LISTENER_KEY = 2;
    void mouseReleaseCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
#endif

public:
    EndScene() : cugl::scene2::Scene2(), _state(State::NONE), _replayPressed(false) {}
    ~EndScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    void setNetwork(std::shared_ptr<NetworkController> n) {_networkCtrl=n;}
    void show(bool won);
    

    bool isVisible()      const { return _state != State::NONE; }
    bool isHost() const {return _networkCtrl->isHost();}
    std::shared_ptr<NetworkController> getNetwork() const {return _networkCtrl;}
    void setActive(bool active) override;

    void release() {_networkCtrl = nullptr;}
    bool didPressReplay() const { return (_replayPressed && _networkCtrl->isHost()) || (_networkCtrl->gameState()==NetworkController::NCGameState::UNSTARTED); }

    void update(float dt) override;
    void render() override;
};

#endif /* __END_SCENE_H__ */

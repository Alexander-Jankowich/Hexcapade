//
//  ClientScene.h
//  Hexcapade
//
#ifndef __CLIENT_SCENE_H__
#define __CLIENT_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class ClientScene : public cugl::scene2::Scene2 {
private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;
    
    NetworkController _networkCtrl;

    std::string _roomInput;

    cugl::Rect _joinRect;
    cugl::Rect _backRect;
    cugl::Rect _inputRect;

    static constexpr Uint32 LISTENER_KEY = 4;
    static constexpr Uint32 KB_LISTEN_KEY = 5;

#ifdef CU_TOUCH_SCREEN
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
    void textInputCB(const cugl::TextInputEvent& event, bool focus);
#else
    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
    void keyPressCB(const cugl::KeyEvent& event, bool focus);
#endif

public:
    ClientScene() : cugl::scene2::Scene2() {}
    ~ClientScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt) override;
    void render() override;
    void disconnect() {_networkCtrl.disconnect();}
    NetworkController::Status getStatus(){return _networkCtrl.getStatus();}
};

#endif /* __CLIENT_SCENE_H__ */

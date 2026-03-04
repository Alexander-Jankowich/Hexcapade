//
//  HostScene.h
//  Hexcapade
//
#ifndef __HOST_SCENE_H__
#define __HOST_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class HostScene : public cugl::scene2::Scene2 {

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    NetworkController _networkCtrl;

    cugl::Rect _startRect;
    cugl::Rect _backRect;

    std::string _roomCode;

    static constexpr Uint32 LISTENER_KEY = 3;

#ifdef CU_TOUCH_SCREEN
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
#else
    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
#endif

    bool checkConnection();

public:
    HostScene() : cugl::scene2::Scene2() {}
    ~HostScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt);
    void render() override;
    
    void disconnect() {_networkCtrl.disconnect();}
    NetworkController::Status getStatus(){return _networkCtrl.getStatus();}

};

#endif /* __HOST_SCENE_H__ */

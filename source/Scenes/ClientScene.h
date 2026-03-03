//
//  ClientScene.h
//  Hexcapade
//
#ifndef __CLIENT_SCENE_H__
#define __CLIENT_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class ClientScene : public cugl::scene2::Scene2 {
public:
    enum class Status { IDLE, JOIN, WAIT, START, ABORT };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    Status _status;
    
    NetworkController _networkCtrl;

    std::string _roomInput;

    cugl::Rect _joinRect;
    cugl::Rect _backRect;
    cugl::Rect _inputRect;

    static constexpr Uint32 LISTENER_KEY = 4;
    static constexpr Uint32 KB_LISTEN_KEY = 5;

    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
    void keyPressCB(const cugl::KeyEvent& event, bool focus);

    bool checkConnection();

public:
    ClientScene() : cugl::scene2::Scene2(), _status(Status::IDLE), _playerCount(1) {}
    ~ClientScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt);
    void render() override;

    Status getStatus() const { return _status; }

};

#endif /* __CLIENT_SCENE_H__ */

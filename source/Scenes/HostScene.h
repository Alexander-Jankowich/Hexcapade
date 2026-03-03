//
//  HostScene.h
//  Hexcapade
//
#ifndef __HOST_SCENE_H__
#define __HOST_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class HostScene : public cugl::scene2::Scene2 {
public:
    enum class Status { WAIT, IDLE, START, ABORT };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    Status _status;
    NetworkController _networkCtrl;

    cugl::Rect _startRect;
    cugl::Rect _backRect;

    std::string _roomCode;

    static constexpr Uint32 LISTENER_KEY = 3;

    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

    bool checkConnection();
    void startGame();

public:
    HostScene() : cugl::scene2::Scene2(), _status(Status::WAIT), _playerCount(1) {}
    ~HostScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt);
    void render() override;

    Status getStatus() const { return _status; }

};

#endif /* __HOST_SCENE_H__ */

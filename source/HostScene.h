//
//  HostScene.h
//  Hexcapade
//
#ifndef __HOST_SCENE_H__
#define __HOST_SCENE_H__

#include <cugl/cugl.h>

class HostScene : public cugl::scene2::Scene2 {
public:
    enum class Status { WAIT, IDLE, START, ABORT };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    Status _status;
    std::shared_ptr<cugl::netcode::NetcodeConnection> _network;
    cugl::netcode::NetcodeConfig _config;

    cugl::Rect _startRect;
    cugl::Rect _backRect;

    std::string _roomCode;
    int _playerCount;

    static constexpr Uint32 LISTENER_KEY = 3;

    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

    bool connect();
    bool checkConnection();
    void startGame();
    static std::string hex2dec(const std::string& hex);

public:
    HostScene() : cugl::scene2::Scene2(), _status(Status::WAIT), _playerCount(1) {}
    ~HostScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt);
    void render() override;

    Status getStatus() const { return _status; }

    std::shared_ptr<cugl::netcode::NetcodeConnection> getConnection() { return _network; }
    void disconnect() {
        if (_network) { _network->close(); }
        _network = nullptr;
    }
};

#endif /* __HOST_SCENE_H__ */
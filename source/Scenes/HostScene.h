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

    std::shared_ptr<NetworkController> _networkCtrl;

    /** The menu button for starting a game */
    std::shared_ptr<cugl::scene2::Button> _startgame;
    /** The back button for the menu scene */
    std::shared_ptr<cugl::scene2::Button> _backout;
    /** The game id label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _gameid;
    /** The players label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _player;
    std::string _roomCode;
    bool _backPressed = false;
    
public:
    HostScene() : cugl::scene2::Scene2() {}
    ~HostScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<NetworkController>& network);
    void dispose() override;
    virtual void setActive(bool value) override;


    void update(float dt) override;
    void disconnect() {_networkCtrl->disconnect();}
    //void release() { _networkCtrl = nullptr; }
    //void obtainNetwork(std::shared_ptr<NetworkController> n) { _networkCtrl = n; }

    NetworkController::Status getStatus(){return _networkCtrl->getStatus();}
    bool didBackOut() const { return _backPressed; }
    std::shared_ptr<NetworkController> getConnection(){return _networkCtrl;}
    std::shared_ptr<NetworkController> getNetwork() {
        return _networkCtrl;
    }

    void resetStatus() { _networkCtrl->resetStatus(); }

    void reset() override {
        _networkCtrl->disconnect();
        _backPressed = false;
        _gameid->setText("");
        _player->setText("1");
    }
    
private:
    void updateText(const std::shared_ptr<cugl::scene2::Button>& button, const std::string text);
    void configureStartButton();


};

#endif /* __HOST_SCENE_H__ */

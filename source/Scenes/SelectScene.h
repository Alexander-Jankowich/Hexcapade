//
//  HostSelectScene.h
//  alpha_final
//
//  Created by Serena Duncan on 4/20/26.
//

#ifndef __SELECT_SCENE_H__
#define __SELECT_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class SelectScene : public cugl::scene2::Scene2 {
    public:
    enum class SSStatus {SELECT, HANDSHAKE, START};
private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    std::shared_ptr<NetworkController> _networkCtrl;

    /** The menu button for starting a game */
    std::shared_ptr<cugl::scene2::Button> _startgame;
    
    bool _startPressed = false;

    SSStatus _status;
    bool sentInit = false;

    bool checkConnection();

public:
    SelectScene() : cugl::scene2::Scene2() {}
    ~SelectScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    virtual void setActive(bool value) override;


    void update(float dt) override;
    void disconnect() {_networkCtrl->disconnect();}
    void release() { _networkCtrl = nullptr; }
    SSStatus getStatus() {return _status;}
    bool canInitMap() {return _networkCtrl->getRandSeed() !=0;}
    bool isHost() {return _networkCtrl->isHost();}
    void obtainNetwork(std::shared_ptr<NetworkController> n) { _networkCtrl = n; }

    //NetworkController::Status getStatus(){return _networkCtrl->getStatus();}
    std::shared_ptr<NetworkController> getConnection(){return _networkCtrl;}
    std::shared_ptr<NetworkController> getNetwork() {
        return _networkCtrl;
    }

    void resetStatus() { _networkCtrl->resetStatus(); }

    void reset() {
        _networkCtrl->disconnect();
    }
    
private:
    void updateText(const std::shared_ptr<cugl::scene2::Button>& button, const std::string text);
    void configureStartButton();


};

#endif // !__SELECT_SCENE_H__

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

    /** The menu button for starting a game */
    std::shared_ptr<cugl::scene2::Button> _startgame;
    /** The back button for the menu scene */
    std::shared_ptr<cugl::scene2::Button> _backout;
    /** The game id label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _gameid;
    /** The players label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _player;
    std::string _roomCode;

    bool checkConnection();

public:
    HostScene() : cugl::scene2::Scene2() {}
    ~HostScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    virtual void setActive(bool value) override;


    void update(float dt);
    
    void disconnect() {_networkCtrl.disconnect();}
    NetworkController::Status getStatus(){return _networkCtrl.getStatus();}
    
private:
    void updateText(const std::shared_ptr<cugl::scene2::Button>& button, const std::string text);
    void configureStartButton();


};

#endif /* __HOST_SCENE_H__ */

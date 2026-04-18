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
    
    std::shared_ptr<NetworkController> _networkCtrl;

    std::string _roomInput;

    /** The menu button for starting a game */
    std::shared_ptr<cugl::scene2::Button> _startgame;
    /** The back button for the menu scene */
    std::shared_ptr<cugl::scene2::Button> _backout;
    /** The game id label (for updating) */
    std::shared_ptr<cugl::scene2::TextField> _gameid;
    /** The players label (for updating) */
    std::shared_ptr<cugl::scene2::Label> _player;
    

public:
    bool canInitGame;

    ClientScene() : cugl::scene2::Scene2() {}
    ~ClientScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    

    void update(float dt) override;
    void disconnect() { if (_networkCtrl) _networkCtrl->disconnect();}
    void release() { _networkCtrl = nullptr; }
    void obtainNetwork(std::shared_ptr<NetworkController> n) { _networkCtrl = n; }

    NetworkController::Status getStatus() { return _networkCtrl->getStatus(); }
    std::shared_ptr<NetworkController> getConnection(){return _networkCtrl;}
    std::shared_ptr<NetworkController> getNetwork() {
        return _networkCtrl;
    }

    void resetStatus() { _networkCtrl->resetStatus(); }
    
    void processInitEvent(const std::shared_ptr<GameInitEvent>& e);

    void reset() {
        _networkCtrl->disconnect();
        _gameid->setText("");
        _player->setText("1");
    }

    
    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
private:
    /**
     * Updates the text in the given button.
     *
     * Techincally a button does not contain text. A button is simply a scene graph
     * node with one child for the up state and another for the down state. So to
     * change the text in one of our buttons, we have to descend the scene graph.
     * This method simplifies this process for you.
     *
     * @param button    The button to modify
     * @param text      The new text value
     */
    void updateText(const std::shared_ptr<cugl::scene2::Button>& button, const std::string text);
    
    /**
     * Reconfigures the start button for this scene
     *
     * This is necessary because what the buttons do depends on the state of the
     * networking.
     */
    void configureStartButton();
};

#endif /* __CLIENT_SCENE_H__ */

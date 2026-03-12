//
//  MenuScene.h
//  Hexcapade
//
#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include <cugl/cugl.h>

class MenuScene : public cugl::scene2::Scene2 {
public:
    enum class Choice { NONE, HOST, JOIN };

private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::graphics::Font> _font;

    Choice _choice;

    /** The menu button for hosting a game */
    std::shared_ptr<cugl::scene2::Button> _hostbutton;
    /** The menu button for joining a game */
    std::shared_ptr<cugl::scene2::Button> _joinbutton;
    


public:
    MenuScene() : cugl::scene2::Scene2(), _choice(Choice::NONE) {}
    ~MenuScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    virtual void setActive(bool value) override;


    void update(float dt) {}

    Choice getChoice() const { return _choice; }
    void resetChoice() { _choice = Choice::NONE; }
};

#endif /* __MENU_SCENE_H__ */

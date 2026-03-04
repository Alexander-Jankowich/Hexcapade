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

    cugl::Rect _hostRect;
    cugl::Rect _joinRect;

    static constexpr Uint32 LISTENER_KEY = 6;

#ifdef CU_TOUCH_SCREEN
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);
#else
    void mousePressCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
#endif

public:
    MenuScene() : cugl::scene2::Scene2(), _choice(Choice::NONE) {}
    ~MenuScene() { dispose(); }

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;

    void update(float dt) {}
    void render() override;

    Choice getChoice() const { return _choice; }
    void resetChoice() { _choice = Choice::NONE; }
};

#endif /* __MENU_SCENE_H__ */
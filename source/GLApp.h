//
//  GLApp.h
//  Hexcapade
//
#ifndef __GL_APP_H__
#define __GL_APP_H__

#include <cugl/cugl.h>
#include "Scenes/GLGameScene.h"
#include "Scenes/EndScene.h"
#include "Scenes/MenuScene.h"
#include "Scenes/HostScene.h"
#include "Scenes/ClientScene.h"
#include "Controllers/NetworkController.h"

/**
 * This class represents the application root for Hexcapade.
 */
class GeometryApp : public cugl::Application {
public:
    /** Top-level application states */
    enum class State { LOAD, MENU, HOST, CLIENT, GAME };

protected:
    /** The global sprite batch for drawing (only want one of these) */
    std::shared_ptr<cugl::graphics::SpriteBatch> _batch;
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;

    /** Whether or not we have finished loading all assets */
    bool _loaded;
    /** The current application state */
    State _scene;

    // Scenes
    cugl::scene2::LoadingScene _loading;
    MenuScene   _mainmenu;
    HostScene   _hostScene;
    ClientScene _clientScene;
    GameScene   _gameplay;
    EndScene    _endScene;

public:
    GeometryApp();
    ~GeometryApp() {}

#pragma mark Application State
    virtual void onStartup()  override;
    virtual void onShutdown() override;
    virtual void onSuspend()  override;
    virtual void onResume()   override;

#pragma mark Application Loop
    virtual void update(float dt)         override;
    virtual void preUpdate(float dt)      override;
    virtual void fixedUpdate(Uint64 step) override;
    virtual void postUpdate(float dt)     override;
    virtual void draw()                   override;

private:
    void updateLoadingScene(float dt);
    void updateMenuScene(float dt);
    void updateHostScene(float dt);
    void updateClientScene(float dt);
    void updateGameScene(float dt);
};

#endif /* __GL_APP_H__ */

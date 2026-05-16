//
//  App.h
//  Hexcapade
//
#ifndef __APP_H__
#define __APP_H__

#include <string>
#include <cugl/cugl.h>
#include "Scenes/GameScene.h"
#include "Scenes/EndScene.h"
#include "Scenes/MenuScene.h"
#include "Scenes/HostScene.h"
#include "Scenes/SelectScene.h"
#include "Scenes/ClientScene.h"
#include "Scenes/TutorialScene.h"
#include "Controllers/NetworkController.h"
#include "Models/Map/LevelLoader.h"
#include "Models/Map/StaticLevelLoader.h"

/**
 * This class represents the application root for Hexcapade.
 */
class GeometryApp : public cugl::Application {
public:
    /** Top-level application states */
    enum class State { LOAD, MENU, TUTORIAL, HOST, CLIENT, SELECT, GAME, END };

protected:
    /** The global sprite batch for drawing (only want one of these) */
    std::shared_ptr<cugl::graphics::SpriteBatch> _batch;
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The network for this run of the app */
    std::shared_ptr<NetworkController> _network;
    
    std::shared_ptr<std::vector<std::string>> _rooms;

    /** Whether or not we have finished loading all assets */
    bool _loaded;
    /** Whether menu music has started on the loading play screen */
    bool _menuMusicStarted;
    /** The current looping music asset key */
    std::string _currentMusicKey;
    /** the volume used by the current looping music */
    float _currentMusicVolume = 1.0f;
    /** whether music should pause when the app is suspended */
    bool _pauseMusicOnSuspend = false;
    /** whether short sound effects should be skipped during app interruption */
    bool _audioInterrupted = false;
    /** The current application state */
    State _scene;

    // Scenes
    cugl::scene2::LoadingScene _loading;
    MenuScene   _mainmenu;
    HostScene   _hostScene;
    ClientScene _clientScene;
    SelectScene _selectScene;
    TutorialScene _tutorialScene;
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
    void updateSelectScene(float dt);
    void updateClientScene(float dt);
    void updateGameScene(float dt);
    void updateEndScene(float dt);
    void updateTutorialScene(float dt);
    /** clears a stuck focus-change audio flag and restores music if the queue was dropped */
    void restoreAudioAfterFocusChange();
    /** plays a short app-level sfx with either a stable or custom playback key */
    bool playSoundWithKey(const std::string& playbackKey, const std::string& assetKey,
                          float volumeMultiplier, bool force);
    /** restarts click/result sounds so repeated taps do not get ignored */
    bool restartSound(const std::string& key, float volumeMultiplier);
    /** keeps music on the cugl music queue and changes volume without restarting the same track */
    void playMusic(const std::string& key, float volumeMultiplier = 1.0f,
                   float fade = 0.35f, bool restartIfSame = false);

};

#endif /* __APP_H__ */

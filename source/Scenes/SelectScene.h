#ifndef __SELECT_SCENE_H__
#define __SELECT_SCENE_H__

#include <cugl/cugl.h>
#include "../Controllers/NetworkController.h"

class SelectScene : public cugl::scene2::Scene2 {
public:
    enum class SSStatus { SELECT, HANDSHAKE, START };
    enum class CharacterType { FOX, CAT, DUCK, BUNNY };
    
    static std::string portraitKey(CharacterType c) {
        switch (c) {
            case CharacterType::CAT:   return "cat_pr";
            case CharacterType::DUCK:  return "duck_pr";
            case CharacterType::BUNNY: return "bunny_pr";
            case CharacterType::FOX:
            default:                   return "fox_pr";
        }
    }
    
private:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<NetworkController>  _networkCtrl;
    
    std::shared_ptr<cugl::scene2::Button>      _startgame;
    std::shared_ptr<cugl::scene2::PolygonNode> _portrait;
    std::shared_ptr<cugl::scene2::Button>      _catBtn;
    std::shared_ptr<cugl::scene2::Button>      _foxBtn;
    std::shared_ptr<cugl::scene2::Button>      _duckBtn;
    std::shared_ptr<cugl::scene2::Button>      _bunnyBtn;
    
    std::shared_ptr<cugl::scene2::TextField> _nameField;
    std::string _playerName = "PLAYER";
    
    CharacterType _selectedCharacter = CharacterType::FOX;
    bool _characterChosen = false;
    bool _startPressed = false;
    bool _readySent = false;
    SSStatus _status = SSStatus::SELECT;
    
    void onCharacterSelected(CharacterType c);
    void configureStartButton();
    
public:
    SelectScene() : cugl::scene2::Scene2() {}
    ~SelectScene() { dispose(); }
    
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    void dispose() override;
    void update(float dt) override;
    void setActive(bool value) override;
    
    void obtainNetwork(std::shared_ptr<NetworkController> n) { _networkCtrl = n; }
    void release() { _networkCtrl = nullptr; }
    SSStatus getStatus() { return _status; }
    bool canInitMap() { return _networkCtrl->getRandSeed() != 0; }
    bool isHost() { return _networkCtrl && _networkCtrl->isHost(); }
    void resetStatus() { if (_networkCtrl) _networkCtrl->resetStatus(); }
    void reset() override { if (_networkCtrl) _networkCtrl->disconnect(); }
    
    std::shared_ptr<NetworkController> getNetwork() { return _networkCtrl; }
    CharacterType getSelectedCharacter() const { return _selectedCharacter; }
    const std::string& getPlayerName() const { return _playerName; }
private:
    void updateText(const std::shared_ptr<scene2::Button>& button, const std::string text);
};
#endif // !__SELECT_SCENE_H__

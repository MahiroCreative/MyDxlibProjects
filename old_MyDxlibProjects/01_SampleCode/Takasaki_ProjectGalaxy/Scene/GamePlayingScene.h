#pragma once
#include "Scene.h"
#include <string>
#include <vector>
#include <map>

class GameManager;

class GamePlayingScene : public Scene
{
public:
    GamePlayingScene(SceneManager& manager);
    ~GamePlayingScene();

    virtual void Load();

    virtual void Update();
    virtual void Draw();

    void getSlectNumber(int a) { m_selectNum = a; }

private:

    int m_btnFrame = 0;
    int m_frame = 0;

    float m_fps = 0.0f;

    int m_selectNum = 0;
    
    bool m_isTitle = false;
    bool m_isContinue = false;
    bool m_isGameOver = false;

    bool m_isClear = false;

    std::shared_ptr<GameManager> m_gameManager;

    // メンバ関数ポインタの宣言
    using UpdateFunc_t = void (GamePlayingScene::*)();
    using DrawFunc_t = void (GamePlayingScene::*)();
    UpdateFunc_t m_updateFunc = nullptr;
    DrawFunc_t m_drawFunc = nullptr;

    // 更新関数
    void FadeInUpdate();
    void NormalUpdate();
    void FadeOutUpdate();

    void ChangeScene(std::shared_ptr<Scene> nextScene);
    void PushScene(std::shared_ptr<Scene> nextScene);
    // 描画関数
    void FadeDraw();
    void NormalDraw();


};




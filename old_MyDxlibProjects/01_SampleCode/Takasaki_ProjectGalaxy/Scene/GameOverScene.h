#pragma once
#include "Scene.h"
#include <vector>

class GamePlayingScene;
class Player;

class GameOverScene : public Scene
{
public:
    GameOverScene(SceneManager& mgr);

    ~GameOverScene();

    void Load();
    void Update();
    void Draw();

    void SetMaterialXNum(int num) { m_materialXNum = num; }
private:
    float m_angle;//sinカーブのためだけ

    int m_materialXNum = 0;
    int m_btnFrame = 0;
    int m_frame;
    int m_fadeSoundFrame = 0;
    int m_fadeSpeed;
    int m_graphHandle = 0;
    int m_selectNumber = 0;
    int m_fontHandle;

    int m_modelHandle = -1;
    int m_frameHandle;

    // メンバ関数ポインタの宣言
    using UpdateFunc_t = void (GameOverScene::*)();
    using DrawFunc_t = void (GameOverScene::*)();
    UpdateFunc_t m_updateFunc;
    DrawFunc_t m_drawFunc;

    // 更新状態を表す関数
    void FadeInUpdate();
    void NormalUpdate();
    void FadeOutUpdate();

    void ChangeScene(std::shared_ptr<Scene> nextScene);

    // 描画状態を表す関数
    void FadeDraw();
    void FadeOutDraw();
    void NormalDraw();
};


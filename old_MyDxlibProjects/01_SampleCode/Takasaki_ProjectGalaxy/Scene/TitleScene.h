#pragma once
#include "Scene.h"
#include <string>
#include <vector>
#include <map>

class SceneMain;


class TitleScene : public Scene
{
public:
    TitleScene(SceneManager& manager);
    ~TitleScene();
    void Load();
    void Update();
    void Draw();

    void getSlectNumber(int a) { m_selectNum = a; }

private:

    int m_btnFrame = 0;
    int m_fadeSpeed = 0;
    int m_frame = 0;

    float m_fps = 0.0f;

    int m_selectNum = 0;

    bool m_isGamePlaying = false;

    // メンバ関数ポインタの宣言
    using UpdateFunc_t = void (TitleScene::*)();
    using DrawFunc_t = void (TitleScene::*)();
    UpdateFunc_t m_updateFunc = nullptr;
    DrawFunc_t m_drawFunc = nullptr;



    // 更新関数
    void FadeInUpdate();
    void NormalUpdate();
    void FadeOutUpdate();

    void ChangeScene(std::shared_ptr<Scene> next);


    // 描画関数
    void FadeDraw();
    void NormalDraw();

};




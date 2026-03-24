#include "Application.h"
#include <DxLib.h>
#include <chrono>
#include <ctime>
#include "NumUtility.h"
#include "EffekseerManager.h"
#include "SoundManager.h"
#include "FileManager.h"
#include "FontManager.h"
#include "StageDataManager.h"
#include "SaveDataManager.h"
#include "RankingDataManager.h"
#include "SceneManager.h"
#include "Game.h"
#include "Input.h"
#include "MyEngine/MyEngine.h"
#include "SceneBase.h"

namespace
{
    const wchar_t* const kWindowText = L"THE GATE:";
#ifdef _DEBUG
    const std::unordered_map<SceneKind, const char* const> SCENE_KIND =
    {
        { SceneKind::TITLE,         "タイトル" },
        { SceneKind::STAGE_SELECT,  "ステージ選択" },
        { SceneKind::MAIN,          "メイン" },
        { SceneKind::CLEAR,         "クリア" },
        { SceneKind::OPTION,        "オプション" },
        { SceneKind::DEBUG,         "デバッグ" },
    };
#endif
}

Application::Application() :
    m_isEnd(false),
#ifdef _DEBUG
    m_isWindows(true),
    m_in(nullptr),
    m_out(nullptr),
#else
    m_isWindows(false),
#endif
    m_screenLight(255)
{
}

Application& Application::GetInstance()
{
    static Application app;
    return app;
}

bool Application::Init()
{
#ifdef _DEBUG
    // コンソール画面生成
    AllocConsole();
    // stdout
    freopen_s(&m_out, "CON", "w", stdout);
    // stdin
    freopen_s(&m_in, "CON", "r", stdin);
    // デバッグコンソールがアクティブウィンドウになるのでゲーム本体のウィンドウをアクティブにする
    SetForegroundWindow(GetMainWindowHandle());
#endif
    ChangeWindowMode(m_isWindows);
    // スクリーン変更しても画像をDeleteしないように
    SetChangeScreenModeGraphicsSystemResetFlag(false);

    // 画面サイズ変更
    SetGraphMode(Game::WINDOW_W, Game::WINDOW_H, 32);
    SetWindowText(kWindowText);

    if (DxLib_Init() == -1)
    {
        return false;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    // ZBufferの使用
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(true);

    auto& effMgr = EffekseerManager::GetInstance();
    effMgr.Init(8000);

    std::random_device rd;
    m_mt = std::mt19937(rd());

    return true;
}

void Application::Run()
{
    auto& scnMgr = SceneManager::GetInstance();
    auto& sndMgr = SoundManager::GetInstance();
    auto& effMgr = EffekseerManager::GetInstance();
    auto& physics = MyEngine::Physics::GetInstance();
    auto& input = Input::GetInstance();
    InitManager();
    NumUtility::GetInstance().Init();
#ifdef _DEBUG
    auto& debug = MyEngine::DebugDraw::GetInstance();
    LONGLONG updateTime;
    LONGLONG drawTime;
    bool isUpdate = true;
    bool isStep = false;
    int frame = 0;
#endif

    LONGLONG time;
    while (ProcessMessage() != -1) 
    {
        time = GetNowHiPerformanceCount();

        ClearDrawScreen();

        input.Update();

#ifdef _DEBUG
//        debug.Gulid();
        
        if (isStep)
        {
            if (input.IsTriggerd(KEY_INPUT_P))
            {
                isUpdate = true;
            }
            if (input.IsTriggerd(KEY_INPUT_RETURN))
            {
                isStep = false;
            }
        }
        else
        {
            isUpdate = true;
            if (input.IsTriggerd(KEY_INPUT_P))
            {
                isStep = true;
            }
        }

        debug.SetStop(!isUpdate);

        if (isUpdate)
        {
            ++frame;
            printf("\n-------------------------------\n");
            ShowTime();
            printf("シーン:%s\n", SCENE_KIND.at(scnMgr.GetNowSceneKind()));
            printf("経過フレーム:%d\n", frame);
            debug.Clear();
            updateTime = GetNowHiPerformanceCount();
#endif
            scnMgr.Update();
            physics.Update();
            sndMgr.Update();
            effMgr.Update();
#ifdef _DEBUG
            drawTime = GetNowHiPerformanceCount();
            updateTime = drawTime - updateTime;
        }

#endif

        scnMgr.Draw();

#ifdef _DEBUG
#if true
        debug.Draw();
        if (isStep)
        {
            DrawString(16, 636, L"動作：ステップ中", 0x00ff00);
        }
        else
        {
            DrawString(16, 636, L"動作：通常中", 0x00ff00);
        }
        if (isUpdate)
        {
            drawTime = GetNowHiPerformanceCount() - drawTime;
            isUpdate = false;
        }
        
        DrawFormatString(16, 652, 0x00ff00, L"FPS : %.2f", GetFPS());
        // 更新時間
        int uW = static_cast<int>(Game::WINDOW_W * (updateTime / 16666.6f));
        DrawBox(72, 668, 72 + uW, 684, 0xff0000, true);
        DrawFormatString(16, 668, 0x00ff00, L"更新 : %d", updateTime);
        // 描画時間
        int dW = static_cast<int>(Game::WINDOW_W * (drawTime / 16666.6f));
        DrawBox(72, 684, 72 + dW, 700, 0x0000ff, true);
        DrawFormatString(16, 684, 0x00ff00, L"描画 : %d", drawTime);
        // 合計時間
        DrawBox(72, 700, 72 + uW, 716, 0xff0000, true);
        DrawBox(72 + uW, 700, 72 + uW + dW, 716, 0x0000ff, true);
        DrawFormatString(16, 700, 0x00ff00, L"合計 : %d", drawTime + updateTime);
#endif
#endif
        ScreenFlip();

        if (m_isEnd) break;
        while (Game::FPS_FRAME > GetNowHiPerformanceCount() - time);
    }

    
}

void Application::Terminate()
{
    EndManager();
    DxLib_End();
#ifdef _DEBUG
    // コンソール画面終了処理
    fclose(m_out); 
    fclose(m_in); 
    FreeConsole();
#endif
}

int Application::GetRand()
{
    return m_mt();
}

int Application::GetRand(int min, int max)
{
    std::uniform_int_distribution<int> uid(min, max);
    return uid(m_mt);
}

float Application::GetRand(float min, float max)
{
    std::uniform_real_distribution<float> urd(min, max);
    return urd(m_mt);
}

bool Application::AddScreenLight(int val)
{
    bool isChange = false;

    int temp = m_screenLight + val;
    if (temp > 255)
    {
        m_screenLight = 255;
        isChange = false;
    }
    else
    {
        m_screenLight = temp;
        isChange = true;
    }
    
    return isChange;
}

bool Application::SubScreenLight(int val)
{
    bool isChange = false;

    int temp = m_screenLight - val;
    if (temp < 0)
    {
        m_screenLight = 0;
        isChange = false;
    }
    else
    {
        m_screenLight = temp;
        isChange = true;
    }

    return isChange;
}

void Application::InitManager()
{
    auto& fileMgr = FileManager::GetInstance();
    auto& scnMgr = SceneManager::GetInstance();
    auto& fontMgr = FontManager::GetInstance();
    auto& sndMgr = SoundManager::GetInstance();
    auto& stageDataMgr = StageDataManager::GetInstance();
    auto& saveDataMgr = SaveDataManager::GetInstance();
    auto& rankMgr = RankingDataManager::GetInstance();

    fileMgr.Init();
    scnMgr.Init();
    fontMgr.Init();
    stageDataMgr.Load();
    saveDataMgr.Load();
    rankMgr.Load();
}

void Application::EndManager()
{
    auto& scnMgr = SceneManager::GetInstance();
    auto& saveDataMgr = SaveDataManager::GetInstance();
    auto& rankMgr = RankingDataManager::GetInstance();
    auto& effMgr = EffekseerManager::GetInstance();
    auto& fileMgr = FileManager::GetInstance();

    scnMgr.End();
    saveDataMgr.Save();
    rankMgr.Save();
    effMgr.End();
    fileMgr.End();
}

void Application::ShowTime() const
{
    // 現在時刻をsystem_clockを用いて取得
    auto now = std::chrono::system_clock::now();
    // 現在時刻をtime_t形式で出力
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    // 時間
    printf("%d時", static_cast<int>((t / 3600 + 9) % 24));
    //分
    printf("%d分", static_cast<int>(t / 60 % 60));
    //秒
    printf("%d秒\n", static_cast<int>(t % 60));
}

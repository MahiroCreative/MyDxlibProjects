#include "DxLib.h"
#include "MyDxlib/MyDxlib.h"
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ＤＸライブラリ初期化設定
    ChangeWindowMode(TRUE);
    SetGraphMode(800, 600, 32);
    SetMainWindowText("Input Library Test");

    if (DxLib_Init() == -1) return -1;

    // 描画先を裏画面に設定
    SetDrawScreen(DX_SCREEN_BACK);

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
        // 1. 各クラスの入力を更新（これを忘れると動きません）
        InputKey::Update();
        InputMouse::Update();

        ClearDrawScreen();

        // --- 2. InputKey のテスト表示 ---
        int y = 40;
        DrawString(20, y, "=== Keyboard Test (Space Key) ===", GetColor(255, 255, 255));
        y += 30;

        // AnyKeyの判定
        if (InputKey::isAnyKey()) {
            DrawString(20, y, "Status: SOME KEY IS PRESSED!", GetColor(255, 255, 0));
        }
        else {
            DrawString(20, y, "Status: IDLE", GetColor(100, 100, 100));
        }
        y += 20;

        // Down/Hold/Timeの判定
        if (InputKey::isDownKey(KEY_INPUT_SPACE)) {
            DrawBox(20, y, 200, y + 20, GetColor(255, 0, 0), TRUE); // 押した瞬間だけ赤
            DrawString(220, y, "<- isDownKey(SPACE)", GetColor(255, 0, 0));
        }
        y += 25;

        if (InputKey::isHoldKey(KEY_INPUT_SPACE)) {
            DrawBox(20, y, 200, y + 20, GetColor(0, 255, 0), TRUE); // 押している間は緑
            std::string timeStr = "<- isHoldKey(SPACE) Time: " + std::to_string(InputKey::HoldKeyTime(KEY_INPUT_SPACE));
            DrawString(220, y, timeStr.c_str(), GetColor(0, 255, 0));
        }
        y += 60;

        // --- 3. InputMouse のテスト表示 ---
        DrawString(20, y, "=== Mouse Test ===", GetColor(255, 255, 255));
        y += 30;

        // 座標取得のテスト
        std::string posStr = "Mouse Pos: (" + std::to_string(InputMouse::GetX()) + ", " + std::to_string(InputMouse::GetY()) + ")";
        DrawString(20, y, posStr.c_str(), GetColor(255, 255, 255));
        y += 30;

        // 左クリックのテスト
        if (InputMouse::isHoldMouse(MOUSE_INPUT_LEFT)) {
            DrawCircle(InputMouse::GetX(), InputMouse::GetY(), 10 + InputMouse::HoldMouseTime(MOUSE_INPUT_LEFT) % 50, GetColor(0, 0, 255), FALSE);
            DrawString(20, y, "LEFT: HOLDING", GetColor(0, 150, 255));
        }
        else {
            DrawString(20, y, "LEFT: IDLE", GetColor(100, 100, 100));
        }
        y += 20;

        // 右クリックのテスト（同時押しの確認）
        if (InputMouse::isDownMouse(MOUSE_INPUT_RIGHT)) {
            DrawString(20, y, "RIGHT: CLICKED!", GetColor(255, 100, 0));
        }
        else if (InputMouse::isHoldMouse(MOUSE_INPUT_RIGHT)) {
            DrawString(20, y, "RIGHT: HOLDING", GetColor(255, 200, 0));
        }

        // 操作説明
        DrawString(20, 550, "ESC: Exit | SPACE: Test Keyboard | LEFT/RIGHT: Test Mouse", GetColor(150, 150, 150));

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
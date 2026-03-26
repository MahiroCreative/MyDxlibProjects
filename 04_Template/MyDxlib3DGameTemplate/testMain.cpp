#include "DxLib.h"
#include "MyDxlib/MyDxlib.h"
#include <iostream>
#include <iomanip>
#include <string>

// =================================================================
// テスト補助関数
// =================================================================

// 浮動小数点の誤差を考慮した比較
bool Near(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Vector3の値が期待通りかチェック
bool NearVec3(const Vector3& v, float x, float y, float z) {
    return Near(v.X, x) && Near(v.Y, y) && Near(v.Z, z);
}

// 結果をコンソールに表示
void PrintResult(const std::string& testName, bool success) {
    printf("%-35s : %s\n", testName.c_str(), success ? "[ PASS ]" : "[ FAIL ]");
}

// =================================================================
// テスト本体
// =================================================================

void RunVector3Tests() {
    printf("======================================\n");
    printf("   Vector3 Comprehensive Test Start   \n");
    printf("======================================\n\n");

    // 1. 定数チェック
    // DxLibの右手系（奥がZマイナス）に基づいているか
    bool consts = NearVec3(Vector3::Up, 0, 1, 0) &&
        NearVec3(Vector3::Down, 0, -1, 0) &&
        NearVec3(Vector3::Forward, 0, 0, -1) &&
        NearVec3(Vector3::Back, 0, 0, 1) &&
        NearVec3(Vector3::Right, 1, 0, 0) &&
        NearVec3(Vector3::Left, -1, 0, 0);
    PrintResult("1. Constants (Forward/Up/etc)", consts);

    // 2. 基本算術演算子
    Vector3 a = { 10.0f, 20.0f, 30.0f };
    Vector3 b = { 5.0f, 2.0f, 1.0f };
    bool ops = NearVec3(a + b, 15, 22, 31) &&
        NearVec3(a - b, 5, 18, 29) &&
        NearVec3(a * 2.0f, 20, 40, 60) &&
        NearVec3(2.0f * a, 20, 40, 60) &&
        NearVec3(a / 2.0f, 5, 10, 15) &&
        NearVec3(-a, -10, -20, -30);
    PrintResult("2. Basic Operators (+-*/)", ops);

    // 3. 複合代入演算子
    Vector3 c = { 10, 10, 10 };
    c += {5, 5, 5}; // 15, 15, 15
    c *= 2.0f;      // 30, 30, 30
    PrintResult("3. Compound Assignment", NearVec3(c, 30, 30, 30));

    // 4. 長さと正規化
    Vector3 vLong = { 0.0f, 3.0f, 4.0f }; // 長さ5のベクトル
    bool lenNorm = Near(vLong.LengthSq(), 25.0f) &&
        Near(vLong.Length(), 5.0f) &&
        NearVec3(vLong.Normalize(), 0.0f, 0.6f, 0.8f);
    PrintResult("4. Length & Normalize", lenNorm);

    // 5. 外積 (Cross Product)
    // 右手系: 右(1,0,0) x 上(0,1,0) = 手前(0,0,1)
    Vector3 cross = Vector3::Cross(Vector3::Right, Vector3::Up);
    PrintResult("5. Cross (Right*Up=Back)", NearVec3(cross, 0, 0, 1));

    // 6. 反射ベクトル (Reflect)
    // 右下(1,-1,0) に進む弾が 床(法線:上 0,1,0) に当たったら 右上(1,1,0) になる
    Vector3 reflect = Vector3::Reflect({ 1, -1, 0 }, Vector3::Up);
    PrintResult("6. Reflect (Floor Bounce)", NearVec3(reflect, 1, 1, 0));

    // 7. 平面投影 (ProjectOnPlane / 壁ずり)
    // 斜め前(1, 0, -1)に進もうとして、正面の壁(法線:手前 0,0,1)に当たったら、横(1,0,0)だけ残る
    Vector3 slide = Vector3::ProjectOnPlane({ 1, 0, -1 }, Vector3::Back);
    PrintResult("7. ProjectOnPlane (Wall Slide)", NearVec3(slide, 1, 0, 0));

    // 8. なす角 (Angle)
    // 上(0,1,0) と 右(1,0,0) は 90度
    float deg = Vector3::Angle(Vector3::Up, Vector3::Right);
    PrintResult("8. Angle (Up & Right = 90deg)", Near(deg, 90.0f));

    // 9. 線形補間 (Lerp)
    Vector3 vMid = Vector3::Lerp({ 0, 0, 0 }, { 10, 20, 30 }, 0.5f);
    PrintResult("9. Lerp (0.5f Midpoint)", NearVec3(vMid, 5, 10, 15));

    // 10. 型変換と代入 (DxLib VECTOR)
    Vector3 vConv = { 1.1f, 2.2f, 3.3f };
    VECTOR dv = vConv; // operator VECTOR
    Vector3 vBack;
    vBack = dv; // operator=
    bool conv = Near(dv.x, 1.1f) && Near(dv.y, 2.2f) && Near(dv.z, 3.3f) &&
        NearVec3(vBack, 1.1f, 2.2f, 3.3f);
    PrintResult("10. DxLib Interop (VECTOR)", conv);

    printf("\n======================================\n");
    printf("            Test Finished             \n");
    printf("======================================\n");
}

// =================================================================
// エントリポイント
// =================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    // ウィンドウモード設定
    ChangeWindowMode(TRUE);
    SetGraphMode(640, 480, 32);
    SetWindowText("Vector3 Test Core");

    // コンソールを開いて printf を有効にする
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    if (DxLib_Init() == -1) return -1;

    // 描画先を裏画面にする
    SetDrawScreen(DX_SCREEN_BACK);

    // テスト実行
    RunVector3Tests();

    printf("\n[SUCCESS] すべての確認が終了しました。\n");
    printf("何かキーを押すと終了します...\n");

    // ループ
    while (ProcessMessage() == 0 && CheckHitKeyAll() == 0) {
        ClearDrawScreen();
        DrawString(20, 20, "Vector3 test results are in the console window.", GetColor(255, 255, 255));
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
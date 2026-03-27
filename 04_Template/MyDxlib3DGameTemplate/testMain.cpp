#include "DxLib.h"
#include "MyDxlib/MyDxlib.h"
#include <iostream>
#include <string>

// =================================================================
// 検証用ヘルパー
// =================================================================

bool Near(float a, float b) { return std::abs(a - b) < 0.01f; }

void PrintTest(const std::string& name, bool success) {
    printf("%-45s : %s\n", name.c_str(), success ? "[ PASS ]" : "[ FAIL ]");
}

// Vector3 を ＤＸライブラリの VECTOR 型に変換するヘルパー
VECTOR ToDX(const Vector3& v) { return VGet(v.X, v.Y, v.Z); }

// =================================================================
// 1. ロジック検証 (コンソール)
// =================================================================

void RunLogicTests() {
    printf("============================================\n");
    printf("   Math Library Comprehensive Logic Test    \n");
    printf("============================================\n\n");

    // --- Vector3 ---
    Vector3 v1 = { 1, 0, 0 };
    PrintTest("Vector3: Cross Product (X x Y = Z)", (Vector3::Cross(v1, Vector3{ 0, 1, 0 })).Z == 1.0f);

    // --- Quaternion ---
    Quaternion q1 = Quaternion::AngleAxis(90.0f, Vector3::Up);
    Vector3 vRot = q1 * Vector3::Forward; // 前を向いて右に90度
    PrintTest("Quaternion: Rotation (Forward to Left)", Near(vRot.X, -1.0f));

    // --- Matrix4x4 ---
    // 逆行列の検証: M * M^-1 = Identity
    Matrix4x4 mOrig = Matrix4x4::Translation({ 1, 2, 3 }) * Matrix4x4::Rotation({ 30, 60, 90 });
    Matrix4x4 mInv = mOrig.Inverse();
    Matrix4x4 mRes = mOrig * mInv;
    PrintTest("Matrix4x4: Manual Inverse Accuracy", Near(mRes.m[0][0], 1.0f) && Near(mRes.m[0][3], 0.0f));

    // 合成順序の検証 (T * R * S)
    Matrix4x4 mTRS = Matrix4x4::Translation({ 0, 10, 0 }) * Matrix4x4::RotationX(90.0f);
    Vector3 vFinal = mTRS * Vector3{ 0, 1, 0 }; // 上(0,1,0)をX90度で奥(0,0,1)へ、その後Yに+10
    PrintTest("Matrix4x4: Composition (T * R)", Near(vFinal.Y, 10.0f) && Near(vFinal.Z, 1.0f));

    printf("\n============================================\n\n");
}

// =================================================================
// 2. 描画検証 (3Dウィンドウ)
// =================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ＤＸライブラリ初期化設定
    ChangeWindowMode(TRUE);
    SetGraphMode(800, 600, 32);
    AllocConsole();
    FILE* fp; freopen_s(&fp, "CONOUT$", "w", stdout);

    if (DxLib_Init() == -1) return -1;

    // 数学ロジックテスト実行
    RunLogicTests();

    // カメラ設定
    // 自作行列でビュー行列を作ってセットするテスト
    Vector3 camPos = { 0, 300, -600 };
    Vector3 camTarget = { 0, 0, 0 };
    // ＤＸライブラリ標準関数でカメラを立てる
    SetCameraPositionAndTargetAndUpVec(ToDX(camPos), ToDX(camTarget), VGet(0, 1, 0));
    SetCameraNearFar(10.0f, 2000.0f);

    float angle = 0.0f;

    while (ProcessMessage() == 0 && CheckHitKeyAll() == 0) {
        ClearDrawScreen();

        angle += 2.0f;

        // --- テストA: ワールド行列による座標変換 ---
        // 原点を中心に、半径200で回転する球
        Matrix4x4 world = Matrix4x4::RotationY(angle) * Matrix4x4::Translation({ 200, 0, 0 });
        Vector3 spherePos = world * Vector3{ 0, 0, 0 };
        DrawSphere3D(ToDX(spherePos), 20.0f, 16, GetColor(255, 255, 0), GetColor(255, 255, 255), TRUE);

        // --- テストB: 逆行列の打ち消し確認 ---
        // 移動した後に逆行列を掛けて「原点」に戻るか確認（常に中心に緑の球があれば成功）
        Matrix4x4 move = Matrix4x4::Translation({ 150, 0, 150 }) * Matrix4x4::RotationY(angle * 0.5f);
        Matrix4x4 invMove = move.Inverse();
        Vector3 originBack = (move * invMove) * Vector3{ 0, 0, 0 };
        DrawSphere3D(ToDX(originBack), 10.0f, 8, GetColor(0, 255, 0), GetColor(255, 255, 255), TRUE);

        // --- テストC: Slerp (球面線形補間) による回転 ---
        // 0度回転と180度回転の間をゆーっくり往復する
        float t = (std::sin(angle * 0.05f) + 1.0f) * 0.5f;
        Quaternion qStart = Quaternion::Identity;
        Quaternion qEnd = Quaternion::AngleAxis(180.0f, Vector3::Up);
        Quaternion qSlerp = Quaternion::Slerp(qStart, qEnd, t);

        // 補間されたクォータニオンを行列に変換して描画
        Matrix4x4 matSlerp = Matrix4x4::Translation({ 0, 100, 0 }) * Matrix4x4::FromQuaternion(qSlerp);
        Vector3 slerpPos = matSlerp * Vector3{ 0, 0, 100 }; // 棒の先端のような位置
        DrawLine3D(VGet(0, 100, 0), ToDX(slerpPos), GetColor(255, 0, 255));
        DrawSphere3D(ToDX(slerpPos), 15.0f, 8, GetColor(255, 0, 255), GetColor(255, 255, 255), TRUE);

        // 地面のグリッド（目印）
        for (int i = -5; i <= 5; i++) {
            DrawLine3D(VGet(i * 100.0f, 0, -500), VGet(i * 100.0f, 0, 500), GetColor(100, 100, 100));
            DrawLine3D(VGet(-500, 0, i * 100.0f), VGet(500, 0, i * 100.0f), GetColor(100, 100, 100));
        }

        DrawString(20, 20, "Yellow: World Matrix (T * R)", GetColor(255, 255, 255));
        DrawString(20, 40, "Green: Inverse Matrix (M * M^-1 = Origin)", GetColor(255, 255, 255));
        DrawString(20, 60, "Pink: Quaternion Slerp", GetColor(255, 255, 255));

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
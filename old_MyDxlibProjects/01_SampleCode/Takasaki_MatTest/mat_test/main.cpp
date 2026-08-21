#include "DxLib.h"
#include"Vec3.h"
#include"Quaternion.h"
#include"Camera.h"
#include"PlayerSphere.h"
#include<memory>
#include<list>
#include"Physics.h"
#include"Collidable.h"
#include"Pad.h"

using namespace MyEngine;

namespace
{
    constexpr int kNetralRadius = 20;//通常時の当たり半径

    constexpr int kDamageFrameMax = 20;
    //アニメーション番号
    constexpr int kIdleAnimIndex = 1;
    //constexpr int kIdleAnimIndex = 2;//待機テスト
    constexpr int kAttackAnimIndex = 30;

    constexpr float kAnimFrameSpeed = 30.0f;//アニメーション進行速度

    //アニメーションの切り替えにかかるフレーム数
    constexpr float kAnimChangeFrame = 8.0f;
    constexpr float kAnimChangeRateSpeed = 1.0f / kAnimChangeFrame;

    //アナログスティックによる移動関連
    constexpr float kMaxSpeed = 5.0f;//プレイヤーの最大速度
    constexpr float kAnalogRangeMin = 0.1f;//アナログスティックの入力判定範囲
    constexpr float kAnalogRangeMax = 0.8f;
    constexpr float kAnalogInputMax = 1000.0f;//アナログスティックから入力されるベクトルの最大値

    constexpr float kFrameParSecond = 60.0f;

    constexpr int kAvoidFrame = 60;

    constexpr int kJumpPower = 50;

    constexpr int kSearchRemainTimeMax = 28;
    constexpr int kChargeRemainCoolTime = 10;

    const char* kGororiHitSEName = "GororiHitSE.mp3";
    const char* kGetItemSEName = "GetItemSE.mp3";
    const char* kOnParrySEName = "Parry.mp3";

    const char* kGetSearchSEName = "Search.mp3";
    const char* name = "Player";
    const char* kFileName = "SpaceHarrier.mv1";

    constexpr int kAnimationNumTpose = 0;
    constexpr int kAnimationNumHit = 1;
    constexpr int kAnimationNumJump = 2;
    constexpr int kAnimationNumRun = 3;
    constexpr int kAnimationNumSpin = 4;
    constexpr int kAnimationNumIdle = 5;

    //照準
    const char* kAimGraphFileName = "Elements_pro.png";
    constexpr int kAimGraphSrkX = 3140;
    constexpr int kAimGraphSrkY = 200;
    constexpr int kAimGraphWidth = 400;
    constexpr int kAimGraphHeight = 370;
}

float GetAngle(Vec3 a, Vec3 b)
{
    a.Normalize();
    b.Normalize();

    float cos = Dot(a, b);
    // コサインをクリッピング
    cos = max(-1.0f, min(1.0f, cos));

    float rad = acos(cos);

#ifdef _DEBUG
    DrawFormatString(0, 125, 0xffffff, "rad(%f),deg(%f)", rad, rad * 180 / DX_PI_F);
#endif

    return rad;
}

bool UpdateAnim(int attachNo,int modelHandle)
{
    //アニメーションが設定されていないので終了
    if (attachNo == -1) return false;

    //アニメーションを進行させる
    float now = MV1GetAttachAnimTime(modelHandle, attachNo);//現在の再生カウント
    now += kAnimFrameSpeed / kFrameParSecond;//アニメーションカウントを進める


    //現在再生中のアニメーションの総カウントを取得する
    float total = MV1GetAttachAnimTotalTime(modelHandle, attachNo);
    bool isLoop = false;
    while (now >= total)
    {
        now -= total;
        isLoop = true;
    }

    MV1SetAttachAnimTime(modelHandle, attachNo, now);

    return isLoop;
}

// プログラムは WinMain から始まります
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) 
{
    ChangeWindowMode(true); // ウィンドウモードにします
    //SetGraphMode(m_windowSize.w, m_windowSize.h, 1);

    SetUseDirect3DVersion(DX_DIRECT3D_11);


    SetWindowSizeChangeEnableFlag(TRUE, TRUE);
    SetChangeScreenModeGraphicsSystemResetFlag(false);

    SetWindowSize(1600, 900);

    void* CallBack();


    DxLib_Init();

    SetUseASyncChangeWindowModeFunction(TRUE, NULL, NULL);

    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(true);
    SetUseBackCulling(true);


    SetWindowText("なめぇを決めてください");
    if (DxLib_Init() == -1)
    {
        return false;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    LONGLONG time;

    
    int modelHandle = MV1LoadModel("SpaceHarrier.mv1");
    int stageHandle = MV1LoadModel("Stage.mv1");
    int prevAnimNo=-1;
    int currentAnimNo=0;
    float animBlendRate=0;

    shared_ptr<Camera> camera;
    camera=make_shared<Camera>();
    list<shared_ptr<Collidable>> star;
    
    SetGlobalAmbientLight(GetColorF(0.0f, 0.0f, 1.0f, 1.0f));
    MV1AttachAnim(modelHandle, 0);
    float rotate = 0;
    Vec3 playerPos(0,-0,0);
    Vec3 nowVec= VGet(0, 0, -1);
    Vec3 dir;
    Vec3 shotDir;
    Vec3 moveDir;
    Vec3 sideVec(1,0,0);
    Vec3 frontVec(0,0,1);
    Vec3 upVec(0,1,0);
    float angle=0;

    auto ChangeAnim = [&](int animIndex) { //さらに古いアニメーションがアタッチされている場合はこの時点で削除しておく
        if (prevAnimNo != -1)
        {
            MV1DetachAnim(modelHandle, prevAnimNo);
        }

        //現在再生中の待機アニメーションは変更前のアニメーション扱いに
        prevAnimNo = currentAnimNo;

        //変更後のアニメーションとして攻撃アニメーションを改めて設定する
        currentAnimNo = MV1AttachAnim(modelHandle, animIndex, -1, false);

        //切り替えの瞬間は変更前のアニメーションが再生される状態にする
        animBlendRate = 0.0f;

        //変更前のアニメーション100%
        MV1SetAttachAnimBlendRate(modelHandle, prevAnimNo, 1.0f - animBlendRate);
        //変更後のアニメーション0%
        MV1SetAttachAnimBlendRate(modelHandle, currentAnimNo, animBlendRate);
    };

    auto Move = [&]() {
        int analogX = 0, analogY = 0;

        GetJoypadAnalogInput(&analogX, &analogY, DX_INPUT_PAD1);

        analogY = -analogY;


        //アナログスティックの入力10%~80%を使用する
        //ベクトルの長さが最大1000になる
        //ベクトルの長さを取得
        Vec3 move;

        float len = move.Length();
        //ベクトルの長さを0.0~1.0の割合に変換する
        float rate = len / kAnalogInputMax;
        sideVec = GetCameraRightVector();
        frontVec = (Vec3::Front()*-1).GetNormalized();

        move = frontVec * static_cast<float>(analogY);//入力が大きいほど利教が大きい,0の時は0
        move += sideVec * static_cast<float>(analogX);

        //アナログスティック無効な範囲を除外する
        rate = (rate - kAnalogRangeMin / (kAnalogRangeMax - kAnalogRangeMin));
        rate = min(rate, 1.0f);
        rate = max(rate, 0.0f);

        //速度が決定できるので移動ベクトルに反映
        move = move.GetNormalized();
        float speed = kMaxSpeed;
        

        //m_angle = fmodf(m_cameraAngle, 360);//mod:余り　
        //MATRIX rotate = MGetRotY((m_angle)-DX_PI_F / 2);//本来はカメラを行列で制御し、その行列でY軸回転
        moveDir = move;
        if (moveDir.Length() == 0)
        {
            moveDir = Vec3(0, 0, 1);
        }
        move = move * speed;
        return move;
    };

    auto ShotTheStar = [&]()
    {
        Vec3 shotPos = playerPos;
        shotPos += upVec.GetNormalized() * 70;
        star.push_back(make_shared<PlayerSphere>(Collidable::Priority::Low, ObjectTag::PlayerBullet,shotPos, shotDir, 1, 0xff0000));
        MyEngine::Physics::GetInstance().Entry(star.back());
    };

    ChangeAnim(kAnimationNumIdle);
    int speed = 1;
    int num=0;
   
    Vec3 front = Vec3::Back();
    Vec3 nowNeckDir= front;

    int index = MV1SearchFrame(modelHandle, "mixamorig:Neck");
    MATRIX ident = MV1GetFrameLocalMatrix(modelHandle, index);
    MV1SetFrameUserLocalMatrix(modelHandle, index, ident);
    while (ProcessMessage() != -1)
    {
        // FPSの固定ように開始時の時間を取得
        time = GetNowHiPerformanceCount();

        ClearDrawScreen();
        if (CheckHitKey(KEY_INPUT_ESCAPE))
        {
            ChangeWindowMode(true);
        }
        
        int directX = 0, directY = 0;
        GetJoypadAnalogInputRight(&directX, &directY, DX_INPUT_PAD1);
        directY = -directY;

        dir = VScale(GetCameraRightVector(), static_cast<float>(directX) * 0.001f);
        dir+=VScale( VGet(0,1,0), static_cast<float>(directY) * 0.001f);
        shotDir = dir;
        dir += Vec3(0,0,-1);
        shotDir += moveDir;
        dir.Normalize();
        shotDir.Normalize();


        playerPos += Move();
        camera->SetCameraPoint(playerPos-Vec3(0,-200,-200));
        camera->Update(playerPos);

        MV1SetScale(stageHandle, VGet(0.05f, 0.05f, 0.05f));
        MV1SetScale(modelHandle,VGet(0.05f, 0.05f, 0.05f));


        //モデルの回転
        angle+=0.02f;

        Quaternion myQ;
        
        angle += 0.02f;
        Vec3 axis = Cross(upVec,moveDir);
        axis.Normalize();
        DrawLine3D(playerPos.VGet(), (playerPos + axis * 60).VGet(), 0xff00ff);
        //myQ =myQ.QMult(myQ,myQ.CreateRotationQuaternion(angle, axis));
        myQ = myQ.QMult(myQ,myQ.CreateRotationQuaternion(atan2(-moveDir.x, -moveDir.z), upVec));
        auto rotatemat = myQ.ToMat();
      
       
        MV1SetRotationMatrix(modelHandle, rotatemat /*Vec3(0, atan2(moveDir.z, -moveDir.x) + DX_PI_F / 2, 0).VGet()*/);
        

        //位置の設定
        //MV1SetPosition(stageHandle, Vec3(0, -50, 0).VGet());
        MV1SetPosition(modelHandle, playerPos.VGet());

        for (auto& item : star)
        {
            item->Update();
        }

        Pad::Update();

        Physics::GetInstance().Update();

        //rotate += 0.1f;
        {
            int index = MV1SearchFrame(modelHandle, "mixamorig:Spine");
            MATRIX shotDirMat = MGetRotVec2(nowVec.VGet(), dir.VGet());
            nowVec = dir;
            MATRIX localMat = MV1GetFrameLocalMatrix(modelHandle, index);
            MATRIX mat = MMult(shotDirMat, localMat);
            MV1SetFrameUserLocalMatrix(modelHandle, index, mat);
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    DrawFormatString(i * 80, j * 16, GetColor(255, 255, 255), "%f", mat.m[i][j]);
                }
            }

            if (Pad::IsTrigger(PAD_INPUT_3))
            {
                ShotTheStar();
                nowVec = VGet(0, 0, -1);
                MV1SetFrameUserLocalMatrix(modelHandle, index, MGetIdent());
            }
        }
        
        {
            num += speed;
            int index = MV1SearchFrame(modelHandle, "mixamorig:LeftArm");
            MATRIX shotDirMat = MGetTranslate(Vec3(0, speed*5, 0).VGet());

            MATRIX localMat = MV1GetFrameLocalMatrix(modelHandle, index);
            MATRIX mat = MMult(shotDirMat, localMat);
            MV1SetFrameUserLocalMatrix(modelHandle, index, mat);

            if (Pad::IsTrigger(PAD_INPUT_3))
            {
                ShotTheStar();
                nowVec = VGet(0, 0, -1);
                MV1SetFrameUserLocalMatrix(modelHandle, index, MGetIdent());
            }
            if (num > 200)speed *= -1;
            if (num < 0)speed *= -1;
        }
        
       /* {
            int index = MV1SearchFrame(modelHandle, "mixamorig:Neck");
            Vec3 neckPos = MV1GetFramePosition(modelHandle, index);
            MATRIX ident = MV1GetFrameLocalMatrix(modelHandle, index);
            Vec3 toTarget = playerPos - neckPos;
            toTarget.Normalize();

            DrawLine3D(neckPos.VGet(), (neckPos + toTarget * 30).VGet(), 0xff0000);
            DrawLine3D(neckPos.VGet(), (neckPos + nowNeckDir * 30).VGet(), 0x0000ff);
            if (GetAngle(front, toTarget) <= DX_PI_F / 2)
            {
                MATRIX DirMat = MGetRotVec2(nowNeckDir.VGet(), toTarget.VGet());
                nowNeckDir = toTarget;
                MATRIX localMat = MV1GetFrameLocalMatrix(modelHandle, index);
                MATRIX mat = MMult(DirMat, localMat);
                MV1SetFrameUserLocalMatrix(modelHandle, index, mat);
            }
            else
            {
                nowNeckDir = front;
                MV1SetFrameUserLocalMatrix(modelHandle, index, ident);
            }
        }*/


        UpdateAnim(currentAnimNo,modelHandle);
        //変更前のアニメーション100%
        MV1SetAttachAnimBlendRate(modelHandle, prevAnimNo, 1.0f - animBlendRate);
        //変更後のアニメーション0%
        MV1SetAttachAnimBlendRate(modelHandle, currentAnimNo, animBlendRate);
        animBlendRate += 0.05f;
        if (animBlendRate > 1.0f)
        {
            animBlendRate = 1.0f;
        }
        //モデルの描画

        MV1DrawModel(stageHandle);
        MV1DrawModel(modelHandle);

        DrawSphere3D(playerPos.VGet(), 5, 8, 0xff0000, 0xffffff, true);

        for (auto& item : star)
        {
            item->Draw();
        }

#ifdef  _DEBUG

        DrawLine3D(Vec3(playerPos+upVec*50).VGet(), Vec3(playerPos + frontVec * 80+upVec*50).VGet(), 0xffffff);
        DrawLine3D(Vec3(playerPos + upVec * 50).VGet(), Vec3(playerPos + shotDir * 80 + upVec * 50).VGet(), 0x00ff00);

#endif //  _DEBUG

  
        ScreenFlip();

        // 60FPSに固定
        while (16667 > GetNowHiPerformanceCount() - time) {};
    }

    DxLib_End();
	return 0;
}
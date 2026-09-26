#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    VERTEX3DSHADER Vertex[ 6 ] ;
    int vshandle ;
    int pshandle ;
    int texhandle ;
    float angle ;
    FLOAT4 pos ;
    MATRIX mtx ;

    // ＤＸライブラリの初期化
    if( DxLib_Init() < 0 )
    {
        // エラーが発生したら直ちに終了
        return -1 ;
    }

    // ２ポリゴン分の頂点のデータをセットアップ
    Vertex[ 0 ].pos  = VGet( -128.0f,  128.0f,  0.0f ) ;
    Vertex[ 0 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 0 ].dif  = GetColorU8( 255,  0,255,255 ) ;
    Vertex[ 0 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 0 ].u    = 0.0f ;
    Vertex[ 0 ].v    = 0.0f ;
    Vertex[ 0 ].su   = 0.0f ;
    Vertex[ 0 ].sv   = 0.0f ;

    Vertex[ 1 ].pos  = VGet(  128.0f,  128.0f,  0.0f ) ;
    Vertex[ 1 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 1 ].dif  = GetColorU8(   0,  0,255,255 ) ;
    Vertex[ 1 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 1 ].u    = 1.0f ;
    Vertex[ 1 ].v    = 0.0f ;
    Vertex[ 1 ].su   = 0.0f ;
    Vertex[ 1 ].sv   = 0.0f ;

    Vertex[ 2 ].pos  = VGet( -128.0f, -128.0f,  0.0f ) ;
    Vertex[ 2 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 2 ].dif  = GetColorU8( 255,255,  0,255 ) ;
    Vertex[ 2 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 2 ].u    = 0.0f ;
    Vertex[ 2 ].v    = 1.0f ;
    Vertex[ 2 ].su   = 0.0f ;
    Vertex[ 2 ].sv   = 0.0f ;


    Vertex[ 3 ].pos  = VGet( -128.0f, -128.0f,  0.0f ) ;
    Vertex[ 3 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 3 ].dif  = GetColorU8( 255,255,  0,255 ) ;
    Vertex[ 3 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 3 ].u    = 0.0f ;
    Vertex[ 3 ].v    = 1.0f ;
    Vertex[ 3 ].su   = 0.0f ;
    Vertex[ 3 ].sv   = 0.0f ;

    Vertex[ 4 ].pos  = VGet(  128.0f,  128.0f,  0.0f ) ;
    Vertex[ 4 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 4 ].dif  = GetColorU8(   0,  0,255,255 ) ;
    Vertex[ 4 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 4 ].u    = 1.0f ;
    Vertex[ 4 ].v    = 0.0f ;
    Vertex[ 4 ].su   = 0.0f ;
    Vertex[ 4 ].sv   = 0.0f ;

    Vertex[ 5 ].pos  = VGet(  128.0f, -128.0f,  0.0f ) ;
    Vertex[ 5 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 5 ].dif  = GetColorU8( 255,  0,  0,255 ) ;
    Vertex[ 5 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 5 ].u    = 1.0f ;
    Vertex[ 5 ].v    = 1.0f ;
    Vertex[ 5 ].su   = 0.0f ;
    Vertex[ 5 ].sv   = 0.0f ;

    // 頂点シェーダーを読み込む
    vshandle = LoadVertexShader( "SetVSConstFMtxTestVS.vso" ) ;

    // ピクセルシェーダーを読み込む
    pshandle = LoadPixelShader( "SetVSConstFMtxTestPS.pso" ) ;

    // 描画に使用する画像の読み込み
    texhandle = LoadGraph( "Tex1.bmp" ) ;

    // 描画先を裏画面にする
    SetDrawScreen( DX_SCREEN_BACK ) ;

    // 回転角度の値を初期化
    angle = 0.0f ;

    // 頂点シェーダー FLOAT4型定数０番に表示座標をセット
    pos.x = 320.0f ;
    pos.y = 240.0f ;
    pos.z = 0.0f ;
    pos.w = 0.0f ;
    SetVSConstF( 0, pos ) ;

    // ESCキーが押されるまでループ
    while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
    {
        // 画面を初期化
        ClearDrawScreen() ;

        // 回転角度を変化させる
        angle += 0.05f ;

        // 回転角度からZ軸回転行列を作成
        mtx = MGetRotZ( angle ) ;

        // 回転行列を頂点シェーダー FLOAT4型定数１０～１３番にセット
        SetVSConstFMtx( 10, mtx ) ;

        // 使用する頂点シェーダーのセット
        SetUseVertexShader( vshandle ) ;

        // 使用するピクセルシェーダーをセット
        SetUsePixelShader( pshandle ) ;

        // 使用するテクスチャを０番にセット
        SetUseTextureToShader( 0, texhandle ) ;

        // シェーダーを使用した２ポリゴンの描画
        DrawPolygon3DToShader( Vertex, 2 ) ;

        // 裏画面の内容を表画面に反映させる
        ScreenFlip() ;
    }

    // 使用したfloat4型定数０番の設定を無効化する
    ResetVSConstF( 0, 1 ) ;

    // 使用したfloat4型定数１０～１３番の設定を無効化する
    ResetVSConstF( 10, 4 ) ;

    // 読み込んだ頂点シェーダーの削除
    DeleteShader( vshandle ) ;

    // 読み込んだピクセルシェーダーの削除
    DeleteShader( pshandle ) ;

    // 読み込んだ画像のグラフィックハンドルを削除
    DeleteGraph( texhandle ) ;

    // ＤＸライブラリの後始末
    DxLib_End() ;

    // ソフトの終了
    return 0 ;
}

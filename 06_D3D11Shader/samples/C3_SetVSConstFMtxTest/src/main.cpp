// DxLib 関数リファレンス SetVSConstFMtx のサンプルの Direct3D 11 版。
// Tex1.bmp を貼り付けたポリゴン２枚を、C++ で設定した回転行列と頂点シェーダーを使用して回転させます。
//
// Direct3D 9 版(reference/d3d9_original/C_function/C3_SetVSConstFMtxTest)からの変更点には【D3D11】と書いています。
#include "DxLib.h"

// 【D3D11】頂点シェーダーの定数バッファ(b4)の中身。HLSL の cbuffer と同じ並びにする(大きさは 16 バイトの倍数)
struct VS_PARAM
{
    FLOAT4 CenterPosition ;     // 回転中心になる座標
    MATRIX RotateZMatrix ;      // Ｚ回転行列(DxLib の MATRIX をそのまま入れる。HLSL 側は row_major で受ける)
} ;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    VERTEX3DSHADER Vertex[ 6 ] = {} ;   // 【D3D11】使わない要素(spos, tan, binorm)も 0 にしておく
    int vshandle ;
    int pshandle ;
    int texhandle ;
    float angle ;
    FLOAT4 pos ;
    MATRIX mtx ;
    int vscb ;
    VS_PARAM *vsparam ;

    // ウインドウモードで起動
    ChangeWindowMode( TRUE ) ;

    // 【D3D11】Direct3D 11 を使う(DxLib の既定。シェーダーは Direct3D 11 用にコンパイルしたものしか使えない)
    SetUseDirect3DVersion( DX_DIRECT3D_11 ) ;

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
    // 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
    vshandle = LoadVertexShader( "shaders/bin/SetVSConstFMtxTestVS.vso" ) ;

    // ピクセルシェーダーを読み込む
    pshandle = LoadPixelShader( "shaders/bin/SetVSConstFMtxTestPS.pso" ) ;

    // 描画に使用する画像の読み込み
    texhandle = LoadGraph( "Tex1.bmp" ) ;

    // 【D3D11】SetVSConstF / SetVSConstFMtx は Direct3D 11 では何もしないので、定数バッファを作って値を渡す
    vscb = CreateShaderConstantBuffer( sizeof( VS_PARAM ) ) ;
    vsparam = ( VS_PARAM * )GetBufferShaderConstantBuffer( vscb ) ;

    // 描画先を裏画面にする
    SetDrawScreen( DX_SCREEN_BACK ) ;

    // 回転角度の値を初期化
    angle = 0.0f ;

    // 【D3D11】表示座標を定数バッファ(b4)の CenterPosition にセット
    pos.x = 320.0f ;
    pos.y = 240.0f ;
    pos.z = 0.0f ;
    pos.w = 0.0f ;
    vsparam->CenterPosition = pos ;

    // ESCキーが押されるまでループ
    while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
    {
        // 画面を初期化
        ClearDrawScreen() ;

        // 回転角度を変化させる
        angle += 0.05f ;

        // 回転角度からZ軸回転行列を作成
        mtx = MGetRotZ( angle ) ;

        // 【D3D11】回転行列を定数バッファ(b4)の RotateZMatrix にセットして、GPU に送る
        vsparam->RotateZMatrix = mtx ;
        UpdateShaderConstantBuffer( vscb ) ;
        SetShaderConstantBuffer( vscb, DX_SHADERTYPE_VERTEX, 4 ) ;

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

    // 【D3D11】ResetVSConstF の代わりに、作った定数バッファを削除する
    DeleteShaderConstantBuffer( vscb ) ;

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

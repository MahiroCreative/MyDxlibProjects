// DxLib 関数リファレンス DrawPolygonIndexed3DToShader のサンプルの Direct3D 11 版。
// LoadVertexShader のサンプル(C1)の C++ の部分のみを DrawPolygonIndexed3DToShader を使用するようにしたプログラムです。
//
// Direct3D 9 版(reference/d3d9_original/C_function/C8_DrawPolygonIndexed3DToShader)からの変更点には【D3D11】と書いています。
#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    VERTEX3DSHADER Vertex[ 4 ] = {} ;   // 【D3D11】使わない要素(spos, tan, binorm)も 0 にしておく
    unsigned short Index[ 6 ] ;
    int vshandle ;
    int pshandle ;
    int texhandle ;
    int x ;
    int xadd ;
    float color ;
    float coloradd ;
    int vscb ;
    int pscb ;
    FLOAT4 *vsparam ;
    FLOAT4 *psparam ;

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
    Vertex[ 0 ].pos  = VGet( 220.0f, 340.0f,  0.0f ) ;
    Vertex[ 0 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 0 ].dif  = GetColorU8( 255,  0,255,255 ) ;
    Vertex[ 0 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 0 ].u    = 0.0f ;
    Vertex[ 0 ].v    = 0.0f ;
    Vertex[ 0 ].su   = 0.0f ;
    Vertex[ 0 ].sv   = 0.0f ;

    Vertex[ 1 ].pos  = VGet( 420.0f, 340.0f,  0.0f ) ;
    Vertex[ 1 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 1 ].dif  = GetColorU8(   0,  0,255,255 ) ;
    Vertex[ 1 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 1 ].u    = 1.0f ;
    Vertex[ 1 ].v    = 0.0f ;
    Vertex[ 1 ].su   = 0.0f ;
    Vertex[ 1 ].sv   = 0.0f ;

    Vertex[ 2 ].pos  = VGet( 220.0f, 140.0f,  0.0f ) ;
    Vertex[ 2 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 2 ].dif  = GetColorU8( 255,255,  0,255 ) ;
    Vertex[ 2 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 2 ].u    = 0.0f ;
    Vertex[ 2 ].v    = 1.0f ;
    Vertex[ 2 ].su   = 0.0f ;
    Vertex[ 2 ].sv   = 0.0f ;

    Vertex[ 3 ].pos  = VGet( 420.0f, 140.0f,  0.0f ) ;
    Vertex[ 3 ].norm = VGet(   0.0f,   0.0f, -1.0f ) ;
    Vertex[ 3 ].dif  = GetColorU8( 255,255,  0,255 ) ;
    Vertex[ 3 ].spc  = GetColorU8(   0,  0,  0,  0 ) ;
    Vertex[ 3 ].u    = 1.0f ;
    Vertex[ 3 ].v    = 1.0f ;
    Vertex[ 3 ].su   = 0.0f ;
    Vertex[ 3 ].sv   = 0.0f ;

    // ２ポリゴン分の頂点番号配列をセットアップ
    Index[ 0 ] = 0 ;
    Index[ 1 ] = 1 ;
    Index[ 2 ] = 2 ;
    Index[ 3 ] = 2 ;
    Index[ 4 ] = 1 ;
    Index[ 5 ] = 3 ;

    // 頂点シェーダーを読み込む
    // 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
    vshandle = LoadVertexShader( "shaders/bin/VertexShaderTestVS.vso" ) ;

    // ピクセルシェーダーを読み込む
    pshandle = LoadPixelShader( "shaders/bin/VertexShaderTestPS.pso" ) ;

    // 描画に使用する画像の読み込み
    texhandle = LoadGraph( "Tex1.bmp" ) ;

    // 【D3D11】SetVSConstF / SetPSConstF は Direct3D 11 では何もしないので、定数バッファを作って値を渡す
    // 大きさは 16 バイトの倍数(FLOAT4 １個 = 16 バイト)
    vscb = CreateShaderConstantBuffer( sizeof( FLOAT4 ) ) ;
    pscb = CreateShaderConstantBuffer( sizeof( FLOAT4 ) ) ;
    vsparam = ( FLOAT4 * )GetBufferShaderConstantBuffer( vscb ) ;
    psparam = ( FLOAT4 * )GetBufferShaderConstantBuffer( pscb ) ;

    // 描画先を裏画面にする
    SetDrawScreen( DX_SCREEN_BACK ) ;

    // 表示座標を移動する処理の初期化
    x = 0 ;
    xadd = 8 ;

    // 色を変化させる処理の初期化
    color = 0.0f ;
    coloradd = 1.0f / 60.0f ;

    // ESCキーが押されるまでループ
    while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
    {
        // 画面を初期化
        ClearDrawScreen() ;

        // 座標を移動させる
        x += xadd ;
        if( x > 200 || x < -200 )
        {
            xadd = -xadd ;
        }

        // 色の値を変化させる
        color += coloradd ;
        if( color <= 0.0f || color >= 1.0f )
        {
            coloradd = -coloradd ;
        }

        // 【D3D11】座標値を頂点シェーダーの定数バッファ(b4)にセット
        vsparam->x = ( float )x ;
        vsparam->y = 0.0f ;
        vsparam->z = 0.0f ;
        vsparam->w = 0.0f ;
        UpdateShaderConstantBuffer( vscb ) ;
        SetShaderConstantBuffer( vscb, DX_SHADERTYPE_VERTEX, 4 ) ;

        // 【D3D11】色の値をピクセルシェーダーの定数バッファ(b4)にセット
        psparam->x = color ;
        psparam->y = color ;
        psparam->z = color ;
        psparam->w = 1.0f ;
        UpdateShaderConstantBuffer( pscb ) ;
        SetShaderConstantBuffer( pscb, DX_SHADERTYPE_PIXEL, 4 ) ;

        // 使用する頂点シェーダーのセット
        SetUseVertexShader( vshandle ) ;

        // 使用するピクセルシェーダーをセット
        SetUsePixelShader( pshandle ) ;

        // 使用するテクスチャを０番にセット
        SetUseTextureToShader( 0, texhandle ) ;

        // シェーダーを使用した２ポリゴンの描画
        DrawPolygonIndexed3DToShader( Vertex, 4, Index, 2 ) ;

        // 裏画面の内容を表画面に反映させる
        ScreenFlip() ;
    }

    // 【D3D11】ResetVSConstF / ResetPSConstF の代わりに、作った定数バッファを削除する
    DeleteShaderConstantBuffer( vscb ) ;
    DeleteShaderConstantBuffer( pscb ) ;

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

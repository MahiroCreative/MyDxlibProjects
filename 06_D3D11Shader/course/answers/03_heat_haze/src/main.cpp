// DxLib 関数リファレンス SetPSConstF のサンプルの Direct3D 11 版。
// Tex1.bmp を貼り付けたポリゴン２枚を、C++ で設定した r, g, b の値を乗算して描画します。
//
// Direct3D 9 版(reference/d3d9_original/C_function/C5_SetPSConstFTest)からの変更点には【D3D11】と書いています。
#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    int texhandle, pshandle ;
    VERTEX2DSHADER Vert[ 6 ] ;
    int r, g, b ;
    int radd, gadd, badd ;
    FLOAT4 f4 ;
    float time = 0.0f ;   // 【課題】経過時間(秒)
    int pscb ;
    FLOAT4 *psparam ;

    // ウインドウモードで起動
    ChangeWindowMode( TRUE ) ;

    // 【D3D11】Direct3D 11 を使う(DxLib の既定。シェーダーは Direct3D 11 用にコンパイルしたものしか使えない)
    SetUseDirect3DVersion( DX_DIRECT3D_11 ) ;

    // ＤＸライブラリの初期化
    if( DxLib_Init() < 0 ) return -1;

    // テクスチャを読み込む
    texhandle = LoadGraph( "TestTex1.jpg" ) ;    // 【課題】色の変化が分かりやすいように写真にした(256×256)

    // ピクセルシェーダーを読み込む
    // 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
    pshandle = LoadPixelShader( "shaders/bin/SetPSConstFTestPS.pso" ) ;

    // 【D3D11】SetPSConstF は Direct3D 11 では何もしないので、定数バッファを作って値を渡す
    pscb = CreateShaderConstantBuffer( sizeof( FLOAT4 ) ) ;
    psparam = ( FLOAT4 * )GetBufferShaderConstantBuffer( pscb ) ;

    // ２ポリゴン分の頂点のデータをセットアップ
    Vert[ 0 ].pos = VGet(   0.0f,   0.0f, 0.0f ) ;
    Vert[ 0 ].rhw = 1.0f ;
    Vert[ 0 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 0 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 0 ].u   = 0.0f ;
    Vert[ 0 ].v   = 0.0f ;
    Vert[ 0 ].su  = 0.0f ;
    Vert[ 0 ].sv  = 0.0f ;

    Vert[ 1 ].pos = VGet( 256.0f,   0.0f, 0.0f ) ;
    Vert[ 1 ].rhw = 1.0f ;
    Vert[ 1 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 1 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 1 ].u   = 1.0f ;
    Vert[ 1 ].v   = 0.0f ;
    Vert[ 1 ].su  = 1.0f ;
    Vert[ 1 ].sv  = 0.0f ;

    Vert[ 2 ].pos = VGet(   0.0f, 256.0f, 0.0f ) ;
    Vert[ 2 ].rhw = 1.0f ;
    Vert[ 2 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 2 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 2 ].u   = 0.0f ;
    Vert[ 2 ].v   = 1.0f ;
    Vert[ 2 ].su  = 0.0f ;
    Vert[ 2 ].sv  = 1.0f ;

    Vert[ 3 ].pos = VGet( 256.0f, 256.0f, 0.0f ) ;
    Vert[ 3 ].rhw = 1.0f ;
    Vert[ 3 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 3 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 3 ].u   = 1.0f ;
    Vert[ 3 ].v   = 1.0f ;
    Vert[ 3 ].su  = 1.0f ;
    Vert[ 3 ].sv  = 1.0f ;

    Vert[ 4 ].pos = VGet(   0.0f, 256.0f, 0.0f ) ;
    Vert[ 4 ].rhw = 1.0f ;
    Vert[ 4 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 4 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 4 ].u   = 0.0f ;
    Vert[ 4 ].v   = 1.0f ;
    Vert[ 4 ].su  = 0.0f ;
    Vert[ 4 ].sv  = 1.0f ;

    Vert[ 5 ].pos = VGet( 256.0f,   0.0f, 0.0f ) ;
    Vert[ 5 ].rhw = 1.0f ;
    Vert[ 5 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 5 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 5 ].u   = 1.0f ;
    Vert[ 5 ].v   = 0.0f ;
    Vert[ 5 ].su  = 1.0f ;
    Vert[ 5 ].sv  = 0.0f ;

    // 描画先を裏画面にする
    SetDrawScreen( DX_SCREEN_BACK ) ;

    // r, g, b の値を変化させる準備
    r = 0 ;
    g = 128 ;
    b = 255 ;
    radd = 10 ;
    gadd = 7 ;
    badd = 3 ;

    // ESCキーが押されるまでループ
    while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
    {
        // 画面を初期化
        ClearDrawScreen() ;

        // r, g, b の値を変化させる
        r += radd ;
        if( r >= 255 )
        {
            r = 255 ;
            radd = -radd ;
        }
        else
        if( r <= 0 )
        {
            r = 0 ;
            radd = -radd ;
        }

        g += gadd ;
        if( g >= 255 )
        {
            g = 255 ;
            gadd = -gadd ;
        }
        else
        if( g <= 0 )
        {
            g = 0 ;
            gadd = -gadd ;
        }

        b += badd ;
        if( b >= 255 )
        {
            b = 255 ;
            badd = -badd ;
        }
        else
        if( b <= 0 )
        {
            b = 0 ;
            badd = -badd ;
        }

        // 【D3D11】r, g, b の値をピクセルシェーダーの定数バッファ(b4)にセット
        // 定数にするときは値を 0.0f ～ 1.0f にする
        // 【課題】r, g, b の代わりに、経過時間(秒)を x に入れて渡す(1 フレームを 1/60 秒として数える)
        time += 1.0f / 60.0f ;
        f4.x = time ;
        f4.y = 0.0f ;
        f4.z = 0.0f ;
        f4.w = 0.0f ;
        *psparam = f4 ;
        UpdateShaderConstantBuffer( pscb ) ;
        SetShaderConstantBuffer( pscb, DX_SHADERTYPE_PIXEL, 4 ) ;

        // 使用するテクスチャを０番にセット
        SetUseTextureToShader( 0, texhandle ) ;

        // 使用するピクセルシェーダーをセット
        SetUsePixelShader( pshandle ) ;

        // シェーダーを使用した２Ｄの２ポリゴンの描画
        DrawPolygon2DToShader( Vert, 2 ) ;

        // 裏画面の内容を表画面に反映させる
        ScreenFlip() ;
    }

    // 読み込んだピクセルシェーダーの削除
    DeleteShader( pshandle ) ;

    // 【D3D11】ResetPSConstF の代わりに、作った定数バッファを削除する
    DeleteShaderConstantBuffer( pscb ) ;

    // 読み込んだ画像のグラフィックハンドルを削除
    DeleteGraph( texhandle ) ;

    // ＤＸライブラリの後始末
    DxLib_End();

    // ソフトの終了
    return 0;
}

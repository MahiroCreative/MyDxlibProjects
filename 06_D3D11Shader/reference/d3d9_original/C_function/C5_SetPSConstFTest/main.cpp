#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    int texhandle, pshandle ;
    VERTEX2DSHADER Vert[ 6 ] ;
    int r, g, b ;
    int radd, gadd, badd ;
    FLOAT4 f4 ;

    // ＤＸライブラリの初期化
    if( DxLib_Init() < 0 ) return -1;

    // テクスチャを読み込む
    texhandle = LoadGraph( "Tex1.bmp" ) ;

    // ピクセルシェーダーを読み込む
    pshandle = LoadPixelShader( "SetPSConstFTestPS.pso" ) ;

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

        // r, g, b の値をピクセルシェーダーの FLOAT4型定数０番にセット
        // 定数にするときは値を 0.0f ～ 1.0f にする
        f4.x = r / 255.0f ;
        f4.y = g / 255.0f ;
        f4.z = b / 255.0f ;
        f4.w = 1.0f ;
        SetPSConstF( 0, f4 ) ;

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

    // 使用した定数を無効化する
    ResetPSConstF( 0, 1 ) ;

    // 読み込んだ画像のグラフィックハンドルを削除
    DeleteGraph( texhandle ) ;

    // ＤＸライブラリの後始末
    DxLib_End();

    // ソフトの終了
    return 0;
}

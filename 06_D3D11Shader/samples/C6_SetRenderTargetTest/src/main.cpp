// DxLib 関数リファレンス SetRenderTargetToShader のサンプルの Direct3D 11 版。
// Tex1.bmp を二つの描画先に描画します、片方は画像の赤成分だけを描画して、もう片方には画像の緑成分だけを描画します。
// そしてその結果を画面に表示します。
//
// Direct3D 9 版(reference/d3d9_original/C_function/C6_SetRenderTargetTest)からの変更点には【D3D11】と書いています。
#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    int texhandle, pshandle ;
    int screen0, screen1 ;
    VERTEX2DSHADER Vert[ 6 ] ;

    // ウインドウモードで起動
    ChangeWindowMode( TRUE ) ;

    // 【D3D11】Direct3D 11 を使う(DxLib の既定。シェーダーは Direct3D 11 用にコンパイルしたものしか使えない)
    SetUseDirect3DVersion( DX_DIRECT3D_11 ) ;

    // ＤＸライブラリの初期化
    if( DxLib_Init() < 0 ) return -1;

    // テクスチャを読み込む
    texhandle = LoadGraph( "Tex1.bmp" ) ;

    // ピクセルシェーダーを読み込む
    // 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
    pshandle = LoadPixelShader( "shaders/bin/SetRenderTargetTestPS.pso" ) ;

    // ２５６×２５６の描画可能なグラフィックハンドルを二つ作成する
    screen0 = MakeScreen( 256, 256 ) ;
    screen1 = MakeScreen( 256, 256 ) ;

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
    Vert[ 1 ].su  = 0.0f ;
    Vert[ 1 ].sv  = 0.0f ;

    Vert[ 2 ].pos = VGet(   0.0f, 256.0f, 0.0f ) ;
    Vert[ 2 ].rhw = 1.0f ;
    Vert[ 2 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 2 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 2 ].u   = 0.0f ;
    Vert[ 2 ].v   = 1.0f ;
    Vert[ 2 ].su  = 0.0f ;
    Vert[ 2 ].sv  = 0.0f ;

    Vert[ 3 ].pos = VGet( 256.0f, 256.0f, 0.0f ) ;
    Vert[ 3 ].rhw = 1.0f ;
    Vert[ 3 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 3 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 3 ].u   = 1.0f ;
    Vert[ 3 ].v   = 1.0f ;
    Vert[ 3 ].su  = 0.0f ;
    Vert[ 3 ].sv  = 0.0f ;

    Vert[ 4 ].pos = VGet(   0.0f, 256.0f, 0.0f ) ;
    Vert[ 4 ].rhw = 1.0f ;
    Vert[ 4 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 4 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 4 ].u   = 0.0f ;
    Vert[ 4 ].v   = 1.0f ;
    Vert[ 4 ].su  = 0.0f ;
    Vert[ 4 ].sv  = 0.0f ;

    Vert[ 5 ].pos = VGet( 256.0f,   0.0f, 0.0f ) ;
    Vert[ 5 ].rhw = 1.0f ;
    Vert[ 5 ].dif = GetColorU8( 255,255,255,255 ) ;
    Vert[ 5 ].spc = GetColorU8(   0,  0,  0,  0 ) ;
    Vert[ 5 ].u   = 1.0f ;
    Vert[ 5 ].v   = 0.0f ;
    Vert[ 5 ].su  = 0.0f ;
    Vert[ 5 ].sv  = 0.0f ;

    // 描画先０を描画可能画像の一つ目に設定
    SetRenderTargetToShader( 0, screen0 ) ;

    // 描画先１を描画可能画像の二つ目に設定
    SetRenderTargetToShader( 1, screen1 ) ;

    // 使用するテクスチャを０番にセット
    SetUseTextureToShader( 0, texhandle ) ;

    // 使用するピクセルシェーダーをセット
    SetUsePixelShader( pshandle ) ;

    // シェーダーを使用した２Ｄの２ポリゴンの描画
    DrawPolygon2DToShader( Vert, 2 ) ;

    // 描画先を表画面に変更
    SetDrawScreen( DX_SCREEN_FRONT ) ;

    // 描画先１の設定をリセット
    SetRenderTargetToShader( 1, -1 ) ;

    // 二つの描画可能画像を並べて描画
    DrawGraph(   0, 0, screen0, FALSE ) ;
    DrawGraph( 256, 0, screen1, FALSE ) ;

    // キー入力待ち
    WaitKey() ;

    // 読み込んだピクセルシェーダーの削除
    DeleteShader( pshandle ) ;

    // MakeScreen で作成したグラフィックハンドルの削除
    DeleteGraph( screen0 ) ;
    DeleteGraph( screen1 ) ;

    // 読み込んだ画像のグラフィックハンドルを削除
    DeleteGraph( texhandle ) ;

    // ＤＸライブラリの後始末
    DxLib_End();

    // ソフトの終了
    return 0;
}

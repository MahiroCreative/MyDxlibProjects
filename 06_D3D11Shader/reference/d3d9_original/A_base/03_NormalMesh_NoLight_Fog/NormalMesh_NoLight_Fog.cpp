#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float RotateAngle ;
	float Z, ZAdd ;

	// ウインドウモードで起動
	ChangeWindowMode( TRUE ) ;

	// ＤＸライブラリの初期化
	if( DxLib_Init() < 0 )
	{
		// エラーが発生したら直ちに終了
		return -1 ;
	}

	// プログラマブルシェーダーモデル２．０が使用できない場合はエラーを表示して終了
	if( GetValidShaderVersion() < 200 )
	{
		// エラー表示
		DrawString( 0, 0, "プログラマブルシェーダー２．０が使用できない環境のようです", GetColor( 255,255,255 ) ) ;

		// キー入力待ち
		WaitKey() ;
		
		// ＤＸライブラリの後始末
		DxLib_End() ;

		// ソフト終了
		return 0 ;
	}

	// 頂点シェーダーを読み込む
	VertexShaderHandle = LoadVertexShader( "NormalMesh_NoLight_FogVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "NormalMesh_NoLight_FogPS.pso" ) ;

	// 剛体メッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "NormalBox.mqo" ) ;

	// フォグを有効にする
	SetFogEnable( TRUE ) ;

	// フォグの色を黄色にする
	SetFogColor( 255, 255, 0 ) ;

	// フォグの開始距離を０、終了距離を１５００にする
	SetFogStartEnd( 0.0f, 1500.0f ) ;

	// 描画先を裏画面にする
	SetDrawScreen( DX_SCREEN_BACK ) ;

	// モデルを回転される値を初期化
	RotateAngle = 0.0f ;

	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;

	// Ｚ値の値を初期化
	Z = 0.0f ;
	ZAdd = 10.0f ;

	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// 画面を黄色で塗りつぶす
		DrawBox( 0, 0, 640, 480, GetColor( 255, 255, 0 ), TRUE ) ;

		// モデルを回転値を加算
		RotateAngle += 0.02f ;

		// モデルの回転値をモデルにセット
		MV1SetRotationXYZ( ModelHandle, VGet( 0.0f, RotateAngle, 0.0f ) ) ;

		// モデルのＺ位置を変更
		Z += ZAdd ;
		if( Z > 1500.0f || Z < 0.0f )
			ZAdd = -ZAdd ;

		// モデルを座標をセット
		MV1SetPosition( ModelHandle, VGet( 320.0f, 240.0f, Z ) ) ;

		// モデルを描画
		MV1DrawModel( ModelHandle ) ;

		// 裏画面の内容を表画面に反映させる
		ScreenFlip() ;
	}

	// 読み込んだ頂点シェーダーの削除
	DeleteShader( VertexShaderHandle ) ;

	// 読み込んだピクセルシェーダーの削除
	DeleteShader( PixelShaderHandle ) ;

	// 読み込んだモデルの削除
	MV1DeleteModel( ModelHandle ) ;

	// ＤＸライブラリの後始末
	DxLib_End() ;

	// ソフトの終了
	return 0 ;
}

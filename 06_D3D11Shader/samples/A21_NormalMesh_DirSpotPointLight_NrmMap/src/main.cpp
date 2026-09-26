// DxLib 公式サンプル「オリジナルシェーダーを使用した３Ｄモデルの描画基本」の「法線マップ付き剛体メッシュのディレクショナルライトとスポットライトとポイントライト一つづつ描画」の Direct3D 11 版。
// reference/d3d9_original/A_base/21_NormalMesh_DirSpotPointLight_NrmMap から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
#include "DxLib.h"
#include <math.h>

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float LightRotateAngle ;
	float ModelRotateAngle ;
	int DirLightHandle ;
	int PointLightHandle ;
	int SpotLightHandle ;
	VECTOR LightPosition ;
	
	// ウインドウモードで起動
	ChangeWindowMode( TRUE ) ;

	// 【D3D11】画面を 32 ビットカラーにする。DxLib の既定は 16 ビットカラーで、Direct3D 11 ではそのとき法線マップの精度が落ち、
	// 平らな所の法線が傾いて明るさが変わる(Direct3D 9 では起きない。仕様書 10.3)
	SetGraphMode( 640, 480, 32 ) ;

	// 【D3D11】Direct3D 11 を使用する(DxLib の既定。シェーダーは Direct3D 11 用にコンパイルしたものしか使えない)
	SetUseDirect3DVersion( DX_DIRECT3D_11 ) ;

	// ＤＸライブラリの初期化
	if( DxLib_Init() < 0 )
	{
		// エラーが発生したら直ちに終了
		return -1 ;
	}

	// プログラマブルシェーダーモデル３．０が使用できない場合はエラーを表示して終了
	if( GetValidShaderVersion() < 300 )
	{
		// エラー表示
		DrawString( 0, 0, "プログラマブルシェーダー３．０が使用できない環境のようです", GetColor( 255,255,255 ) ) ;

		// キー入力待ち
		WaitKey() ;
		
		// ＤＸライブラリの後始末
		DxLib_End() ;

		// ソフト終了
		return 0 ;
	}


	// 頂点シェーダーを読み込む
	// 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
	VertexShaderHandle = LoadVertexShader( "shaders/bin/NormalMesh_DirSpotPointLight_NrmMapVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "shaders/bin/NormalMesh_DirSpotPointLight_NrmMapPS.pso" ) ;


	// 剛体メッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "NormalBox_NrmMap.mv1" ) ;

	// モデルの回転角度を初期化
	ModelRotateAngle = 0.0f ;


	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;


	// 標準ライトをオフにする
	SetLightEnable( FALSE ) ;

	// ディレクショナルライトを作成する
	DirLightHandle = CreateDirLightHandle( VGet( -1.0f, -1.0f, 0.0f ) ) ;

	// ポイントライトハンドルを作成する
	PointLightHandle = CreatePointLightHandle( VGet( 0.0f, 0.0f, 0.0f ), 1000.0f, 0.86f, 0.0000f, 0.0000f ) ;

	// スポットライトを作成する
	SpotLightHandle = CreateSpotLightHandle( VGet( 0.0f, 0.0f, 0.0f ), VGet( 0.0f, -1.0f, 0.0f ), 0.7f, 0.6f, 1000.0f, 0.391586f, 0.001662f, 0.0f ) ;

	// スポットライトのアンビエントカラーを無効にする
	SetLightAmbColorHandle( SpotLightHandle, GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ) ;

	// スポットライトのディフューズカラーを緑にする
	SetLightDifColorHandle( SpotLightHandle, GetColorF( 0.0f, 1.0f, 0.0f, 0.0f ) ) ;

	// ライトの位置を回転する値を初期化
	LightRotateAngle = 0.0f ;


	// 描画先を裏画面にする
	SetDrawScreen( DX_SCREEN_BACK ) ;

	// モデルの見える位置にカメラを配置
	SetCameraPositionAndTarget_UpVecY( VGet( 400.0f, 200.0f, -400.0f ), VGet( 0.0f, 0.0f, 0.0f ) ) ;


	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// ポイントライトの位置の回転値を加算
		LightRotateAngle += 0.02f ;

		// ポイントライトの位置の更新
		LightPosition.x = sin( LightRotateAngle ) * 600.0f ;
		LightPosition.y = 0.0f ;
		LightPosition.z = cos( LightRotateAngle ) * 600.0f ;
		SetLightPositionHandle( PointLightHandle, LightPosition ) ;

		// スポットライトの位置の更新
		LightPosition.x = 0.0f ;
		LightPosition.y = cos( LightRotateAngle ) * 700.0f ;
		LightPosition.z = sin( LightRotateAngle ) * 700.0f ;
		SetLightPositionHandle( SpotLightHandle, LightPosition ) ;

		// ライトの向きを回転中心方向にする
		SetLightDirectionHandle( SpotLightHandle, VScale( LightPosition, -1.0f ) ) ;


		// モデルの回転値を加算
		ModelRotateAngle += 0.002f ;

		// モデルの回転角度を変更
		MV1SetRotationXYZ( ModelHandle, VGet( 0.0f, ModelRotateAngle, 0.0f ) ) ;


		// モデルを描画
		MV1DrawModel( ModelHandle ) ;

		// 裏画面の内容を表画面に反映させる
		ScreenFlip() ;
	}

	// ディレクショナルライトの削除
	DeleteLightHandle( DirLightHandle ) ;

	// スポットライトの削除
	DeleteLightHandle( SpotLightHandle ) ;

	// ポイントライトの削除
	DeleteLightHandle( PointLightHandle ) ;

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

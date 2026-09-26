// DxLib 公式サンプル「オリジナルシェーダーを使用した３Ｄモデルの描画基本」の「剛体メッシュのスポットライトありフォンシェーディング描画」の Direct3D 11 版。
// reference/d3d9_original/A_base/14_NormalMesh_SpotLight_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
#include "DxLib.h"
#include <math.h>

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float LightRotateAngle ;
	VECTOR LightPosition ;

	// ウインドウモードで起動
	ChangeWindowMode( TRUE ) ;

	// 【D3D11】Direct3D 11 を使用する(DxLib の既定。シェーダーは Direct3D 11 用にコンパイルしたものしか使えない)
	SetUseDirect3DVersion( DX_DIRECT3D_11 ) ;

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
	// 【D3D11】拡張機能の「シェーダーをすべてコンパイル」の出力先 shaders/bin から読む
	VertexShaderHandle = LoadVertexShader( "shaders/bin/NormalMesh_SpotLight_PhongVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "shaders/bin/NormalMesh_SpotLight_PhongPS.pso" ) ;

	// 剛体メッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "NormalBox.mqo" ) ;

	// 描画先を裏画面にする
	SetDrawScreen( DX_SCREEN_BACK ) ;

	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;

	// 観察しやすい位置にカメラを移動
	SetCameraPositionAndTarget_UpVecY( VGet( 400.0f, 400.0f, -400.0f ), VGet( 0.0f, 0.0f, 0.0f ) ) ;

	// ライトの位置を回転する値を初期化
	LightRotateAngle = 0.0f ;

	// 標準ライトのタイプをスポットライトにする
	ChangeLightTypeSpot( VGet( 0.0f, 0.0f, 0.0f ), VGet( 1.0f, 0.0f, 0.0f ), 0.7f, 0.6f, 700.0f, 0.391586f, 0.001662f, 0.0f ) ;

	// アンビエントライトをＯＦＦにする
	SetLightAmbColor( GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ) ;

	// グローバルアンビエントライト( 大域環境光 )を２０％の明るさにする
	SetGlobalAmbientLight( GetColorF( 0.2f, 0.2f, 0.2f, 0.0f ) ) ;

	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// ライトの位置の回転値を加算
		LightRotateAngle += 0.02f ;

		// ライトの位置の更新
		LightPosition.x = sin( LightRotateAngle ) * 400.0f ;
		LightPosition.y = 0.0f ;
		LightPosition.z = cos( LightRotateAngle ) * 400.0f ;
		SetLightPosition( LightPosition ) ;

		// ライトの向きを原点方向にする
		SetLightDirection( VScale( LightPosition, -1.0f ) ) ;

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

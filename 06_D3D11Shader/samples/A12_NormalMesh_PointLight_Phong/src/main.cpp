// DxLib 公式サンプル「オリジナルシェーダーを使用した３Ｄモデルの描画基本」の「剛体メッシュのポイントライトありフォンシェーディング描画」の Direct3D 11 版。
// reference/d3d9_original/A_base/12_NormalMesh_PointLight_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
#include "DxLib.h"
#include <math.h>

#define DRAW_NUM	(3)
#define SPACE		(512.0f)

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float LightRotateAngle ;
	int i, j ;
	float DrawX, DrawZ ;

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
	VertexShaderHandle = LoadVertexShader( "shaders/bin/NormalMesh_PointLight_PhongVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "shaders/bin/NormalMesh_PointLight_PhongPS.pso" ) ;

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
	SetCameraPositionAndTarget_UpVecY( VGet( 800.0f, 400.0f, -800.0f ), VGet( 0.0f, 0.0f, 0.0f ) ) ;

	// ライトの位置を回転する値を初期化
	LightRotateAngle = 0.0f ;

	// 標準ライトをポイントライトにする
	ChangeLightTypePoint( VGet( 0.0f, 0.0f, 0.0f ), 1000.0f, 1.016523f, 0.010100f, 0.0020f ) ;

	// ライトの光を強くする
	SetLightDifColor( GetColorF( 200.0f, 200.0f, 200.0f, 0.0f ) ) ;

	// ライトのアンビエントカラーを無効にする
	SetLightAmbColor( GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ) ;

	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// ポイントライトの位置の回転値を加算
		LightRotateAngle += 0.02f ;

		// ポイントライトの位置の更新
		SetLightPosition( VGet( sin( LightRotateAngle ) * 400.0f, 400.0f, cos( LightRotateAngle ) * 400.0f ) ) ;

		// モデルを描画
		DrawZ = - ( DRAW_NUM - 1 ) * SPACE / 2.0f ;
		for( i = 0 ; i < DRAW_NUM ; i ++ )
		{
			DrawX = - ( DRAW_NUM - 1 ) * SPACE / 2.0f ;
			for( j = 0 ; j < DRAW_NUM ; j ++ )
			{
				// 位置を設定
				MV1SetPosition( ModelHandle, VGet( DrawX, 0.0f, DrawZ ) ) ;

				// 描画
				MV1DrawModel( ModelHandle ) ;

				DrawX += SPACE ;
			}
			DrawZ += SPACE ;
		}

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

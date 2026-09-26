// DxLib 公式サンプル「オリジナルシェーダーを使用した３Ｄモデルの描画基本」の「剛体メッシュのディレクショナルライトありフォンシェーディング描画」の Direct3D 11 版。
// reference/d3d9_original/A_base/11_NormalMesh_DirLight_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float RotateAngle ;

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
	VertexShaderHandle = LoadVertexShader( "shaders/bin/NormalMesh_DirLight_PhongVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "shaders/bin/NormalMesh_DirLight_PhongPS.pso" ) ;

	// 剛体メッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "NormalBox.mqo" ) ;

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

	// モデルを画面の中心にセット
	MV1SetPosition( ModelHandle, VGet( 320.0f, 240.0f, 0.0f ) ) ;

	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// モデルを回転値を加算
		RotateAngle += 0.02f ;

		// モデルの回転値をモデルにセット
		MV1SetRotationXYZ( ModelHandle, VGet( 0.0f, RotateAngle, 0.0f ) ) ;

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

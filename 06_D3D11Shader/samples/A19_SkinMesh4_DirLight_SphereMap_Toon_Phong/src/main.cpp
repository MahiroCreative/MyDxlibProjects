// DxLib 公式サンプル「オリジナルシェーダーを使用した３Ｄモデルの描画基本」の「ＭＭＤ互換のトゥーンレンダリング　　スキニングメッシュでディレクショナルライト一つとスフィアマップのフォンシェーディング」の Direct3D 11 版。
// reference/d3d9_original/A_base/19_SkinMesh4_DirLight_SphereMap_Toon_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int GradTexHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;

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
	VertexShaderHandle = LoadVertexShader( "shaders/bin/SkinMesh4_DirLight_SphereMap_Toon_PhongVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "shaders/bin/SkinMesh4_DirLight_SphereMap_Toon_PhongPS.pso" ) ;

	// グラデーションテクスチャを読み込む
	GradTexHandle = LoadGraph( "GradTex.bmp" ) ;

	// スキニングメッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "DxChara.pmx" ) ;


	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用するテクスチャ１にグラデーションテクスチャをセットする
	SetUseTextureToShader( 1, GradTexHandle ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;


	// 描画先を裏画面にする
	SetDrawScreen( DX_SCREEN_BACK ) ;

	// モデルの見える位置にカメラを配置
	SetCameraPositionAndTarget_UpVecY( VGet( 0.0f, 700.0f, -800.0f ), VGet( 0.0f, 350.0f, 0.0f ) ) ;


	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// モデルを描画
		MV1DrawModel( ModelHandle ) ;

		// 裏画面の内容を表画面に反映させる
		ScreenFlip() ;
	}

	// 使用するテクスチャからグラデーションテクスチャを外す
	SetUseTextureToShader( 1, -1 ) ;

	// グラデーションテクスチャを削除
	DeleteGraph( GradTexHandle ) ;

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

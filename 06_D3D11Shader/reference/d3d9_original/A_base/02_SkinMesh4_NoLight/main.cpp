#include "DxLib.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	int AnimIndex ;
	float AnimCounter ;

	// ウインドウモードで起動
	ChangeWindowMode( TRUE ) ;

	// Direct3D9Ex を使用する
	SetUseDirect3DVersion( DX_DIRECT3D_9EX ) ;

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
	VertexShaderHandle = LoadVertexShader( "SkinMesh4_NoLightVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "SkinMesh4_NoLightPS.pso" ) ;

	// スキニングメッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "DxChara.x" ) ;

	// アニメーション０をアタッチ
	AnimIndex = MV1AttachAnim( ModelHandle, 0 ) ;

	// アニメーションカウンタをリセット
	AnimCounter = 0.0f ;

	// 描画先を裏画面にする
	SetDrawScreen( DX_SCREEN_BACK ) ;

	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;

	// モデルの見える位置にカメラを配置
	SetCameraPositionAndTarget_UpVecY( VGet( 0.0f, 700.0f, -1100.0f ), VGet( 0.0f, 350.0f, 0.0f ) ) ;

	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// アニメーション時間を進める
		AnimCounter += 100.0f ;
		if( AnimCounter > MV1GetAnimTotalTime( ModelHandle, 0 ) )
		{
			AnimCounter -= MV1GetAnimTotalTime( ModelHandle, 0 ) ;
		}
		MV1SetAttachAnimTime( ModelHandle, AnimIndex, AnimCounter ) ;

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

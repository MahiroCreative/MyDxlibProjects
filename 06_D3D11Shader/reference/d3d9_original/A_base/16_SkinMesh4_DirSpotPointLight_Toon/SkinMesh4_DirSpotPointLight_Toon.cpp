#include "DxLib.h"
#include <math.h>

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ModelHandle ;
	int GradTexHandle ;
	int PixelShaderHandle ;
	int VertexShaderHandle ;
	float LightRotateAngle ;
	int AnimIndex ;
	float AnimCounter ;
	int DirLightHandle ;
	int PointLightHandle ;
	int SpotLightHandle ;
	VECTOR LightPosition ;

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
	VertexShaderHandle = LoadVertexShader( "SkinMesh4_DirSpotPointLight_ToonVS.vso" ) ;

	// ピクセルシェーダーを読み込む
	PixelShaderHandle = LoadPixelShader( "SkinMesh4_DirSpotPointLight_ToonPS.pso" ) ;


	// グラデーションテクスチャを読み込む
	GradTexHandle = LoadGraph( "GradTex.bmp" ) ;

	// スキニングメッシュモデルを読み込む
	ModelHandle = MV1LoadModel( "DxChara.x" ) ;

	// 分かりやすいように服のマテリアルを緑色にする
	MV1SetMaterialDifColor( ModelHandle, 1, GetColorF( 0.0f, 0.5f, 0.0f, 1.0f ) ) ;


	// アニメーション０をアタッチ
	AnimIndex = MV1AttachAnim( ModelHandle, 0 ) ;

	// アニメーションカウンタをリセット
	AnimCounter = 0.0f ;


	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader( TRUE ) ;

	// 使用するテクスチャ１にグラデーションテクスチャをセットする
	SetUseTextureToShader( 1, GradTexHandle ) ;

	// 使用する頂点シェーダーをセット
	SetUseVertexShader( VertexShaderHandle ) ;

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader( PixelShaderHandle ) ;


	// 標準ライトをオフにする
	SetLightEnable( FALSE ) ;

	// ディレクショナルライトを作成する
	DirLightHandle = CreateDirLightHandle( VGet( 1.0f, 0.0f, 0.0f ) ) ;

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
	SetCameraPositionAndTarget_UpVecY( VGet( 0.0f, 700.0f, -1100.0f ), VGet( 0.0f, 350.0f, 0.0f ) ) ;


	// ESCキーが押されるまでループ
	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 )
	{
		// 画面を初期化
		ClearDrawScreen() ;

		// ポイントライトの位置の回転値を加算
		LightRotateAngle += 0.02f ;

		// ポイントライトの位置の更新
		LightPosition.x = sin( LightRotateAngle ) * 600.0f ;
		LightPosition.y = 400.0f ;
		LightPosition.z = cos( LightRotateAngle ) * 600.0f - 250.0f ;
		SetLightPositionHandle( PointLightHandle, LightPosition ) ;

		// スポットライトの位置の更新
		LightPosition.x = 0.0f ;
		LightPosition.y = cos( LightRotateAngle ) * 700.0f + 400.0f ;
		LightPosition.z = sin( LightRotateAngle ) * 700.0f - 250.0f ;
		SetLightPositionHandle( SpotLightHandle, LightPosition ) ;

		// ライトの向きを回転中心方向にする
		SetLightDirectionHandle( SpotLightHandle, VSub( VGet( 0.0f, 400.0f, -250.0f ), LightPosition ) ) ;

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

	// ディレクショナルライトの削除
	DeleteLightHandle( DirLightHandle ) ;

	// スポットライトの削除
	DeleteLightHandle( SpotLightHandle ) ;

	// ポイントライトの削除
	DeleteLightHandle( PointLightHandle ) ;

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
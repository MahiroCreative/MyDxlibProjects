#include "DxLib.h"

/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//ただし、このシリーズで解説する内容は必要必要、というレベルです。抜けが出ます。
//また、初学者向けの解説のような事はしていません。使い方の例が書いてある程度です。
//分からない部分は自ら能動的に調べて下さい。
//Dxlibのリファレンスを頻繁に見て関数に意味などを確認しながら進めるのが正しい使い方です。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・DxLib
// 　- Key入力
//   - 文字の表示
//・ゲームプログラミング
//   - デバッグ表示(DebugDraw)
//   - キャラクタ操作
// 　- 弾の発射
//・C/C++言語
//   - bool代数

/*Dxlibのエントリーポイント*/
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*Dxlib初期化*/
	SetGraphMode(1280, 720, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*画像の読み込み*/
	int drawHandle = LoadGraph("Chara.png");

	//Player変数
	int pPosX = 100;
	int pPosY = 300;
	int pSpeed = 2;
	//Bullet変数
	int bPosX = pPosX;
	int bPosY = pPosY;
	int bSpeed = 8;
	bool isBullet = false;

	/*ゲームループ部*/
	while (TRUE)
	{
		//裏画面の初期化(ダブルバッファリング1)
		ClearDrawScreen();

		/*Key入力の取得*/
		//CheckHitKeyに対応キーの数値を入れることで使う(詳細はリファレンス)。

		//Playerの移動
		if (CheckHitKey(KEY_INPUT_W))//Wで上移動
		{
			pPosY -= pSpeed;
		}
		else if (CheckHitKey(KEY_INPUT_S))//Sで下移動
		{
			pPosY += pSpeed;
		}
		else if (CheckHitKey(KEY_INPUT_D))//Dで右移動
		{
			pPosX += pSpeed;
		}
		else if (CheckHitKey(KEY_INPUT_A))//Aで左移動
		{
			pPosX -= pSpeed;
		}

		//弾の発射
		if (CheckHitKey(KEY_INPUT_RETURN) && isBullet==false)//弾は一発しか発射できない
		{
			isBullet = true;
		}
		//弾の移動
		if (isBullet)
		{
			bPosX += bSpeed;
		}
		//弾の位置初期化
		if (bPosX>1280)//画面外に出たら
		{
			bPosX = pPosX;
			bPosY = pPosY;
			isBullet = false;
		}

		/*Draw*/
		DrawGraph(pPosX,pPosY,drawHandle,1);//キャラクタ描画
		if (isBullet)//弾の描画
		{
			DrawCircle(bPosX,bPosY,4,GetColor(255,255,255),1);//GetColorはカラーコードで色を指定できる
		}

		/*DebugDraw*/
		//ゲームプレイには直接必要ではないが、開発中には必要な内容を表示している。
		DrawString(0, 0,"操作説明:WASDで上左下右,Enterで発射",GetColor(255,0,0));//文字列を表示する関数

		//裏画面を表へ(ダブルバッファリング3)
		ScreenFlip();
		/*リフレッシュ処理(-1ならエラー)*/
		if (ProcessMessage() < 0) { break; }
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//終了 
}
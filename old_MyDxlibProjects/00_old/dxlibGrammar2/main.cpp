#include "DxLib.h"

/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//ただし、このシリーズで解説する内容は必要必要、というレベルです。抜けが出ます。
//Dxlibのリファレンスを頻繁に見て関数に意味などを確認しながら進めるのが正しい使い方です。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・DxLib
// 　- 	DrawRotaGraph();
//・ゲームプログラミング
//   - 円の当たり判定

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*Dxlib初期化*/
	SetGraphMode(1280, 720, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*画像の読み込み*/
	int drawHandle = LoadGraph("Chara.png");

	/*変数*/
	//Player変数
	int pPosX = 100;//x座標
	int pPosY = 300;//y座標
	double pScale = 1;//画像の倍率
	double pRotate = 0;//回転角度(ラジアン)
	int pSpeed = 2;
	//Bullet変数
	int bPosX = pPosX;
	int bPosY = pPosY;
	int bR = 4;
	int bSpeed = 8;
	bool isBullet = false;
	//Enemy変数
	int ePosX = 1000;
	int ePosY = 360;
	int eR = 80;
	int eSpeed = 8;
	bool isHit = false;

	/*ゲームループ部*/
	//gameRoop.
	while (TRUE)
	{
		//裏画面の初期化
		ClearDrawScreen();

		/*Player処理*/
		//移動処理
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
		//回転処理
		if (CheckHitKey(KEY_INPUT_R))
		{
			pRotate += 0.1;
		}
		else if (pRotate > (3.14 *2))
		{
			pRotate = 0;
		}
		//拡大処理
		if (CheckHitKey(KEY_INPUT_Q))
		{
			pScale += 0.1f;
		}
		else//面倒なのでボタン押してないとすぐ戻る
		{
			pScale = 1;
		}

		/*Bullet処理*/
		//弾の発射
		if (CheckHitKey(KEY_INPUT_RETURN) && isBullet == false)//弾は一発しか発射できない
		{
			isBullet = true;
		}
		//弾の移動
		if (isBullet)
		{
			bPosX += bSpeed;
		}
		//弾の位置初期化
		if (bPosX > 1280)//画面外に出たら
		{
			bPosX = pPosX;
			bPosY = pPosY;
			isBullet = false;
		}

		/*Enemy処理*/
		//円の当たり判定
		//『弾の中心からエネミーの中心までの距離』<『弾の半径 + エネミーの半径』なら、当たっていると言える。
		//本来なら √(弾の座標 - エネミーの座標)^2 < 弾の半径+エネミーの半径 を比較することになるが、
		//ルートの計算はプログラミング的には重く、誤差も出やすい。そこで両辺を2乗して簡単にする。
		//(弾の座標 - エネミーの座標)^2 < (弾の半径+エネミーの半径)^2　を比較することで判定する。
		//三平方の定理より、(弾の座標 - エネミーの座標)^2 = (弾のx座標 - エネミーのx座標)^2 + (弾のy座標 - エネミーのy座標)^2
		int delBulletToEnemyX = bPosX - ePosX;//Xの座標の差
		int delBulletToEnemyY = bPosY - ePosY;//Yの座標の差
		int delBulletToEnemyMag = delBulletToEnemyX * delBulletToEnemyX + delBulletToEnemyY * delBulletToEnemyY;
		int delRMag = (bR + eR) * (bR + eR);
		//当たり判定の計算
		if (delBulletToEnemyMag < delRMag)
		{
			isHit = true;
		}
		else
		{
			isHit = false;
		}


		/*Draw*/
		//キャラクタ描画
		//前回まで使っていたDrawGraphだと指定座標が読み込んだ画像の左上になってしまう。
		//それを防ぐために真ん中を表示するこちらを使用
		//こちらの関数は拡大率と回転角度なども指定できる。
		DrawRotaGraph(pPosX,pPosY,pScale,pRotate,drawHandle,1);
		//弾の描画
		if (isBullet)
		{
			DrawCircle(bPosX, bPosY, bR, GetColor(255, 255, 255), 1);
		}
		//エネミー描画
		if (isHit)//弾が当たっていたら色が変わる
		{
			DrawCircle(ePosX, ePosY, eR, GetColor(0, 0, 255), 1);
		}
		else
		{
			DrawCircle(ePosX, ePosY, eR, GetColor(255, 0, 255), 1);
		}



		/*DebugDraw*/
		DrawString(0,0,"操作説明:WASD(上左下右),R(回転),Q(拡大),Enter(発射)",GetColor(255,0,0));



		//裏画面を表へ
		ScreenFlip();

		//裏画面を表へ(ダブルバッファリング3)
		ScreenFlip();
		/*リフレッシュ処理(-1ならエラー)*/
		if (ProcessMessage() < 0) { break; }
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//終了 
}
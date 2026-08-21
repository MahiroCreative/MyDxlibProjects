#include "DxLib.h"
#include <math.h>

/*概要*/
//名称:DxlibGameTemplate1
//C言語の『配列/ポインタ/構造体』を使って組んだテンプレートです。
// 非常に簡易的ですが
// ・シーン遷移(グローバル変数とif文で実装)
// ・インプットシステム(構造体を用いて多少スマートに)
// ・円の当たり判定(ベクトル成分のみ構造体で実装)
// などを備えています。
// 作成するゲームの下地として使うよりは『読んで理解できるか？』で学習進度を確認してください。
// 教科書やDxlibのリファレンスを見ながらの理解で構いません。
// 分からない学生はCの学習をやり直してください。

/*構造*/
//『配列/ポインタ/構造体』を使用することで、
// 前回まで無理やりグローバル変数などでやっていたことを多少効率化しています。
// 【例：】
// ・シーン管理をポインタでちょっとだけスマートに
// ・弾丸など複数存在するデータを配列で一括管理
// ・エネミー/プレイヤー/弾丸のパラメータデータを構造体で１つに纏める
// ・ポンタはシーン管理に利用(返り値でのやり方と効率は変わらない)
// ・Input系はポインタと構造体を使用している。
// ・ポインタの使用で多重ループ構造の解消
// シーンの切り替えは前回同様実行メソッドを切り替える事でシーン管理をしています。
// また、シーン管理には列挙型を用いています。
//【ループ構造改善：】
//　前回までの構造だとシーン毎にゲームループが存在し、Mainループ中にシーンループがあり多重ループが発生している。
//　多重ループ構造はなるべく改善すべきである上に、ループの中にループある環境によってfpsが安定しない。
//　そのためループ構造の解消を行うためにオブジェクト座標系のポインタの乱用が発生している。

/*ゲームの仕様*/
//・タイトル画面
//	-START : ゲームシーンに移動
//  -RANK: ランキングシーンに移動
//	-END : ゲーム終了
//・ゲームシーン
// - Player :WASDで動く。マウス左クリックで弾を発射する。エネミーの弾が当たるとタイトルに戻る。弾は連射できる。
// - Enemy:上下に動く。一定間隔で弾を発射。撃つたびに弾の速度が早くなる。プレイヤーの弾が当たるとスコアが上昇する。弾が当たってる間は色が変わる。弾丸を一度に複数はなてる。
// - Score:右上に表示。現在のスコアが表示される。
//・ランキングシーン
// - ゲームを起動している間のMAXSCOREが表示される。ゲームを終了すると値は0に戻る。

/*注意*/
//fpsの差による挙動の違いは考慮していません。
//環境により、オブジェクトの速度などが変わってしまいます。

/*グローバル定数*/
enum SceneKind//シーン管理用
{
	GAMEEND,
	TITLESCENE,
	GAMESCENE,
	RANKINGSCENE
};
constexpr int MAXPLAYERBULLET = 3;
constexpr int MAXENEMYBULLET = 8;

/*グローバル変数*/
int MaxScore = 0;
struct IsKeyInput//Keyフラグ管理
{
	bool isInputEnterHold = false;//Enter用の変数
	bool isInputUpHold = false;//Up用の変数
	bool isInputDownHold = false;//Down用の変数
};

/*プロトタイプ宣言*/
//Scene.
void TitleScene(SceneKind* _nextScene);
void GameScene(SceneKind* _nextScene);
void RankingScene(SceneKind* _nextScene);
//KeyInput.
bool InputEnter(IsKeyInput* _isKey);
bool InputUp(IsKeyInput* _isKey);
bool InputDown(IsKeyInput* _isKey);

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*定数*/
	//画面サイズ
	constexpr int ScreenSizeX = 1280;//幅
	constexpr int ScreenSizeY = 720;//高さ

	/*変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(ScreenSizeX, ScreenSizeY, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*シーン管理用*/
	SceneKind nextScene;
	//初期化
	nextScene = SceneKind::TITLESCENE;

	/*シーンループ部*/
	//gameRoop.
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		//裏画面の初期化
		ClearDrawScreen();

		/*シーン遷移処理*/
		if (nextScene == SceneKind::TITLESCENE)
		{
			TitleScene(&nextScene);
		}
		else if (nextScene == SceneKind::GAMESCENE)
		{
			GameScene(&nextScene);
		}
		else if (nextScene == SceneKind::RANKINGSCENE)
		{
			RankingScene(&nextScene);
		}
		else if (nextScene == SceneKind::GAMEEND)
		{
			break;
		}

		//裏画面を表へ
		ScreenFlip();

		//リフレッシュ処理(-1ならエラー)
		if (ProcessMessage() < 0) { break; }

		//ループ終了処理
		if (CheckHitKey(KEY_INPUT_ESCAPE)) { break; }

		//fps固定(60fps:16.66ms)
		//ループ開始時刻から16.66ms経つまで停止
		while (GetNowHiPerformanceCount() - roopStartTime < 16667) {}
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//終了 
}

/*シーン関数*/
void TitleScene(SceneKind* _nextScene)
{
	/*変数*/
	bool _gameRoop = true;
	int _arrowPosY = 440;
	int _countFrame = 0;
	IsKeyInput _isKeyInput;

	/*GameRoop*/
	while (_gameRoop)
	{
		/*オブジェクト操作*/
		//Input Down.
		if (InputDown(&_isKeyInput))
		{
			if (_arrowPosY == 440)
			{
				_arrowPosY = 480;
			}
			else
			{
				_arrowPosY = 440;
			}
		}
		//Input Up.
		if (InputUp(&_isKeyInput))
		{
			if (_arrowPosY == 440)
			{
				_arrowPosY = 480;
			}
			else
			{
				_arrowPosY = 440;
			}
		}

		/*シーン遷移処理*/
		//エンターでシーン変更
		if (InputEnter(&_isKeyInput))
		{
			if (_arrowPosY == 440)
			{
				*_nextScene = SceneKind::GAMESCENE;
				break;
			}
			else
			{
				*_nextScene = SceneKind::GAMEEND;
				break;
			}

		}

		/*タイマ更新*/
		_countFrame++;
		if (_countFrame > 80000) { _countFrame = 0; }//一定以上数が増えたら初期化(数は適当)

		/*Draw*/
		//裏画面の初期化
		ClearDrawScreen();

		//タイトルロゴ
		SetFontSize(80);//フォントサイズ変更
		DrawString(440, 240, "DxlibGame", GetColor(255, 255, 255));
		SetFontSize(40);//フォントサイズ上
		DrawString(460, 320, "-GameTemplate2-", GetColor(255, 255, 255));
		SetFontSize(20);//フォントサイズ初期化
		//ゲームシーンテキスト
		DrawString(600, 440, "START", GetColor(255, 255, 255));
		//ゲームエンドテキスト
		DrawString(600, 480, "END", GetColor(255, 255, 255));
		//矢印表示(点滅させる)
		if ((_countFrame % 80) < 40)
		{
			DrawString(560, _arrowPosY, "->", GetColor(255, 255, 255));
		}

		/*DebugDraw*/
		//シーン名表示
		DrawString(0, 0, "TitleScene", GetColor(255, 255, 255));

		//裏画面を表へ
		ScreenFlip();
	}

}
void GameScene(SceneKind* _nextScene)
{
	/*構造体の作成*/
	//Player
	struct Player { int X = 0; int Y = 0; int R = 0; };
	//Enemy
	struct Enemy { int X = 0; int Y = 0; int R = 0; };
	//EnemyBullet
	struct PlayerBullet { int X = 0; int Y = 0; int R = 0; };
	//PlayerBullet
	struct EnemyBullet { int X = 0; int Y = 0; int R = 0; };

	/*変数の作成*/
	bool _gameRoop = true;
	int _countFrame = 0;
	int _playerSpeed = 3;
	int _playerHandle = LoadGraph("Chara.png");
	IsKeyInput _isKeyInput;
	Player _player = {20,360,8};
	Enemy _enemy = {1100,360,64};

	/*GameRoop*/
	while (_gameRoop)
	{
		//裏画面の初期化
		ClearDrawScreen();

		/*player動作*/
		_player.Y -= CheckHitKey(KEY_INPUT_W) * _playerSpeed;//up.
		_player.Y += CheckHitKey(KEY_INPUT_S) * _playerSpeed;//down.
		_player.X += CheckHitKey(KEY_INPUT_D) * _playerSpeed;//right.
		_player.X -= CheckHitKey(KEY_INPUT_A) * _playerSpeed;//left.

		/*Draw*/
		//player
		DrawRotaGraph(_player.X, _player.Y, 1, 0, _playerHandle, true);
		//enemy
		DrawCircle(_enemy.X, _enemy.Y, _enemy.R, GetColor(0, 0, 255), 1);

		/*DebugDraw*/
		//シーン名表示
		DrawString(0, 0, "GameScene", GetColor(255, 255, 255));

		/*シーン遷移*/
		if (InputEnter(&_isKeyInput))
		{
			*_nextScene = SceneKind::TITLESCENE;
			break;
		}



		//裏画面を表へ
		ScreenFlip();
	}


}
void RankingScene(SceneKind* _nextScene)
{

}

/*Input関数*/
//Enterが押されたかどうかを判定する関数
bool InputEnter(IsKeyInput* _isKey)
{
	if (CheckHitKey(KEY_INPUT_RETURN) && !_isKey->isInputEnterHold)
	{
		_isKey->isInputEnterHold = true;
		return true;
	}
	else if (!CheckHitKey(KEY_INPUT_RETURN))
	{
		_isKey->isInputEnterHold = false;
	}

	return false;
}
//Upが押されたかどうかを判定する関数
bool InputUp(IsKeyInput* _isKey)
{
	if (CheckHitKey(KEY_INPUT_UP) && !_isKey->isInputDownHold)
	{
		_isKey->isInputDownHold = true;
		return true;
	}
	else if (!CheckHitKey(KEY_INPUT_UP))
	{
		_isKey->isInputDownHold = false;
	}

	return false;
}
//Downが押されたかどうかを判定する関数
bool InputDown(IsKeyInput* _isKey)
{
	if (CheckHitKey(KEY_INPUT_DOWN) && !_isKey->isInputUpHold)
	{
		_isKey->isInputUpHold = true;
		return true;
	}
	else if (!CheckHitKey(KEY_INPUT_DOWN))
	{
		_isKey->isInputUpHold = false;
	}

	return false;
}
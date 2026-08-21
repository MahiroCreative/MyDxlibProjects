#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"
#include "ShootingPlayer.h"
#include "ShootingEnemy.h"

class ShootingScene : public _baseGameScene
{
public:
	/*コンストラクタデストラクタ*/ 
	//コンストラクタ
	ShootingScene()
		: _nextScene(), _playerBulletTimer(0), _playerChargeFrame(0),
		_player(nullptr), _enemy(nullptr), _pPlayerBullet(nullptr),_vPlayerBullets(),
		_pEnemyBullet(nullptr), _vEnemyBullets()
	{
		Init();
	}
	//デストラクタ
	~ShootingScene() = default;

	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//更新
	int Update() override;
	//描画
	void Draw() override;

	/*メンバ関数*/
	//任意のキーでタイトルに戻る
	void CheckReturnTitle(int KeyCode);
	//PlayerBulletの描画
	void DrawPlayerBullet();
	//PlayreBulletの生成
	void CreatePlayerBullet();
	//PlayerBulletの更新
	void UpdatePlayerBullet();
	//PlayerBulletの削除
	void DeletePlayerBullet();
	//EnemyBulletの描画
	void DrawEnemyBullet();
	//EnemyBulletの生成
	void CreateEnemyBullet();
	//EnemyBulletの更新
	void UpdateEnemyBullet();
	//EnemyBulletの削除
	void DeleteEnemyBullet();
	//PlayerとEnemyBulletの当たり判定
	void CheckPlayerCollision();
	//EnemyとPlayerBulletの当たり判定
	void CheckEnemyCollision();

	/*Debug用の関数*/


private:
	/*処理変数*/
	//nextScene.
	GameSetting::SceneState _nextScene;
	//PlayerBullet生成用のタイマー
	int _playerBulletTimer;
	int _playerChargeFrame;

	/*ゲームオブジェクト変数*/
	// 型エイリアスを定義
	using PlayerPtr = std::unique_ptr<ShootingPlayer>;
	using EnemyPtr = std::unique_ptr<ShootingEnemy>;
	using BulletPtr = std::unique_ptr<SimpleBullet>;
	using BulletVector = std::vector<BulletPtr>;
	//オブジェクト変数の作成
	PlayerPtr _player;
	EnemyPtr _enemy;
	// 単一のBulletオブジェクト変数
	BulletPtr _pPlayerBullet,_pEnemyBullet;
	// Bulletオブジェクト用のベクター
	BulletVector _vPlayerBullets, _vEnemyBullets;
};
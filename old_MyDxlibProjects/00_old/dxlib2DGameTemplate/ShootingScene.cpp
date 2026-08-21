#include "ShootingScene.h"

namespace
{
	//ショットダメージ
	constexpr int _kShotDamage = 1;
	//チャージショットダメージ
	constexpr int _kChargeShotDamage = 3;
}

/*メンバ関数*/
void ShootingScene::Init()
{
	//変数初期化
	_nextScene = GameSetting::SceneState::ShootingGame;
	_playerBulletTimer = 0;
	_playerChargeFrame = 0;

	//ゲームオブジェクト生成
	_player = std::make_unique<ShootingPlayer>();
	_enemy = std::make_unique<ShootingEnemy>();

	//初期化
	_player->Init(Vector2{ 100,GameSetting::WINDOW_CENTER_Y });
	_enemy->Init(Vector2{ GameSetting::WINDOW_WIDTH - 80 ,GameSetting::WINDOW_CENTER_Y });
}

int ShootingScene::Update()
{
	/*更新*/
	//Key入力の更新
	InputKey::Update();
	//player更新
	_player->Update();
	//enemy更新
	_enemy->Update();
	//playerbullet更新
	UpdatePlayerBullet();
	//enemybullet更新
	UpdateEnemyBullet();

	/*オブジェクト生成*/
	//PlayerBulletの生成(ChargeBullet含む)
	CreatePlayerBullet();
	//EnemyBulletの生成
	CreateEnemyBullet();

	/*オブジェクト削除*/
	//PlayerBulletの削除(ChargeBullet含む)
	DeletePlayerBullet();
	//EnemyBulletの削除
	DeleteEnemyBullet();

	/*当たり判定*/
	//PlayerとEnemyBulletの当たり判定
	CheckPlayerCollision();
	CheckEnemyCollision();

	/*return.*/
	//・PlayerのHPが0
	//・EnemyのHPが0
	//・Bボタンが押される
	// 上記いずれかでタイトルに戻る
	CheckReturnTitle(KEY_INPUT_B);
	//int型に変換して次のシーンを返す
	return static_cast<int>(_nextScene);
}

void ShootingScene::Draw()
{
	//現在のシーン名を表示
	DrawString(0, 0, "ShootingGame: WASD(上左下右),Enter(Shot),B(タイトル)", GetColor(255, 255, 255));
	//PlayerBullet描画
	DrawPlayerBullet();
	//EnemyBullet描画
	DrawEnemyBullet();
	//player描画
	_player->Draw();
	//enemy描画
	_enemy->Draw();

}

void ShootingScene::CheckReturnTitle(int KeyCode)
{
	//PlayerのHPが0以下になったらゲームオーバー
	if (_player->GetHp()<=0) { _nextScene = GameSetting::SceneState::OverGame; }

	//EnemyのHPが0以下になったらクリアー
	if (_enemy->GetHp() <= 0) { _nextScene = GameSetting::SceneState::ClearGame; }

	//Bボタンが押されたらタイトルに戻る
	if (InputKey::isDownKey(KeyCode)){_nextScene = GameSetting::SceneState::Title;}
}

void ShootingScene::DrawPlayerBullet()
{
	for (auto& bullet : _vPlayerBullets)
	{
		bullet->Draw();
	}
}

void ShootingScene::CreatePlayerBullet()
{
	//PlayerBulletの生成
	if (_player->IsShot())
	{
		//Bulletの生成
		_pPlayerBullet = std::make_unique<SimpleBullet>();
		//初期化
		_pPlayerBullet->Init(_player->GetTransform().Position, _player->GetShotSpeed(), _player->GetShotSize(), "playerShot", Color::OrangeColor);
		//Bulletの追加
		_vPlayerBullets.push_back(std::move(_pPlayerBullet));
	}

	//ChargeBulletの生成
	if (_player->IsChargeShot())
	{
		//ChargeBulletの生成
		_pPlayerBullet = std::make_unique<SimpleBullet>();
		//初期化
		_pPlayerBullet->Init(_player->GetTransform().Position, _player->GetChargeShotSpeed(), _player->GetChargeShotSize(), "playerCharge", Color::OrangeColor);
		//ChargeBulletの追加
		_vPlayerBullets.push_back(std::move(_pPlayerBullet));
	}
}

void ShootingScene::UpdatePlayerBullet()
{
	//PlayerBulletの更新
	for (auto& bullet : _vPlayerBullets)
	{
		bullet->Update();
	}
}

void ShootingScene::DeletePlayerBullet()
{
	// 全弾チェック
	for (auto it = _vPlayerBullets.begin(); it != _vPlayerBullets.end();)
	{
		if ((*it)->IsOutOfScreen())
		{
			// 画面外に出た弾を削除
			//(削除した場合、空いた場所に後ろの要素が詰められる)
			it = _vPlayerBullets.erase(it);
		}
		else
		{
			// 次の要素へ
			++it;
		}
	}
}

void ShootingScene::DrawEnemyBullet()
{
	for (auto& bullet : _vEnemyBullets)
	{
		bullet->Draw();
	}
}

void ShootingScene::CreateEnemyBullet()
{
	//EnemyBulletの生成
	if (_enemy->IsShot())
	{
		//Bulletの生成
		_pEnemyBullet = std::make_unique<SimpleBullet>();
		//初期化
		_pEnemyBullet->Init(_enemy->GetTransform().Position, _enemy->GetShotSpeed(), _enemy->GetShotSize(), "enemyShot", Color::GreenColor);
		//Bulletの追加
		_vEnemyBullets.push_back(std::move(_pEnemyBullet));
	}

	//ChargeBulletの生成
	if (_enemy->IsChargeShot())
	{
		//ChargeBulletの生成
		_pEnemyBullet = std::make_unique<SimpleBullet>();
		//初期化
		_pEnemyBullet->Init(_enemy->GetTransform().Position, _enemy->GetChargeShotSpeed(), _enemy->GetChargeShotSize(), "enemyCharge", Color::GreenColor);
		//ChargeBulletの追加
		_vEnemyBullets.push_back(std::move(_pEnemyBullet));
	}
}

void ShootingScene::UpdateEnemyBullet()
{
	//EnemyBulletの更新
	for (auto& bullet : _vEnemyBullets)
	{
		bullet->Update();
	}
}

void ShootingScene::DeleteEnemyBullet()
{
	// 全弾チェック
	for (auto it = _vEnemyBullets.begin(); it != _vEnemyBullets.end();)
	{
		if ((*it)->IsOutOfScreen())
		{
			// 画面外に出た弾を削除
			//(削除した場合、空いた場所に後ろの要素が詰められる)
			it = _vEnemyBullets.erase(it);
		}
		else
		{
			// 次の要素へ
			++it;
		}
	}
}

void ShootingScene::CheckPlayerCollision()
{
	for (auto& bullet : _vEnemyBullets)
	{
		//コリジョン取得
		CircleCollision2D bulletCol = bullet->GetCollision();
		CircleCollision2D playerCol = _player->GetCollision();
		std::string bulletTag = bullet->GetTag();

		//PlayerとEnemyBulletの当たり判定
		if (playerCol.IsCollision(bulletCol))
		{
			//PlayerのHPを減らす
			if (bulletTag == "enemyCharge")
			{
				_player->Damage(_kChargeShotDamage);
			}
			else
			{
				_player->Damage(_kShotDamage);
			}

		}
	}
}

void ShootingScene::CheckEnemyCollision()
{
	for (auto& bullet : _vPlayerBullets)
	{
		//コリジョン取得
		CircleCollision2D bulletCol = bullet->GetCollision();
		CircleCollision2D enemyCol = _enemy->GetCollision();
		std::string bulletTag = bullet->GetTag();

		//PlayerとEnemyBulletの当たり判定
		if (enemyCol.IsCollision(bulletCol))
		{
			//EnemyのHPを減らす
			if (bulletTag == "playerCharge")
			{
				_enemy->Damage(_player->GetChargeShotDamage());
			}
			else
			{
				_enemy->Damage(_player->GetShotDamage());
			}

		}
	}
}



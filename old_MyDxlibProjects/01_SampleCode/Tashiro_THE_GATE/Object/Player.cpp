#include "Player.h"
#include "Game.h"
#include "File.h"
#include "MathHelp.h"
#include "Easing.h"
#include "Input.h"
#include "CommandList.h"
#include "Application.h"
#include "SoundManager.h"
#include "AnimController.h"
#include "PlayerCamera.h"
#include "Physics.h"
#include "Collider/ColliderSphere.h"
#include "Collider/ColliderCapsule.h"
#include "Gate/GateManager.h"
#include "Gate/GateBullet.h"
#include "Gate/Gate.h"
#include "Object/HandObject.h"
#include "DebugDraw.h"
#include "ObjectTag.h"
#include "StringUtility.h"

namespace
{
	enum class ColIndex
	{
		CENTER,
		MODEL,
	};

	// ファイルID
	const wchar_t* const FILE_PALYER = L"M_Player";
	const wchar_t* const FILE_HP_FRAME = L"I_HpFrame";
	const wchar_t* const FILE_BASE_HP_BAR = L"I_BaseHpBar";
	const wchar_t* const FILE_NOW_HP_BAR = L"I_NowHpBar";
	const wchar_t* const FILE_HIT_HP_BAR = L"I_HitHpBar";
	const wchar_t* const FILE_LANDING = L"S_Landing";
	const wchar_t* const FILE_WALK = L"S_Walk";
	const wchar_t* const FILE_SHOT = L"S_PlayerShot";

	// アニメーション関連
	const wchar_t* const ANIM_INFO_PATH = L"Data/Master/AnimPlayerMaster.csv";
	const wchar_t* const ANIM_IDLE = L"Idle";
	const wchar_t* const ANIM_WALK = L"Walk";
	const wchar_t* const ANIM_JUMP = L"Jump";
	const wchar_t* const ANIM_AERIAL = L"Aerial";
	const wchar_t* const ANIM_LANDING = L"Landing";
	const wchar_t* const ANIM_DEATH = L"Death";

	// 画像サイズ
	constexpr float FILE_SIZE = 0.35f;
	// HPフレームの描画位置
	constexpr int DRAW_HP_FRME_X = 157;
	constexpr int DRAW_HP_FRME_Y = 70;
	// HPバーの描画位置
	constexpr int DRAW_HP_BAR_X = 157 + 36;
	constexpr int DRAW_HP_BAR_Y = 70;
	// 揺らす大きさ
	constexpr int SHAKE_SIZE = 5;

	// FIXME: ここらへんの情報は一部CSVで管理するようにするかも
	constexpr float HAND_RADIUS = 2.0f;	// 物を持てる範囲
	constexpr float HAND_DIS = 4.0f;	// 物を持つ当たり判定の中心との距離

	// モデルのファイルサイズ
	constexpr float MODEL_SIZE_SCALE = 0.0325f;

	/* 判定系 */
	constexpr float MODEL_COL_RADIUS = 1.0f;	// モデルカプセル半径
	constexpr float MODEL_COL_SIZE = 3.5f;		// モデルカプセルサイズ
	constexpr float CENTER_COL_RADIUS = 0.5f;	// 中心カプセル半径
	constexpr float CENTER_COL_SIZE = 4.25f;	// 中心カプセルサイズ
	/* 力系 */
	constexpr float LANDING_SPEED = 0.3f;	// 地面での移動速度
	constexpr float AERIAL_SPEED = 0.15f;	// 空中での移動速度
	constexpr float JUMP_POWER = 0.55f;	// ジャンプ力
	// 撃つ間隔
	constexpr int SHOT_INTERVAL = 60;	
	// 最大HP
	constexpr int MAX_HP = 100;	
	// HP回復までの待機フレーム
	constexpr int RECEVER_WAIT_FRAME = 120;	

	// ピポット
	const Vec3 LOOK_PIVOT = Vec3(0, MODEL_COL_SIZE * 0.5f, 0);	// 見る場所
	const Vec3 MODEL_PIVOT = Vec3(0, -MODEL_COL_RADIUS - MODEL_COL_SIZE * 0.5f, 0);	// モデル場所

	// HPバーを揺らすフレーム
	constexpr int SHKE_HP_BAR_FRAME = 30;
	// 減少HPを減らし始めるまでのフレーム
	constexpr int START_DECREASE_PRE_HP_BAR_FRAME = 10;
}

Player::Player(const std::shared_ptr<PlayerCamera>& camera, const std::shared_ptr<GateManager>& gateMgr) :
	Object3DBase(Priority::HIGH, ObjectTag::PALYER),
	m_updateFunc(&Player::IdleUpdate),
	m_camera(camera),
	m_gateMgr(gateMgr),
	m_handObj(nullptr),
	m_nowState(State::IDLE),
	m_hp(MAX_HP),
	m_preHp(MAX_HP),
	m_receverFrame(0),
	m_shotInteval(0),
	m_shakeHpBarFrame(-1),
	m_isOneHand(false),
	m_isDeath(false),
	m_isRecever(false),
	m_isGround(false),
	m_isCatch(false),
	m_isWarp(false)
{
}

Player::~Player()
{
}

void Player::AsyncInit()
{
	// ファイル読み込み
	LoadModel(FILE_PALYER);
	auto& fileMgr = FileManager::GetInstance();
	m_files[FILE_HP_FRAME] = fileMgr.Load(FILE_HP_FRAME);
	m_files[FILE_BASE_HP_BAR] = fileMgr.Load(FILE_BASE_HP_BAR);
	m_files[FILE_NOW_HP_BAR] = fileMgr.Load(FILE_NOW_HP_BAR);
	m_files[FILE_HIT_HP_BAR] = fileMgr.Load(FILE_HIT_HP_BAR);
	m_files[FILE_LANDING] = fileMgr.Load(FILE_LANDING);
	m_files[FILE_WALK] = fileMgr.Load(FILE_WALK);
	m_files[FILE_SHOT] = fileMgr.Load(FILE_SHOT);
}

void Player::Init(const Vec3& pos, bool isOneHand)
{
	m_anim = std::make_shared<AnimController>();
	m_anim->Init(ANIM_INFO_PATH, m_modelHandle, ANIM_IDLE);

	OnEntryPhysics();
	m_rigid.SetPos(pos + Vec3(0, 1, 0));
	m_updateFunc = &Player::IdleUpdate;
	m_pivot = MODEL_PIVOT;
	m_scale *= MODEL_SIZE_SCALE;

	auto col = std::dynamic_pointer_cast<MyEngine::ColliderCapsule>(CreateCollider(MyEngine::ColKind::CAPSULE));
	col->isTrigger = true;
	col->radius = CENTER_COL_RADIUS;
	col->size = CENTER_COL_SIZE;
	col->dir = Vec3::Up();
	auto col2 = std::dynamic_pointer_cast<MyEngine::ColliderCapsule>(CreateCollider(MyEngine::ColKind::CAPSULE));
	col2->radius = MODEL_COL_RADIUS;
	col2->size = MODEL_COL_SIZE;
	col2->dir = Vec3::Up();

	m_handCol = std::make_shared<MyEngine::ColliderSphere>();
	m_handCol->radius = HAND_RADIUS;
	m_handCol->isTrigger = true;

	m_isOneHand = isOneHand;

	m_hp = MAX_HP;
	m_preHp = MAX_HP;

	m_shakeHpBarFrame = -1;
}

void Player::Restart(const Vec3& pos)
{
	// アニメーション初期化
	m_anim->Change(ANIM_IDLE, true, false, false);

	// 位置変更
	m_rigid.SetPos(pos);

	// ものを持っていれば離す
	if (m_isCatch)
	{
		m_handObj->EndHand();
		m_handCol = nullptr;
		m_isCatch = false;
	}

	// 体力初期化
	m_hp = MAX_HP;
	m_preHp = MAX_HP;

	m_shakeHpBarFrame = -1;

	// 更新先変更
	m_updateFunc = &Player::IdleUpdate;
	// 死亡していないことに
	m_isDeath = false;
}

void Player::Update()
{
	// 死んでいない場合更新
	if (!m_isDeath)
	{
		--m_shotInteval;
		HandUpdate();
		RecoverHpUpdate();
	}
	HpBarUpdate();
	(this->*m_updateFunc)();
	// トリガーが入力されたら発射
	const auto& trigger = Input::GetInstance().GetTriggerData();
	if (trigger.LT > 0.0f || trigger.RT > 0.0f)
	{
		OnShot();
	}
	AnimUpdate();
	m_rotation = Easing::Slerp(m_rotation, m_nextRot, 0.2f);

	m_camera->Update(m_rigid.GetPos() + LOOK_PIVOT);
#ifdef _DEBUG
	const auto& pos = m_rigid.GetPos();
	const auto& vel = m_rigid.GetVelocity();
	printf("PlayerPos:(%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
	printf("PlayerVel:(%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
	for (auto& item : m_throughTag)
	{
		printf("%s\n", Tags::TAG_NAME.at(item));
	}
#endif
}

void Player::Draw() const
{
	// FIXME: モデルの描画が邪魔だから一時的に消している
	Object3DBase::Draw();
}

void Player::DrawHpUI() const
{
	// 画像サイズ取得
	int w, h;
	GetGraphSize(m_files.at(FILE_BASE_HP_BAR)->GetHandle(), &w, &h);
	// 大きさ変更
	w = static_cast<int>(w * FILE_SIZE);
	h = static_cast<int>(h * FILE_SIZE);

	int shakeX = 0;
	int shakeY = 0;
	// バーを揺らす場合
	if (m_shakeHpBarFrame > 0)
	{
		// サイズ調整
		int size = static_cast<int>((static_cast<float>(m_shakeHpBarFrame) / SHKE_HP_BAR_FRAME) * SHAKE_SIZE);
		// 揺らす範囲をランダムで取得
		auto& app = Application::GetInstance();
		shakeX = app.GetRand(0, size);
		shakeY = app.GetRand(0, size);
	}

	// フレームの描画
	DrawRotaGraphFast(DRAW_HP_FRME_X + shakeX, DRAW_HP_FRME_Y + shakeY, FILE_SIZE, 0.0f, m_files.at(FILE_HP_FRAME)->GetHandle(), true);
	// ベースとなるHPバーを描画
	DrawRotaGraphFast(DRAW_HP_BAR_X + shakeX, DRAW_HP_BAR_Y + shakeY, FILE_SIZE, 0.0f, m_files.at(FILE_BASE_HP_BAR)->GetHandle(), true);
	// 下二つのバーの描画位置を取得
	int x = DRAW_HP_BAR_X - static_cast<int>(w * 0.5f) + shakeX;
	int y = DRAW_HP_BAR_Y - static_cast<int>(h * 0.5f) + shakeY;
	// 減少HPの割合を取得
	float rate = m_preHp / static_cast<float>(MAX_HP);
	// 減少HPバーを割合を考慮して描画
	DrawExtendGraph(x, y, x + static_cast<int>(w * rate), y + h, m_files.at(FILE_HIT_HP_BAR)->GetHandle(), true);
	// 現在HPの割合を取得
	rate = m_hp / static_cast<float>(MAX_HP);
	// 現在HPバーを割合を考慮して描画
	DrawExtendGraph(x, y, x + static_cast<int>(w * rate), y + h, m_files.at(FILE_NOW_HP_BAR)->GetHandle(), true);
}

void Player::OnDamage(int damage)
{
	// 減少HPが現在のHP未満なら代入する
	if (m_preHp < m_hp)
	{
		m_preHp = m_hp;
	}
	m_shakeHpBarFrame = SHKE_HP_BAR_FRAME;

	m_hp -= damage;
	if (m_hp <= 0)
	{
		OnDeath();
		m_hp = 0;
	}
	m_receverFrame = RECEVER_WAIT_FRAME;
	m_isRecever = true;
}

std::shared_ptr<Camera> Player::GetCamera() const
{
	return m_camera;
}

void Player::OnCollideStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnCollideStay(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	const auto& isFind = std::find(m_groundTag.begin(), m_groundTag.end(), tag) != m_groundTag.end();
	if (isFind)
	{
		OnLanding();
		if (tag == ObjectTag::FLOOR_MOVE)
		{
			auto vel = m_rigid.GetVelocity();
			vel.y = 0;
			m_rigid.SetVelocity(vel);
			auto pos = m_rigid.GetPos();
			pos += colider->GetVelocity();
			m_rigid.SetPos(pos);
		}
	}
}

void Player::OnTriggerStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerStay(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::GATE)
	{
		auto gate = dynamic_cast<Gate*>(colider);
		// 中心の判定の場合
		if (selfIndex == static_cast<int>(ColIndex::CENTER))
		{
			if (m_gateMgr->IsCreateBothGates() && !m_isAddTag[colider])
			{
				// スルータグ追加
				auto gate = dynamic_cast<Gate*>(colider);
				m_throughTag.emplace_back(gate->GetHitObjectTag());
				m_isAddTag[colider] = true;
			}
		}
		// モデルの判定の場合
		else if (selfIndex == static_cast<int>(ColIndex::MODEL))
		{
			auto pairGate = gate->GetPairGate();
			// ワープ判定
			if (gate->CheckWarp(m_rigid.GetPos()))
			{
				gate->OnWarp(m_rigid.GetPos(), m_rigid, true);

				m_camera->OnWarp(-gate->GetNorm(), pairGate->GetNorm(), m_rigid.GetPos());
				OnAerial();

				// スルータグの変更
				if (m_isAddTag[colider]) m_throughTag.pop_back();
				m_isAddTag[colider] = false;
				m_throughTag.push_back(pairGate->GetHitObjectTag());
				m_isAddTag[pairGate.get()] = true;
			}
		}
	}
}

void Player::OnTriggerExit(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerExit(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::GATE && selfIndex == static_cast<int>(ColIndex::CENTER))
	{
		if (m_isAddTag[colider])
		{
			// スルータグ外す
			auto gate = dynamic_cast<Gate*>(colider);
			auto hitTag = gate->GetHitObjectTag();
			for (auto it = m_throughTag.begin(); it != m_throughTag.end(); ++it)
			{
				if (*it == hitTag)
				{
					m_throughTag.erase(it);
					break;
				}
			}
			m_isAddTag[colider] = false;
		}
	}
}

void Player::HandUpdate()
{
	m_handCol->center = m_camera->GetInfo().look * HAND_DIS + LOOK_PIVOT;
	auto& phsyics = MyEngine::Physics::GetInstance();
	const auto& res = phsyics.GetHitObject(m_rigid.GetPos(), m_handCol.get(), ObjectTag::HAND_OBJ);
	if (!res.empty())
	{
		auto& input = Input::GetInstance();
		if (input.IsTriggerd(Command::BTN_HAND))
		{
			OnHand(res[0].col);
		}
	}

	if (m_isCatch)
	{
		m_handObj->SetPos(m_rigid.GetPos() + m_handCol->center);
		m_handObj->ZeroGravity();
	}
#ifdef _DEBUG
	auto& debug = MyEngine::DebugDraw::GetInstance();
	debug.DrawSphere(m_rigid.GetPos() + m_handCol->center, m_handCol->radius, 0x0000ff, false);
#endif
}

void Player::RecoverHpUpdate()
{
	// 回復しきっていたら処理なし
	if (!m_isRecever) return;

	// 回復待機時間のを減らす
	--m_receverFrame;
	// 回復待機中なら終了
	if (m_receverFrame > 0) return;

	// HP増加
	++m_hp;
	// 最大HPを超えたら修正して回復終了とする
	if (m_hp > MAX_HP)
	{
		m_hp = MAX_HP;
		m_isRecever = false;
	}
}

void Player::HpBarUpdate()
{
	// 揺らし時間内なら時間を減らしていく
	if (m_shakeHpBarFrame > -1)
	{
		--m_shakeHpBarFrame;
	}

	// 減少HPが現在のHPより大きく、揺らし時間が一定時間経っていれば減少HPを減らしていく
	if (m_preHp > m_hp && m_shakeHpBarFrame < SHKE_HP_BAR_FRAME - START_DECREASE_PRE_HP_BAR_FRAME)
	{
		--m_preHp;
		// HPより小さくなったらHPと同じに修正する
		if (m_preHp < m_hp)
		{
			m_preHp = m_hp;
		}
	}
}

void Player::AnimUpdate()
{
	if (m_nowState == State::MOVE)
	{
		auto sqSpeed = m_rigid.GetVelocity().SqLength();
		auto rate = (sqSpeed / (LANDING_SPEED * LANDING_SPEED)) * 0.7f + 0.3f;
		m_anim->Update(rate);
	}
	else
	{
		m_anim->Update();
	}
}

void Player::IdleUpdate()
{
	auto& input = Input::GetInstance();
	const auto& trigger = input.GetTriggerData();
	// 左スティックが入力されたら移動状態に遷移
	if (trigger.LStick.SqLength() > 0.0f)
	{
		OnWalk();
		return;
	}
	// ジャンプに遷移
	if (input.IsTriggerd(Command::BTN_JUMP))
	{
		OnJump();
		return;
	}
}

void Player::WalkUpdate()
{
	auto& input = Input::GetInstance();
	// ジャンプに遷移
	if (input.IsTriggerd(Command::BTN_JUMP))
	{
		OnJump();
		SoundManager::GetInstance().Stop(m_files.at(FILE_WALK)->GetHandle());
	}
	// 左スティックが入力されている間は移動
	if (Move(LANDING_SPEED)) return;

	// 入力されていなかったらアイドル状態に遷移
	OnIdle();
	SoundManager::GetInstance().Stop(m_files.at(FILE_WALK)->GetHandle());
}

void Player::JumpUpdate()
{
	if (m_anim->IsLoop())
	{
		OnAerial();
		return;
	}
}

void Player::AerialUpdate()
{
	Move(AERIAL_SPEED);
}

void Player::LandingUpdate()
{
	auto& input = Input::GetInstance();
	const auto& trigger = input.GetTriggerData();
	// 左スティックが入力されたら移動状態に遷移
	if (trigger.LStick.SqLength() > 0.0f)
	{
		OnWalk();
	}
	// 何もせずにアニメーションがループ完了したら待機状態に遷移
	else if (m_anim->IsLoop())
	{
		OnIdle();
	}
}

void Player::DeathUpdate()
{
	// 特になし
}

bool Player::Move(float speed)
{
	auto& input = Input::GetInstance();
	const auto& trigger = input.GetTriggerData();
	if (trigger.LStick.SqLength() > 0.0f)
	{
		// Y軸を無視したカメラの正面方向・右方向を取得
		auto& cInfo = m_camera->GetInfo();
		auto cameraFront = cInfo.front;
		cameraFront.y = 0;
		cameraFront.Normalize();
		// カメラの向いている方向に合わせて移動ベクトルを作成
		Vec3 newVel;
		newVel = cameraFront * trigger.LStick.y + cInfo.right * trigger.LStick.x;
		// 次の回転方向のベクトルを取得
		m_nextRot = Quaternion::GetQuaternion(Vec3::Back(), newVel);
		// 移動ベクトルを移動スピードの長さに変換して、y軸を現在の速度にする
		newVel *= AERIAL_SPEED;
		newVel.y = m_rigid.GetVelocity().y;
		m_rigid.SetVelocity(newVel);
		return true;
	}
	else
	{
		return false;
	}
}

void Player::OnIdle()
{
	// 速度ゼロ
	m_rigid.SetVelocity(Vec3());
	m_anim->Change(ANIM_IDLE);
	m_updateFunc = &Player::IdleUpdate;
	m_nowState = State::IDLE;
}

void Player::OnWalk()
{
	SoundManager::GetInstance().PlaySe3D(m_files.at(FILE_WALK)->GetHandle(), shared_from_this());
	m_anim->Change(ANIM_WALK);
	m_updateFunc = &Player::WalkUpdate;
	m_nowState = State::MOVE;
}

void Player::OnJump()
{
	if (!m_isGround) return;

	auto vel = m_rigid.GetVelocity();
	vel.y = JUMP_POWER;
	m_rigid.SetVelocity(vel);
	m_isGround = false;
	m_anim->Change(ANIM_JUMP, true, true);
	m_updateFunc = &Player::JumpUpdate;
	m_nowState = State::JUMP;
}

void Player::OnAerial()
{
	m_isGround = false;
	m_anim->Change(ANIM_AERIAL);
	m_updateFunc = &Player::AerialUpdate;
	m_nowState = State::AERIAL;
}

void Player::OnLanding()
{
	if (m_isGround) return;

	SoundManager::GetInstance().PlaySe3D(m_files.at(FILE_LANDING)->GetHandle(), shared_from_this());
	m_isGround = true;
	m_rigid.SetVelocity(Vec3());
	m_anim->Change(ANIM_LANDING, true, true);
	m_updateFunc = &Player::LandingUpdate;
	m_nowState = State::LANDING;
}

void Player::OnDeath()
{
	if (m_isDeath) return;

	m_isDeath = true;
	m_rigid.SetVelocity(Vec3());
	m_anim->Change(ANIM_DEATH, true, true);
	m_updateFunc = &Player::DeathUpdate;
	m_nowState = State::DEATH;
}

void Player::OnShot()
{
	// ショット待機時間なら撃たない
	if (m_shotInteval > 0) return;

	auto& input = Input::GetInstance();
	const auto& trigger = input.GetTriggerData();
	bool isCreate = false;
	GateKind kind;
	// 右トリガーが押されていたらオレンジゲートを撃つ
	if (trigger.RT > 0.0f)
	{
		isCreate = true;
		kind = GateKind::Orange;
	}
	// 片手ステージではないかつ左トリガーが押されていたらブルーゲートを撃つ
	else if (!m_isOneHand && trigger.LT > 0.0f)
	{
		isCreate = true;
		kind = GateKind::Blue;
	}
	// 弾の生成
	if (isCreate)
	{
		SoundManager::GetInstance().PlaySe3D(m_files.at(FILE_SHOT)->GetHandle(), shared_from_this());
		// インターバルを初期化
		m_shotInteval = SHOT_INTERVAL;
		// 生成
		auto bullet = std::make_shared<GateBullet>(m_gateMgr, kind);
		// カメラの向いている方向に弾を進ませる
		bullet->Init(m_rigid.GetPos(), m_camera->GetInfo().look);
		// 追加
		m_gateMgr->AddBullet(bullet);
	}
}

void Player::OnHand(MyEngine::Collidable* obj)
{
	if (m_isCatch)
	{
		m_handObj->EndHand();
		m_handObj = nullptr;
	}
	else
	{
		m_handObj = dynamic_cast<HandObject*>(obj);
		m_handObj->OnHnad();
	}

	m_isCatch = !m_isCatch;
}

#include "WarpGate.h"
#include"ColliderSphere.h"
#include"Player.h"
#include<EffekseerForDXLib.h>
#include"SoundManager.h"
#include"EffectManager.h"
#include"ScreenManager.h"

namespace
{
	const char* kGaussScreenName = "Gauss";
	const char* kColorScreenName = "Color";
	const char* kBrightScreenName = "HighBright";
}

Effekseer::Matrix43 GetEffMatrix(DxLib::MATRIX mat)
{
	Effekseer::Matrix43 res;
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			res.Value[y][x] = mat.m[y][x];
		}
	}
	return res;
}

namespace
{
	const char* name = "warpGate";
	const char* effectname = "warpEffect.efk";
}

WarpGate::WarpGate(Vec3 pos,Vec3 warpPos,int handle):Collidable(Priority::Static, ObjectTag::WarpGate),
m_emitterHandle(EffectManager::GetInstance().GetEffectData(effectname))
{
	SetAntiGravity(true);
	m_warpPos = warpPos;
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);//ここで入れたのは重力の影響範囲
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = 6;
	m_rigid->SetPos(pos);

	m_effectPlayHandle=PlayEffekseer3DEffect(m_emitterHandle);
	m_gaussScreenHandle = ScreenManager::GetInstance().GetScreenData(kGaussScreenName, 1600, 900);
	m_colorScreenHandle = ScreenManager::GetInstance().GetScreenData(kColorScreenName, 1600, 900);

	m_sideVec == Cross(m_upVec, (warpPos - m_rigid->GetPos()).GetNormalized());
}

WarpGate::~WarpGate()
{
	StopEffekseer3DEffect(m_effectPlayHandle);
	EffectManager::GetInstance().DeleteEffectData(effectname);
}

void WarpGate::Init()
{
}

void WarpGate::Update()
{
	
	
}

void WarpGate::SetEffectPos()
{
	m_upVec = (m_warpPos - m_rigid->GetPos()).GetNormalized();
	
	MATRIX mat ;

	Vec3 axis=Cross(Vec3::Up(), m_upVec);
	mat = MGetRotAxis(axis.VGet(), acos(Dot(Vec3::Up(), m_upVec)));

	mat = MMult(mat,MGetTranslate(Vec3(m_rigid->GetPos()+m_upVec*20).VGet()));
	auto effect = GetEffekseer3DManager();
	effect->SetBaseMatrix(m_effectPlayHandle,GetEffMatrix(mat));
}

void WarpGate::Draw()
{
	//m_screenHandle=GetDrawScreen();
	//SetDrawScreen(m_colorScreenHandle);
	//ClearDrawScreen();
	//DrawLine3D(m_rigid->GetPos().VGet(), ((m_warpPos - m_rigid->GetPos()).GetNormalized() * 300 + m_rigid->GetPos()).VGet(), 0xaaaaff);

	//GraphFilterBlt(m_colorScreenHandle, m_gaussScreenHandle, DX_GRAPH_FILTER_GAUSS, 16,900);
	//
	//SetDrawScreen(m_screenHandle);

	//// 描画ブレンドモードを加算にする
	//SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

	//
	//DrawExtendGraph(0, 0, 1600, 900, m_gaussScreenHandle,true);
	//DrawExtendGraph(0, 0, 1600, 900, m_gaussScreenHandle, true);

	//// 描画ブレンドモードをブレンド無しに戻す
	//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

	DrawSphere3D(m_rigid->GetPos().VGet(), 6, 7, 0xff00ff, 0xff00ff, false);
	DrawCube3D(Vec3(m_rigid->GetPos() + Vec3(6, 6, 6)).VGet(), Vec3(m_rigid->GetPos() + Vec3(-6, -6, -6)).VGet(), 0xffff00, 0xffff00, false);
}

void WarpGate::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (colider->GetTag() == ObjectTag::Stage)
	{
		m_nowPlanetPos = colider->GetRigidbody()->GetPos();
	}
	if (colider->GetTag() == ObjectTag::Player)
	{
		PlaySoundMem(SoundManager::GetInstance().GetSoundData("boost.mp3"), DX_PLAYTYPE_BACK);
		colider->GetRigidbody()->SetVelocity(Vec3(m_warpPos - colider->GetRigidbody()->GetPos()).GetNormalized() * 5);
		auto player = std::dynamic_pointer_cast<Player>(colider);
		player->m_playerUpdate = &Player::BoostUpdate;
		player->SetBoost(m_sideVec);
	}
}

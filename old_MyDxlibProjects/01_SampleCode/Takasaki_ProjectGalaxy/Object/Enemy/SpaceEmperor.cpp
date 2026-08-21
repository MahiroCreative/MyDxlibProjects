#include "SpaceEmperor.h"
#include"ModelManager.h"

#include"Pad.h"
namespace
{
	constexpr float kFrameParSecond = 60.0f;

	constexpr float kScaleMag = 0.3f;

	//アニメーション番号
	constexpr int kAnimIndexDynamicPose = 0;
	constexpr int kAnimIndexIdle = 1;
	constexpr int kAnimIndexAttack = 2;

	constexpr float kAnimFrameSpeed = 30.0f;//アニメーション進行速度

	//アニメーションの切り替えにかかるフレーム数
	constexpr float kAnimChangeFrame = 8.0f;
	constexpr float kAnimChangeRateSpeed = 1.0f / kAnimChangeFrame;

	const char* kSpaceEmperorFileName = "SpaceEmperor.mv1";
	constexpr float kArmExtensionSpeedMax = 3;


}

SpaceEmperor::SpaceEmperor(Vec3 pos) : Enemy(-1,Priority::Boss,ObjectTag::SpaceEmperor),
m_armExtensionDistance(0),
m_armExtensionSpeed(kArmExtensionSpeedMax),
m_animBlendRate(0),
m_currentAnimNo(-1),
m_prevAnimNo(0),
m_neckNowDir(Vec3(0,0,-1)),
m_isFindTarget(false),
m_armMoveDir(Vec3::Zero())
{
	m_rigid->SetPos(pos);
	m_modelHandle = ModelManager::GetInstance().GetModelData(kSpaceEmperorFileName);
	m_update = &SpaceEmperor::IntroUpdate;
	{
		AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
		auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
		item->radius = 6;

	}
	{
		AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine:: ColliderBase::ColideTag::Fist);
		m_armCol = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
		m_armCol->radius = 6;
	}
	MV1SetScale(m_modelHandle,Vec3(kScaleMag, kScaleMag, kScaleMag).VGet());
	ChangeAnim(kAnimIndexDynamicPose);

	m_frontVec = Vec3::Front();
	m_neckFrameIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Neck");

}

SpaceEmperor::~SpaceEmperor()
{
}

void SpaceEmperor::Init()
{
}

void SpaceEmperor::Update()
{
	(this->*m_update)();

	UpdateAnim(m_currentAnimNo);
	//変更前のアニメーション100%
	DxLib::MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimNo, 1.0f - m_animBlendRate);
	//変更後のアニメーション0%
	DxLib::MV1SetAttachAnimBlendRate(m_modelHandle, m_currentAnimNo, m_animBlendRate);
	m_animBlendRate += 0.05f;
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f;
	}

	m_neckPos = MV1GetFramePosition(m_modelHandle, m_neckFrameIndex);
}

void SpaceEmperor::SetMatrix()
{
	MV1SetPosition(m_modelHandle,m_rigid->GetPos().VGet());
}

void SpaceEmperor::Draw()
{
	MV1DrawModel(m_modelHandle);

	for (auto& col : m_colliders)
	{
		auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(col);
		DrawSphere3D((m_rigid->GetPos()+item->GetShift()).VGet(), item->radius, 20, 0xffff00, 0xffffff, false);
	}
}

void SpaceEmperor::OnBossPlanet()
{
	m_update = &SpaceEmperor::IntroUpdate; 
	m_isFindTarget = true; 
	ChangeAnim(kAnimIndexIdle);
}

void SpaceEmperor::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (m_update == &SpaceEmperor::HitUpdate)return;
	
	if (ownTag == MyEngine::ColliderBase::ColideTag::Fist)
	{
		if (colider->GetTag() == ObjectTag::Player)
		{
			m_hitDir = m_rigid->GetPos() - colider->GetRigidbody()->GetPos();
			m_hitDir.Normalize();
			ChangeAnim(kAnimIndexIdle);
			m_update = &SpaceEmperor::HitUpdate;

		}
		if (colider->GetTag() == ObjectTag::PlayerBullet)
		{
			m_hitDir = m_rigid->GetPos() - colider->GetRigidbody()->GetPos();
			m_hitDir.Normalize();
			ChangeAnim(kAnimIndexIdle);
			m_update = &SpaceEmperor::HitUpdate;
		}
	}
	
}

void SpaceEmperor::DoNothingUpdate()
{
	m_rigid->SetVelocity(Vec3::Zero());
}

void SpaceEmperor::IntroUpdate()
{
	m_update = &SpaceEmperor::IdleUpdate;
}

void SpaceEmperor::IdleUpdate()
{
	//{//プレイヤーの方を見るようにしたい
	//	int index = MV1SearchFrame(m_modelHandle, "mixamorig:Neck");
	//	Vec3 neckPos = MV1GetFramePosition(m_modelHandle, index);
	//	Vec3 toTarget = m_target->GetRigidbody()->GetPos()-neckPos;
	//	toTarget.Normalize();

	//	DrawLine3D(neckPos.VGet(), (neckPos + toTarget * 30).VGet(), 0xff0000);
	//	DrawLine3D(neckPos.VGet(), (neckPos + m_neckNowDir * 30).VGet(), 0x0000ff);
	//	MATRIX DirMat = MGetRotVec2(m_neckNowDir.VGet(), toTarget.VGet());
	//	m_neckNowDir = toTarget;
	//	MATRIX localMat = MV1GetFrameLocalMatrix(m_modelHandle, index);
	//	MATRIX mat = MMult(DirMat, localMat);
	//	MV1SetFrameUserLocalMatrix(m_modelHandle, index, mat);
	//}
	
	
	int index = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
	Vec3  leftHandShiftPos = Vec3(MV1GetFramePosition(m_modelHandle, index)) - m_rigid->GetPos();
	m_armCol->SetShiftPosNum(leftHandShiftPos);

	/*ChangeAnim(kAnimIndexAttack);
	m_update = &SpaceEmperor::AttackUpdate;*/

	m_rigid->SetVelocity(Vec3::Zero());
	
}

void SpaceEmperor::AttackUpdate()
{
	int index = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
	Vec3  leftHandShiftPos = Vec3(MV1GetFramePosition(m_modelHandle, index)) - m_rigid->GetPos();
	m_armCol->SetShiftPosNum(leftHandShiftPos);
	
	m_rigid->SetVelocity(Vec3::Zero());

}

void SpaceEmperor::HitUpdate()
{
	{
		int index = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
		Vec3  leftHandShiftPos = Vec3(MV1GetFramePosition(m_modelHandle, index)) - m_rigid->GetPos();
		m_armCol->SetShiftPosNum(leftHandShiftPos);
	}
	//プレイヤーの方を見るようにしたい
	/*{
		int index = MV1SearchFrame(m_modelHandle, "mixamorig:Neck");
		MATRIX hitDirMat = MGetRotVec2(m_neckNowDir.VGet(), m_hitDir.VGet());
		m_neckNowDir = m_hitDir;
		MATRIX localMat = MV1GetFrameLocalMatrix(m_modelHandle, index);
		MATRIX mat = MMult(hitDirMat, localMat);
		MV1SetFrameUserLocalMatrix(m_modelHandle, index, mat);
	}*/
	int index = MV1SearchFrame(m_modelHandle, "mixamorig:LeftArm");
	MATRIX shotDirMat = MGetTranslate((m_hitDir*m_armExtensionSpeed).VGet());
	m_armExtensionDistance += m_armExtensionSpeed;
	MATRIX localMat = MV1GetFrameLocalMatrix(m_modelHandle, index);
	MATRIX mat = MMult(shotDirMat, localMat);
	MV1SetFrameUserLocalMatrix(m_modelHandle, index, mat);

	if (m_armExtensionDistance > 200)m_armExtensionSpeed *= -1;
	if (m_armExtensionDistance < 0)
	{
		m_update = &SpaceEmperor::IdleUpdate;
		m_armExtensionSpeed *= -1;
	}

	m_rigid->SetVelocity(Vec3::Zero());
}


bool SpaceEmperor::UpdateAnim(int attachNo)
{
	//アニメーションが設定されていないので終了
	if (attachNo == -1) return false;

	//アニメーションを進行させる
	float now = MV1GetAttachAnimTime(m_modelHandle, attachNo);//現在の再生カウント
	now += kAnimFrameSpeed * m_animSpeed / kFrameParSecond;//アニメーションカウントを進める

	//現在再生中のアニメーションの総カウントを取得する
	float total = MV1GetAttachAnimTotalTime(m_modelHandle, attachNo);
	bool isLoop = false;
	while (now >= total)
	{
		now -= total;
		isLoop = true;
	}

	MV1SetAttachAnimTime(m_modelHandle, attachNo, now);

	return isLoop;
}

void SpaceEmperor::ChangeAnim(int animIndex, int speed)
{
	m_animSpeed = speed;
	//さらに古いアニメーションがアタッチされている場合はこの時点で削除しておく
	if (m_prevAnimNo != -1)
	{
		MV1DetachAnim(m_modelHandle, m_prevAnimNo);
	}

	//現在再生中の待機アニメーションは変更前のアニメーション扱いに
	m_prevAnimNo = m_currentAnimNo;

	//変更後のアニメーションとして攻撃アニメーションを改めて設定する
	m_currentAnimNo = MV1AttachAnim(m_modelHandle, animIndex, -1, false);

	//切り替えの瞬間は変更前のアニメーションが再生される状態にする
	m_animBlendRate = 0.0f;

	//変更前のアニメーション100%
	DxLib::MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimNo, 1.0f - m_animBlendRate);
	//変更後のアニメーション0%
	DxLib::MV1SetAttachAnimBlendRate(m_modelHandle, m_currentAnimNo, m_animBlendRate);
}

#include "Button.h"
#include "File.h"
#include "SoundManager.h"
#include "Collider/ColliderBox.h"
#include "Object/Gimmick/GimmickLinkObject.h"

namespace
{
	const wchar_t* const FILE_PUSH = L"S_ButtonPush";

	constexpr float MOVE_SPEED = 0.02f;
	constexpr float MOVE_SIZE = 0.8f;
	constexpr int RELEASE_COUNT = 2;
}

Button::Button() :
	GimmickSendObject(Priority::STATIC, ObjectTag::BUTTON),
	m_count(0),
	m_isRelease(false),
	m_isDown(false),
	m_isUp(false)
{
	m_stateMachine.AddState(State::DOWN, [this]() { OnDown(); }, [this]() { UpdateDown(); }, nullptr);
	m_stateMachine.AddState(State::UP,	 [this]() { OnUp(); },	 [this]() { UpdateUp(); },	 nullptr);
}

Button::~Button()
{
}

void Button::Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity)
{
	Object3DBase::Init(pos, scale, rot, list, isGravity);

	m_pushSe = FileManager::GetInstance().Load(FILE_PUSH);

	m_startPos = pos;
	m_stateMachine.ChangeState(State::UP);
}

void Button::Update()
{
	if (m_isRelease)
	{
		++m_count;
		if (m_count > RELEASE_COUNT)
		{
			m_stateMachine.ChangeState(State::UP);
			m_isRelease = false;
		}
	}

	m_stateMachine.Update();
}

void Button::OnCollideStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnCollideStay(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::PALYER || tag == ObjectTag::HAND_OBJ)
	{
		m_linkObj->OnGimmick();

		m_isRelease = false;
		m_stateMachine.ChangeState(State::DOWN);
	}
}

void Button::OnCollideExit(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnCollideExit(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::PALYER || tag == ObjectTag::HAND_OBJ)
	{
		m_linkObj->StopGimmick();

		if (m_isRelease) return;
		m_count = 0;
		m_isRelease = true;
	}
}

void Button::UpdateDown()
{
	if (!m_isDown) return;

	auto pos = m_rigid.GetPos();
	pos.y -= MOVE_SPEED;
	if (pos.y < m_startPos.y - MOVE_SIZE)
	{
		pos.y = m_startPos.y - MOVE_SIZE;
		m_isDown = false;
	}
	m_rigid.SetPos(pos);
}

void Button::UpdateUp()
{
	if (!m_isUp) return;

	auto pos = m_rigid.GetPos();
	pos.y += MOVE_SPEED;
	if (pos.y > m_startPos.y)
	{
		pos.y = m_startPos.y;
		m_isUp = false;
	}
	m_rigid.SetPos(pos);
}

void Button::OnDown()
{
	SoundManager::GetInstance().PlaySe3D(m_pushSe->GetHandle(), shared_from_this());
	m_isDown = true;
}

void Button::OnUp()
{
	m_isUp = true;
}

#include "Object3DBase.h"
#include <DxLib.h>
#include <cassert>
#include "Physics.h"
#include "File.h"
#include "Matrix4x4.h"
#include "Collider/Colliders.h"

Object3DBase::Object3DBase(Priority priority, ObjectTag tag) :
	MyEngine::Collidable(priority, tag),
	m_modelHandle(-1),
	m_vsH(-1),
	m_psH(-1),
	m_pivot { 0.0f, 0.0f, 0.0f },
	m_scale { 1.0f, 1.0f, 1.0f },
	m_rotation {},
	m_modelFile(nullptr),
	m_isLoadModel(false)
{
	m_createFunc[MyEngine::ColKind::BOX] = &Object3DBase::CreateColldierBox;
	m_createFunc[MyEngine::ColKind::SPHERE] = &Object3DBase::CreateColldierSphere;
	m_createFunc[MyEngine::ColKind::CAPSULE] = &Object3DBase::CreateColldierCapsule;
}

Object3DBase::~Object3DBase()
{
}

void Object3DBase::Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity)
{
	OnEntryPhysics();
	m_rigid.SetPos(pos);
	m_scale = scale;
	m_rigid.SetGravity(isGravity);
	m_rotation = rot;

	for (auto& item : list)
	{
		auto& func = m_createFunc.at(item.t1);
		(this->*func)(item.t2);
	}
}

void Object3DBase::End()
{
	Collidable::End();
	if (m_isLoadModel)
	{
		MV1DeleteModel(m_modelHandle);
		m_isLoadModel = false;
	}
}

void Object3DBase::Draw() const
{
	// モデルをロードしていない場合は描画しない
	if (!m_isLoadModel) return;

	// シェーダの設定
	auto err = SetUseVertexShader(m_vsH);
	assert(err != -1 && "頂点シェーダの設定に失敗");
	err = SetUsePixelShader(m_psH);
	assert(err != -1 && "ピクセルシェーダの設定に失敗");

	// スケール・回転・座標(pivot有り)を適用して描画
	Matrix4x4 mat;
	mat = Matrix4x4::Scale(m_scale) * Matrix4x4::Rot(m_rotation) * Matrix4x4::Pos(m_rigid.GetPos() + m_pivot);
	MV1SetMatrix(m_modelHandle, mat.GetMATRIX());
	err = MV1DrawModel(m_modelHandle);
	assert(err != -1 && "3Dモデルの描画に失敗");
}

bool Object3DBase::LoadModel(const wchar_t* const fileId)
{
	if (m_isLoadModel)
	{
		assert(false && "既にロードされています");
		return false;
	}
	auto& fileMgr = FileManager::GetInstance();
	m_modelFile = fileMgr.Load(fileId);
	m_modelHandle = MV1DuplicateModel(m_modelFile->GetHandle());
	m_vsH = fileMgr.GetVS(fileId);
	m_psH = fileMgr.GetPS(fileId);
	m_isLoadModel = true;
	return true;
}

void Object3DBase::CreateColldierBox(MyEngine::ColliderBase* base)
{
	auto box = dynamic_cast<MyEngine::ColliderBox*>(base);

	auto col = std::dynamic_pointer_cast<MyEngine::ColliderBox>(CreateCollider(MyEngine::ColKind::BOX));
	col->center = box->center;
	col->isTrigger = box->isTrigger;
	col->size = box->size;
	col->rotation = box->rotation;
	col->norm = box->norm;
}

void Object3DBase::CreateColldierSphere(MyEngine::ColliderBase* base)
{
	auto sphere = dynamic_cast<MyEngine::ColliderSphere*>(base);

	auto col = std::dynamic_pointer_cast<MyEngine::ColliderSphere>(CreateCollider(MyEngine::ColKind::SPHERE));
	col->center = sphere->center;
	col->isTrigger = sphere->isTrigger;
	col->radius = sphere->radius;
}

void Object3DBase::CreateColldierCapsule(MyEngine::ColliderBase* base)
{
	auto capsule = dynamic_cast<MyEngine::ColliderCapsule*>(base);

	auto col = std::dynamic_pointer_cast<MyEngine::ColliderCapsule>(CreateCollider(MyEngine::ColKind::CAPSULE));
	col->center = capsule->center;
	col->isTrigger = capsule->isTrigger;
	col->dir = capsule->dir;
	col->size = capsule->size;
	col->radius = capsule->radius;
}

bool Object3DBase::IsInCamera() const
{
	return false;
}


#pragma once
#include "Collidable.h"
#include <memory>
#include <unordered_map>
#include "Vec3.h"
#include "Quaternion.h"
#include "Utility.h"

class FileBase;
namespace
{
	class ColliderBase;
	enum class ColKind;
}

class Object3DBase : public MyEngine::Collidable
{
public:
	Object3DBase(Priority priority, ObjectTag tag);
	virtual ~Object3DBase();

	virtual void Init() {}
	virtual void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity);
	virtual void End() override;
	virtual void Restart() {}
	virtual void Update() {}
	virtual void Draw() const;

	/// <summary>
	/// モデルのロード
	/// </summary>
	/// <param name="fileId">ファイルID</param>
	/// <returns>true:成功 /false:失敗</returns>
	virtual bool LoadModel(const wchar_t* const fileId);

	bool IsLoadModel() const { return m_isLoadModel; };

private:
	void CreateColldierBox(MyEngine::ColliderBase* base);
	void CreateColldierSphere(MyEngine::ColliderBase* base);
	void CreateColldierCapsule(MyEngine::ColliderBase* base);

	bool IsInCamera() const;

protected:
	// モデルハンドル
	int m_modelHandle;
	// 頂点シェーダハンドル
	int m_vsH;
	// ピクセルシェーダハンドル
	int m_psH;

	// ピボット(モデルの中心をずらす)
	Vec3 m_pivot;
	// スケール
	Vec3 m_scale;
	// 回転
	Quaternion m_rotation;

private:
	using CreateColliderFunc_t = void(Object3DBase::*)(MyEngine::ColliderBase*);
	std::unordered_map<MyEngine::ColKind, CreateColliderFunc_t> m_createFunc;
	// モデルファイル
	std::shared_ptr<FileBase> m_modelFile;
	// モデルをロードしているか
	bool m_isLoadModel;
};


#include "EffekseerManager.h"
#include <EffekseerForDXLib.h>
#include <algorithm>
#include "Geometry.h"
#include "FileManager.h"
#include "FileBase.h"

Effekseer::Matrix43 GetEffMat(const Matrix4x4& mat);

EffekseerManager::EffekseerManager()
{
}

EffekseerManager::~EffekseerManager()
{
}

EffekseerManager& EffekseerManager::GetInstance()
{
	static EffekseerManager instance;
	return instance;
}

void EffekseerManager::Init(int particle)
{
	// 引数：パーティクル数
	Effekseer_Init(particle);
	// 歪みエフェクトの初期化
	Effekseer_InitDistortion();
	// フルスク切り替え時におかしくならないように設定
	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}

void EffekseerManager::End()
{
	Effkseer_End();
}

void EffekseerManager::Update()
{
	// 更新
	UpdateEffekseer3D();
	// 削除リスト
	std::list<const wchar_t*> eraseList;
	// 現在流しているエフェクト全てを確認
	for (auto& item : m_playInfoList)
	{
		// 流しているエフェクトが終了していれば削除
		auto& info = item.second;
		info.handleList.remove_if(
			[](const auto& handle)
			{
				return IsEffekseer3DEffectPlaying(handle) == -1;
			});
		// 流しているエフェクトがなくなれば削除リストに追加
		if (info.handleList.empty())
		{
			eraseList.emplace_back(item.first);
		}
	}
	for (auto& item : eraseList)
	{
		// mapから削除することによりファイルも削除する
		m_playInfoList.erase(item);
	}
}

void EffekseerManager::Draw() const
{
	// DxLibの描画周りと同期させる
	Effekseer_Sync3DSetting();
	// 描画
	DrawEffekseer3D();
}

int EffekseerManager::Play(const wchar_t* const id)
{
	// mapにまだ追加されていなければ
	auto isFind = m_playInfoList.find(id) != m_playInfoList.end();
	if (!isFind)
	{
		auto& mgr = FileManager::GetInstance();
		PlayInfo info;
		// モデルのロード
		info.file = mgr.Load(id);
		m_playInfoList[id] = info;
	}
	auto& info = m_playInfoList.at(id);
	// エフェクトの再生
	auto handle = PlayEffekseer3DEffect(info.file->GetHandle());
	// mapのプレイリストに追加
	info.handleList.emplace_back(handle);
	// ハンドルを返す
	return handle;
}

void EffekseerManager::Stop(int handle)
{
	StopEffekseer3DEffect(handle);
}

void EffekseerManager::AllStop()
{
	// エフェクトの全停止
	for (auto& item : m_playInfoList)
	{
		for (auto& handle : item.second.handleList)
		{
			StopEffekseer3DEffect(handle);
		}
	}
	// mapから全削除
	m_playInfoList.clear();
}

void EffekseerManager::SetInfo(int handle, const Vec3& pos, const Quaternion& rot)
{
	SetInfo(handle, pos, rot, Vec3(1.0f, 1.0f, 1.0f));
}

void EffekseerManager::SetInfo(int handle, const Vec3& pos, const Quaternion& rot, const Vec3& scale)
{
	const auto& mgr = GetEffekseer3DManager();

	auto mat = Matrix4x4::Scale(scale) * Matrix4x4::Rot(rot) * Matrix4x4::Pos(pos);
	mgr->SetBaseMatrix(handle, GetEffMat(mat));
	
}

Effekseer::Matrix43 GetEffMat(const Matrix4x4& mat)
{
	Effekseer::Matrix43 ret;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			ret.Value[y][x] = mat.m[y][x];
		}
	}

	return ret;
}

#pragma once
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include "Vec3.h"
#include "Quaternion.h"

class AFile;
class FileBase;
class GateManager;
class Object3DBase;
class CheckPoint;
class Player;
namespace MyEngine
{
	class ColliderBase;
	enum class ColKind;
}

class StageManager
{
private:
	struct ModelData
	{
		Vec3 pos;
		Vec3 scale;
		Quaternion rot;
	};
public:
	StageManager(const wchar_t* const stageName);
	~StageManager();

	void AsyncInit();
	void Init(Player* player, GateManager* gateMgr);
	void Update();
	void Draw() const;
	void End();

	void Restart();

	void UpdateCheckPoint(int checkNo);
	const Vec3& GetCheckPoint() const;
	bool IsClear() const;

private:
	void LoadAndCreateObject(Player* player, GateManager* gateMgr);
	void LoadGimmickLinkInfo();
	void LoadFloorMoveInfo();
	void LoadCheckPoint();

	MyEngine::ColliderBase* LoadBoxColInfo(const std::shared_ptr<AFile>& file, const ModelData& data);
	MyEngine::ColliderBase* LoadSphereColInfo(const std::shared_ptr<AFile>& file, const ModelData& data);
	MyEngine::ColliderBase* LoadCapsuleColInfo(const std::shared_ptr<AFile>& file, const ModelData& data);

private:
	using CreateColliderFunc_t = MyEngine::ColliderBase*(StageManager::*)(const std::shared_ptr<AFile>&, const ModelData&);
	std::unordered_map<MyEngine::ColKind, CreateColliderFunc_t> m_createFunc;

	std::list<std::shared_ptr<FileBase>> m_files;

	std::vector<std::shared_ptr<AFile>> m_afiles;
	std::vector<std::shared_ptr<Object3DBase>> m_objs;
	std::vector<std::shared_ptr<CheckPoint>> m_checkPoints;

	const wchar_t* const m_stageName;
};


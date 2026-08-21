#pragma once
#include <memory>
class SceneManager;
/// <summary>
/// シーン基底クラス
/// いろんなシーンの基になるクラス
/// </summary>
class Scene
{
protected:
	SceneManager& m_manager;
public:
	/// <summary>
	/// 生成時にSceneManagerの参照を受け取っておく
	/// </summary>
	/// <param name="manager">SceneManagerの参照</param>
	Scene(SceneManager& manager);
	virtual ~Scene() {};

	virtual void Load() = 0;

	/// <summary>
	/// シーンの更新
	/// </summary>
	/// <param name="input">入力オブジェクト</param>
	virtual void Update() = 0;
	/// <summary>
	/// シーンの描画
	/// </summary>
	virtual void Draw() = 0;


};




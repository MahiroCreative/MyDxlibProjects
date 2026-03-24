#pragma once
#include"Collidable.h"

class Planet : public MyEngine::Collidable
{
public:
	Planet();
	Planet(int modelHandle);
	virtual ~Planet();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	virtual Vec3 GravityEffect(std::shared_ptr<Collidable> obj)=0;//Physicsに呼び出される,返り値で補正後のベクトルを返す
	virtual Vec3 GetNormVec(Vec3 pos) = 0;
	bool GetClearFlag() { return clearFlag; }
	virtual void SetIsSearch(bool flag) { m_isSearch=flag; }
protected:
	int modelH;
	float gravityPower;
	bool clearFlag=false;
	int m_pointLightHandle;
	bool m_isSearch;
};


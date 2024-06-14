#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region Class
class Script_LiveObject abstract : public Component {
	COMPONENT_ABSTRACT(Script_LiveObject, Component)

private:
	float mMaxHP{};
	float mShieldAmount{};
	float mCrntHP{};
	float mPrevHP{};

	bool  mIsDead{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	void SetMaxHP(float hp) { mMaxHP = hp; }
	void SetShield(float shield) { mShieldAmount = shield; }
	void AddShield(float shield) { mShieldAmount += shield; }

	bool IsDead() const { return mIsDead; }
	float GetCrntHp() const { return mCrntHP; }
	float GetMaxHp() const { return mMaxHP; }

	bool UpdatePrevHP() { bool res = mPrevHP == mCrntHP; mPrevHP = mCrntHP; return res; }

	virtual bool Hit(float damage, Object* instigator = nullptr);
	virtual void Dead();
	virtual void Resurrect();
};

#pragma endregion
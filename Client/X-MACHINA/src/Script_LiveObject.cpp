#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_LiveObject.h"
#include "Timer.h"



void Script_LiveObject::Awake()
{
	base::Awake();
}

void Script_LiveObject::Start()
{
	base::Start();

	mCrntHP = mMaxHP;
	mPrevHP = mMaxHP;
	mRemoveTime = 4.f;
}

void Script_LiveObject::Update()
{
	base::Update();
}

void Script_LiveObject::FillHP(float amount)
{
	mCrntHP += amount;
	if (mCrntHP > mMaxHP) {
		mCrntHP = mMaxHP;
	}
}

bool Script_LiveObject::Hit(float damage, Object* instigator)
{
	if (mCrntHP <= 0) {
		return false;
	}

	if (mShieldAmount > 0) {
		mShieldAmount -= damage;
	}
	else {
		//mCrntHP -= damage;
	}

	if (mCrntHP <= 0) {
		Dead();
		return true;
	}

	return false;
}

void Script_LiveObject::Dead()
{
	mIsDead = true;
}

void Script_LiveObject::Resurrect()
{
	mCrntHP = mMaxHP;
	mIsDead = false;
}

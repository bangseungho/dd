#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_EnemyManager.h"
#include "Script_GroundObject.h"
#include "Script_PheroObject.h"
#include "Script_DefaultEnemyBT.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"

#include "Timer.h"
#include "Object.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

#include "XLManager.h"


void Script_Enemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mObject->AddComponent<Script_GroundObject>();
	mObject->AddComponent<Script_PheroObject>();
#ifndef SERVER_COMMUNICATION
	mObject->AddComponent<Script_DefaultEnemyBT>();
#endif

	SetEnemyStat(mObject->GetName());
	SetMaxHP(mEnemyMgr->mStat.MaxHp);

	if (mEnemyMgr->mStat.Attack1AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
	if (mEnemyMgr->mStat.Attack2AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
	if (mEnemyMgr->mStat.Attack3AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
	if (mEnemyMgr->mStat.DeathAnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.DeathAnimName)->AddEndCallback(std::bind(&Script_Enemy::DeathEndCallback, this));
	}
}

void Script_Enemy::Update()
{
	base::Update();

	mObject->mObjectCB.HitRimFactor = std::max(mObject->mObjectCB.HitRimFactor - DeltaTime(), 0.f);

	// TODO : 임의로 발표를 위해 여기에 둠 추후에 변경해야 한다.
	if (IsDead()) {
		mEnemyMgr->mState = EnemyState::Death;

		mAccTime += DeltaTime();

		mEnemyMgr->RemoveAllAnimation();
		mEnemyMgr->mController->SetValue("Death", true);

		//ExecuteCallback();

		if (mAccTime >= mRemoveTime) {
			mObject->mObjectCB.HitRimFactor = 0.7f;
			mObject->Destroy();
		}
	}
}

void Script_Enemy::OnDestroy()
{
	CLIENT_NETWORK->EraseMonster(mObject->GetID());
}

void Script_Enemy::Attack()
{
	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Attack", true);
}

bool Script_Enemy::Hit(float damage, Object* instigator)
{
	bool res = base::Hit(damage, instigator);

	mObject->mObjectCB.HitRimFactor = 0.7f;
	
	if (nullptr != instigator) {
		mEnemyMgr->mTarget = instigator;
	}

	return res;
}

void Script_Enemy::SetEnemyStat(const std::string& modelName)
{
	XLManager::I->Set(modelName, mEnemyMgr->mStat);
}

void Script_Enemy::AttackCallback()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	if (Vec3::Distance(mEnemyMgr->mTarget->GetPosition(), mObject->GetPosition()) <= mEnemyMgr->mStat.AttackRange) {
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			liveObject->Hit(mEnemyMgr->mStat.AttackRate, mObject);
		}
	}
}

void Script_Enemy::AttackEndCallback()
{
	//mEnemyMgr->mController->SetValue("Attack", false);
	//mEnemyMgr->mState = EnemyState::Idle;
}

void Script_Enemy::DeathEndCallback()
{
	mEnemyMgr->mController->GetCrntMotion()->SetSpeed(0.f);
}
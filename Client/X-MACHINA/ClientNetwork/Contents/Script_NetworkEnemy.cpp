#include "stdafx.h"
#include "Script_NetworkEnemy.h"
#include "Timer.h"
#include "Script_EnemyManager.h"
#include "GameFramework.h"
#include "Object.h"
#include "ClientNetworkManager.h"
#include "AnimatorController.h"

void Script_NetworkEnemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->GetComponent<Script_EnemyManager>().get();
}

void Script_NetworkEnemy::Update()
{
	base::Update();

	if (mEnemyMgr->mState == EnemyState::MoveToTarget)
		MoveToTarget();
	if (mEnemyMgr->mState == EnemyState::Attack)
		Attack();
	if (mEnemyMgr->mState == EnemyState::Idle)
		Idle();
	if (mEnemyMgr->mState == EnemyState::Death)
		Death();
}

void Script_NetworkEnemy::SetPosition(const Vec3& pos)
{
	mObject->SetPosition(pos);
}

void Script_NetworkEnemy::SetRotation(float angle)
{
	mObject->SetLocalRotation(Quat::Identity);
	mObject->Rotate(Vector3::Up, angle);
}

void Script_NetworkEnemy::SetLocalRotation(const Quat& quat)
{
	mObject->SetLocalRotation(quat);
}

void Script_NetworkEnemy::SetState(EnemyState state)
{
	mEnemyMgr->mState = state;

	switch (mEnemyMgr->mState)
	{
	case EnemyState::Idle:
		std::cout << "Idle" << std::endl;
		break;
	case EnemyState::Walk:
		std::cout << "Walk" << std::endl;
		break;
	case EnemyState::GetHit:
		std::cout << "GetHit" << std::endl;
		break;
	case EnemyState::Attack:
		std::cout << "Attack" << std::endl;
		break;
	case EnemyState::Death:
		std::cout << "Death" << std::endl;
		break;
	case EnemyState::MoveToTarget:
		std::cout << "MoveToTarget" << std::endl;
		break;
	case EnemyState::MoveToPath:
		std::cout << "MoveToPath" << std::endl;
		break;
	case EnemyState::Patrol:
		std::cout << "Patrol" << std::endl;
		break;
	default:
		break;
	}
}

void Script_NetworkEnemy::SetState(FBProtocol::MONSTER_BT_TYPE btType)
{
	switch (btType)
	{
	case FBProtocol::MONSTER_BT_TYPE_DEATH:
		SetState(EnemyState::Death);
		break;
	case FBProtocol::MONSTER_BT_TYPE_ATTACK:
		SetState(EnemyState::Attack);
		break;
	case FBProtocol::MONSTER_BT_TYPE_GETHIT:
		SetState(EnemyState::GetHit);
		break;
	case FBProtocol::MONSTER_BT_TYPE_MOVE_TO_TARGET:
		SetState(EnemyState::MoveToTarget);
		break;
	case FBProtocol::MONSTER_BT_TYPE_MOVE_TO_PATH:
		SetState(EnemyState::MoveToPath);
		break;
	case FBProtocol::MONSTER_BT_TYPE_PATROL:
		SetState(EnemyState::Patrol);
		break;
	default:
		SetState(EnemyState::Idle);
		break;
	}
}

void Script_NetworkEnemy::SetTarget(Object* target)
{
	mEnemyMgr->mTarget = target;
}

void Script_NetworkEnemy::SetActiveMyObject(bool isActive)
{
	mObject->SetActive(isActive);
}

EnemyState Script_NetworkEnemy::GetState()
{
	return mEnemyMgr->mState;
}

void Script_NetworkEnemy::MoveToTarget()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	// 허리 쪽부터 광선을 쏴야 맞는다.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = mEnemyMgr->mTarget->GetPosition() + mEnemyMgr->mTarget->GetUp() * 0.5f;

	// 오브젝트로부터 타겟까지의 벡터
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	if (toTarget.Length() < mEnemyMgr->mStat.AttackRange) {
		return;
	}

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;

	// 타겟에 도착하지 않았을 경우에만 이동
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}

	mEnemyMgr->mController->SetValue("Attack", false);
	mEnemyMgr->mController->SetValue("Walk", true);
}

void Script_NetworkEnemy::Attack()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.AttackRotationSpeed);
	mEnemyMgr->mController->SetValue("Walk", false);
	mEnemyMgr->mController->SetValue("Attack", true);
}

void Script_NetworkEnemy::Idle()
{
	mEnemyMgr->Reset();
}

void Script_NetworkEnemy::Death()
{
	mEnemyMgr->mState = EnemyState::Death;

	mDeathAccTime += DeltaTime();

	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Death", true);

	if (mDeathAccTime >= mDeathRemoveTime) {
		mObject->mObjectCB.HitRimFactor = 0.7f;
		mObject->Destroy();
		CLIENT_NETWORK->EraseMonster(mObject->GetID());
	}
}

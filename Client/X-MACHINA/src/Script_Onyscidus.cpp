#include "stdafx.h"
#include "Script_Onyscidus.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

void Script_Onyscidus::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_Onyscidus::AttackCallback, this), 17);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_Onyscidus::AttackCallback, this), 44);
}


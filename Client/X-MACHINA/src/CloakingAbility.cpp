#include "stdafx.h"
#include "CloakingAbility.h"

#include "Scene.h"
#include "Object.h"
#include "Timer.h"
#include "Animator.h"

#include "Component/ParticleSystem.h"
#include "Script_Player.h"
#include "Script_AfterImageObject.h"



CloakingAbility::CloakingAbility()
	:
	Ability("Cloaking"),
	PheroAbilityInterface(30.f)
{
}

void CloakingAbility::Update(float activeTime)
{
	if (!ReducePheroAmount()) {
		mTerminateCallback();
		DeActivate();
		return;
	}
}

void CloakingAbility::Start()
{
	mAfterImage = mObject->AddComponent<Script_AfterImageObject>();
	mAfterImage->SetAfterImage(10, 3.f);
	mAfterImage->SetActive(false);
}

void CloakingAbility::Activate()
{
	base::Activate();
	mBuffSparkPS = ParticleManager::I->Play("MagicCircle_Sparks", mObject);
	mBuffDotPS = ParticleManager::I->Play("MagicCircle_Dot", mObject);

	mAfterImage->SetActive(true);
}

void CloakingAbility::DeActivate()
{
	base::DeActivate();
	mBuffSparkPS->Stop();
	mBuffDotPS->Stop();

	mAfterImage->SetActive(false);
}

bool CloakingAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}
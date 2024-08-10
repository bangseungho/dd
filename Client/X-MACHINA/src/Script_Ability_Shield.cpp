#include "stdafx.h"
#include "Script_Ability_Shield.h"

#include "BattleScene.h"
#include "GameFramework.h"

#include "Shader.h"
#include "Object.h"
#include "ResourceMgr.h"

#include "Script_Player.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"





/// +-------------------------------------------------
///		Script_Ability_Shield 
/// -------------------------------------------------+
void Script_Ability_Shield::Awake()
{
	base::Awake();

	Init("Shield", 2.f, 4.5f);
	SetPheroCost(100.f);

	mLayer = 1;
	mAbilityCB.Duration = 4.f;
	mShieldAmount = 30.f;
	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel("Shield");

	mShader = RESOURCE<Shader>("ShieldAbility");
	SetType(Type::Cooldown);
}

void Script_Ability_Shield::Start()
{
	base::Start();

	mPlayer = mObject->GetComponent<Script_PheroPlayer>();
}

void Script_Ability_Shield::Update()
{
	base::Update();

	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.85f, 0.f };
	mRenderedObject->SetPosition(playerPos);
}

void Script_Ability_Shield::On()
{
	if (!ReducePheroAmount()) {
		SetActive(false);
		return;
	}

	base::On();

	mPlayer->SetShield(mShieldAmount);

#ifdef SERVER_COMMUNICATION
	/// +-------------------------------
	///		SKILLPACKET BROADCAST
	/// -------------------------------+
	auto cpkt = FBS_FACTORY->CPkt_Player_OnSkill(FBProtocol::PLAYER_SKILL_TYPE_SHIELD);
	CLIENT_NETWORK->Send(cpkt);
#endif
}

void Script_Ability_Shield::Off()
{
	base::Off();

	mPlayer->SetShield(0.f);
}

bool Script_Ability_Shield::ReducePheroAmount(bool checkOnly)
{
	if (mPlayer) {
		return mPlayer->ReducePheroAmount(mPheroCost);
	}

	return false;
}





/// +-------------------------------------------------
///		Script_Remote_Ability_Shield 
/// -------------------------------------------------+
void Script_Remote_Ability_Shield::On()
{
	Script_RenderedAbility::On();
}

void Script_Remote_Ability_Shield::Off()
{
	Script_RenderedAbility::Off();
}

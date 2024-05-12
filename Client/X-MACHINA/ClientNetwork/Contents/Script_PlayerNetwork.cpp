#include "stdafx.h"
#include "Script_PlayerNetwork.h"
#include "ClientNetworkManager.h"
#include "FBsPacketFactory.h"

#include "GameFramework.h"
#include "X-Engine.h"
#include "Object.h"
#include "FBsPacketFactory.h"
#include "Animator.h"

#include "InputMgr.h"
#include "Script_GroundObject.h"
#include "Script_Player.h"


void Script_PlayerNetwork::Awake()
{
	base::Awake();

	mLatencyTimePoint_latest = std::chrono::steady_clock::now();

}

void Script_PlayerNetwork::LateUpdate()
{
	base::LateUpdate();

	DoInput();
	//DoNetLatency();

}

void Script_PlayerNetwork::UpdateData(const void* data)
{


}

#include <math.h>
void Script_PlayerNetwork::DoInput()
{
	///* Player Network 관련 기능을 담당하는 Script에 넣을 예정 .. */
	if (KEY_TAP('W') || KEY_TAP('A') || KEY_TAP('S') || KEY_TAP('D'))
	{
		mMoveTimePoint_latest = std::chrono::steady_clock::now();
		
		float					Vel        = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
		Vec3					Pos        = GameFramework::I->GetPlayer()->GetPosition();
		Vec3					MoveDir    = Pos - mPrevPos; MoveDir.Normalize();
		
		Vec4					QuatRot = GameFramework::I->GetPlayer()->GetRotation();
		Vec3 Rot{};  Rot.y = GetYRotation();

		int32_t	moveState = PLAYER_MOVE_STATE::Start;
		Vec3					SpineDir   = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
		long long				latency    = FBS_FACTORY->CurrLatency.load();

		auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency);
		CLIENT_NETWORK->Send(pkt);

		mPrevPos = Pos;
	}

	if (KEY_PRESSED('W') || KEY_PRESSED('A') || KEY_PRESSED('S') || KEY_PRESSED('D'))
	{
		/* 1s에 PlayerNetworkInfo::SendInterval_CPkt_Trnasform 개수 만큼 패킷을 보낸다. */
		auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mMoveTimePoint_latest).count()
			>= PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000)
		{
			mMoveTimePoint_latest = currentTime;

			float					Vel       = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
			Vec3					Pos       = GameFramework::I->GetPlayer()->GetPosition();
			Vec3					MoveDir   = Pos - mPrevPos; MoveDir.Normalize();
			Vec4					QuatRot       = GameFramework::I->GetPlayer()->GetRotation();
			Vec3 Rot{};  Rot.y = GetYRotation();

			int32_t	moveState = PLAYER_MOVE_STATE::Progress;
			Vec3					SpineDir  = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
			long long				latency   = FBS_FACTORY->CurrLatency.load();

			auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency);
			CLIENT_NETWORK->Send(pkt);

			mPrevPos = Pos;

		}
	}

	if (KEY_AWAY('W') || KEY_AWAY('A') || KEY_AWAY('S') || KEY_AWAY('D'))
	{
		float					Vel       = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
		Vec3					Pos       = GameFramework::I->GetPlayer()->GetPosition();
		Vec3					MoveDir   = Pos - mPrevPos; MoveDir.Normalize();
		Vec4					QuatRot = GameFramework::I->GetPlayer()->GetRotation();
		Vec3 Rot{};  Rot.y = GetYRotation();

		int32_t	moveState = PLAYER_MOVE_STATE::End;
		Vec3					SpineDir  = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
		long long				latency   = FBS_FACTORY->CurrLatency.load();

		auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency);
		CLIENT_NETWORK->Send(pkt);

		mPrevPos = Pos;

	}
}

void Script_PlayerNetwork::DoNetLatency()
{
	/* 1초에 10번 Latency 패킷을 측정한다. */

	/* 1s에 PlayerNetworkInfo::SendInterval_CPkt_NetworkLateny 개수 만큼 패킷을 보낸다. */
	auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mLatencyTimePoint_latest).count()
		>= PlayerNetworkInfo::sendInterval_CPkt_NetworkLatency * 1000) {

		mLatencyTimePoint_latest = currentTime;

		long long timeStamp = CLIENT_NETWORK->GetCurrentTimeMilliseconds();
		auto pkt = FBS_FACTORY->CPkt_NewtorkLatency(timeStamp);
		CLIENT_NETWORK->Send(pkt);

	}
}

float Script_PlayerNetwork::GetYRotation()
{
	return Vector3::SignedAngle(Vector3::Forward, mObject->GetLook().xz(), Vector3::Up);
}

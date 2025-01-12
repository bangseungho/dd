#include "stdafx.h"
#include "FBsPacketFactory.h"
#include "ClientNetwork/Include/PacketHeader.h"

/* RELATED FLAT BUFFERS HEADER */
#undef max
#include "ClientNetwork/Include/SendBuffersFactory.h"
#include "ClientNetwork/Include/SocketData.h"

#include "ClientNetwork/Contents/Script_NetworkPlayer.h"
#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"
#include "ClientNetwork/Contents/ServerSession.h"

#include "Script_SceneManager.h"
#include "Script_LobbyManager.h"

#include "Object.h"
#include "ServerSession.h"
#include "NetworkEvents.h"
#include "ClientNetworkManager.h"
#include "GameFramework.h"


DEFINE_SINGLETON(FBsPacketFactory);

std::atomic<long long> FBsPacketFactory::TotalLatency = 0;
std::atomic<int> FBsPacketFactory::LatencyCount = 0;
std::atomic<long long> FBsPacketFactory::CurrLatency = 0;



/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS SERVER PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★
bool FBsPacketFactory::ProcessFBsPacket(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen)
{
	/* ▶ Packet Struct ------------------------------------------------- */
	/* [PacketHeader(ProtocolID, PktSize)][DATA-(FlatBuffers Serialized)] */
	/* ------------------------------------------------------------------ */

	PacketHeader* Head = reinterpret_cast<PacketHeader*>(packetBuf);
	const void* DataPtr = packetBuf + sizeof(PacketHeader);
	switch (Head->ProtocolID)
	{
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_LogIn:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_LogIn ]\n");
		const FBProtocol::SPkt_LogIn* packet = flatbuffers::GetRoot<FBProtocol::SPkt_LogIn>(DataPtr);
		if (!packet) return false;
		Process_SPkt_LogIn(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_EnterLobby:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_EnterLobby ]\n");
		const FBProtocol::SPkt_EnterLobby* packet = flatbuffers::GetRoot<FBProtocol::SPkt_EnterLobby>(DataPtr);
		if (!packet) return false;
		Process_SPkt_EnterLobby(session, *packet);
	}
	break;	
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_EnterGame:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_EnterGame ]\n");
		const FBProtocol::SPkt_EnterGame* packet = flatbuffers::GetRoot<FBProtocol::SPkt_EnterGame>(DataPtr);
		if (!packet) return false;
		Process_SPkt_EnterGame(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_PlayGame:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_PlayGame ]\n");
		const FBProtocol::SPkt_PlayGame* packet = flatbuffers::GetRoot<FBProtocol::SPkt_PlayGame>(DataPtr);
		if (!packet) return false;
		Process_SPkt_PlayGame(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Chat:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_Chat ]\n");
		const FBProtocol::SPkt_Chat* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Chat>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Chat(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Custom:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ FBsProtocolID_SPkt_Custom ]\n");
		const FBProtocol::SPkt_Custom* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Custom>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Custom(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_NetworkLatency:
	{
		//LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_NetworkLatency ]\n");
		const FBProtocol::SPkt_NetworkLatency* packet = flatbuffers::GetRoot<FBProtocol::SPkt_NetworkLatency>(DataPtr);
		if (!packet) return false;
		Process_SPkt_NetworkLatency(session, *packet);
	}
	break;

	/// ________________________________________________________________________________
	/// Remote Player 
	/// ________________________________________________________________________________ 

	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Player_Transform:
	{
		//LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_Transform ]\n");
		const FBProtocol::SPkt_Player_Transform* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_Transform>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_Transform(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_NewPlayer:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_NewPlayer ]\n");
		const FBProtocol::SPkt_NewPlayer* packet = flatbuffers::GetRoot<FBProtocol::SPkt_NewPlayer>(DataPtr);
		if (!packet) return false;
		Process_SPkt_NewPlayer(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_RemovePlayer:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_RemovePlayer ]\n");
		const FBProtocol::SPkt_RemovePlayer* packet = flatbuffers::GetRoot<FBProtocol::SPkt_RemovePlayer>(DataPtr);
		if (!packet) return false;
		Process_SPkt_RemovePlayer(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Player_Animation:
	{
		const FBProtocol::SPkt_Player_Animation* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_Animation>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_Animation(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Player_Weapon:
	{
		const FBProtocol::SPkt_Player_Weapon* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_Weapon>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_Weapon(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Player_AimRotation:
	{
		const FBProtocol::SPkt_Player_AimRotation* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_AimRotation>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_AimRotation(session, *packet);
	}
	break;


	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_PlayerOnSkill:
	{
		const FBProtocol::SPkt_PlayerOnSkill* packet = flatbuffers::GetRoot<FBProtocol::SPkt_PlayerOnSkill>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_OnSkill(session, *packet);
	}
	break;
	/// ________________________________________________________________________________
	/// Monster 
	/// ________________________________________________________________________________ 

	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_NewMonster:
	{
		const FBProtocol::SPkt_NewMonster* packet = flatbuffers::GetRoot<FBProtocol::SPkt_NewMonster>(DataPtr);
		if (!packet) return false;
		Process_SPkt_NewMonster(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_RemoveMonster:
	{
		const FBProtocol::SPkt_RemoveMonster* packet = flatbuffers::GetRoot<FBProtocol::SPkt_RemoveMonster>(DataPtr);
		if (!packet) return false;
		Process_SPkt_RemoveMonster(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_DeadMonster:
	{
		const FBProtocol::SPkt_DeadMonster* packet = flatbuffers::GetRoot<FBProtocol::SPkt_DeadMonster>(DataPtr);
		if (!packet) return false;
		Process_SPkt_DeadMonster(session, *packet);
	}
	break;

	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Monster_Transform:
	{
		const FBProtocol::SPkt_Monster_Transform* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_Transform>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_Transform(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Monster_HP:
	{
		const FBProtocol::SPkt_Monster_HP* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_HP>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_HP(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Monster_State:
	{
		const FBProtocol::SPkt_Monster_State* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_State>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_State(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Monster_Target:
	{
		const FBProtocol::SPkt_MonsterTarget* packet = flatbuffers::GetRoot<FBProtocol::SPkt_MonsterTarget>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_Target(session, *packet);
	}
	break;
	/// ________________________________________________________________________________
	/// Phero 
	/// ________________________________________________________________________________ 
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_GetPhero:
	{
		const FBProtocol::SPkt_GetPhero* packet = flatbuffers::GetRoot<FBProtocol::SPkt_GetPhero>(DataPtr);
		if (!packet) return false;

		Process_SPkt_GetPhero(session, *packet);
	}
	break;

	/// ________________________________________________________________________________
	/// Bullet 
	/// ________________________________________________________________________________ 

	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Bullet_OnShoot:
	{
		const FBProtocol::SPkt_Bullet_OnShoot* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Bullet_OnShoot>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Bullet_OnShoot(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Bullet_OnCollision:
	{
		const FBProtocol::SPkt_Bullet_OnCollision* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Bullet_OnCollision>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Bullet_OnCollision(session, *packet);
	}
	break;

	/// ________________________________________________________________________________
/// Bullet 
/// ________________________________________________________________________________ 
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Item_Interact:
	{
		const FBProtocol::SPkt_Item_Interact* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Item_Interact>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Item_Interact(session, *packet);
	}
	break;
	case FBProtocol::FBsProtocolID::FBsProtocolID_SPkt_Item_ThrowAway:
	{
		const FBProtocol::SPkt_Item_ThrowAway* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Item_ThrowAway>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Item_ThrowAway(session, *packet);
	}
	break;

	default:
	{
		assert(0);
	}
	break;
	}

	return true;
}

bool FBsPacketFactory::Process_SPkt_Invalid(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen)
{
	return false;
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [LogIn, Chat, NetworkLatency, Entergame] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_LogIn(SPtr_Session session, const FBProtocol::SPkt_LogIn& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_LogIn
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
#ifdef CONNECT_WITH_TEST_CLIENT
	return true;
#endif
	auto serverSession = std::dynamic_pointer_cast<ServerSession>(session);
	bool IsLogInSuccess = pkt.success();


	/// +------------------------------------------------------------------------------
	///	 ( X )	FAIL - LOGIN 
	/// ------------------------------------------------------------------------------+
	if (IsLogInSuccess == false) {
		GameFramework::I->FailLogin();
		return true;
	}

	/// +------------------------------------------------------------------------------
	///	 ( O ) 	SUCCESS - LOGIN 
	/// ------------------------------------------------------------------------------+
	else if (IsLogInSuccess == true) {
		std::string name = pkt.name()->c_str();
		LOG_MGR->Cout(session->GetID(), " NAME : ", name, "\n");
		session->SetName(name);

		// 로그인이 허가 되었으니 서버에 EnterGame 요청 
		auto cpkt = FBS_FACTORY->CPkt_EnterLobby(serverSession->GetID());
		session->Send(cpkt);
	}

	return true;
}

bool FBsPacketFactory::Process_SPkt_EnterGame(SPtr_Session session, const FBProtocol::SPkt_EnterGame& pkt)
{

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_EnterGame
	/// > {
	/// > 	success: bool;			// 1 byte
	/// > }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○

	Script_SceneManager::I->LobbyManager()->ChangeToBattleScene();

	return true;
}

bool FBsPacketFactory::Process_SPkt_EnterLobby(SPtr_Session session, const FBProtocol::SPkt_EnterLobby& pkt)
{
	int player_order = pkt.order();

	LOG_MGR->SetColor(TextColor::BrightCyan);
	LOG_MGR->Cout("PROCESS ENTER LOBBY PACKET\n ");
	GamePlayerInfo MyInfo = GetPlayerInfo(pkt.myinfo());

	LOG_MGR->Cout("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
	LOG_MGR->SetColor(TextColor::BrightGreen);
	LOG_MGR->Cout("[MY] NAME : ", MyInfo.Name, " ", " SESSION ID : ", MyInfo.Id, '\n');
	LOG_MGR->SetColor(TextColor::Default);

	GameFramework::I->Login(MyInfo.Id);
	MyInfo.Name = session->GetName();
	sptr<NetworkEvent::Game::Event_RemotePlayer::Add> EventData = CLIENT_NETWORK->CreateEvent_Add_RemotePlayer(MyInfo);
	CLIENT_NETWORK->RegisterEvent(EventData);

	/// ________________________________________________________________________________
	/// Remote Player infos ( Range : Same Room ) 
	/// ________________________________________________________________________________ 


	int PlayersCnt = pkt.players()->size();
	for (UINT16 i = 0; i < PlayersCnt; ++i) {
		GamePlayerInfo RemoteInfo = GetPlayerInfo(pkt.players()->Get(i));

		if (RemoteInfo.Id == MyInfo.Id) continue;
		LOG_MGR->SetColor(TextColor::BrightGreen);
		LOG_MGR->Cout("[REMOTE] NAME : ", RemoteInfo.Name, " ", " SESSION ID : ", RemoteInfo.Id, '\n');
		LOG_MGR->SetColor(TextColor::Default);

		sptr<NetworkEvent::Game::Event_RemotePlayer::Add> EventData = CLIENT_NETWORK->CreateEvent_Add_RemotePlayer(RemoteInfo);
		CLIENT_NETWORK->RegisterEvent(EventData);
	}
	LOG_MGR->Cout("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");

	return true;
}

bool FBsPacketFactory::Process_SPkt_PlayGame(SPtr_Session session, const FBProtocol::SPkt_PlayGame& pkt)
{
	auto cpkt = FBS_FACTORY->CPkt_EnterGame(session->GetID());
	session->Send(cpkt);

	return false;
}


bool FBsPacketFactory::Process_SPkt_Chat(SPtr_Session session, const FBProtocol::SPkt_Chat& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Chat
	/// > {
	/// > 	player_id: uint;		// uint64
	/// > 	message: string;	// 가변 크기
	/// > }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○	

	uint32_t	id		= pkt.player_id();
	std::string message = pkt.message()->c_str();

	sptr<NetworkEvent::Game::Event_Contents::Chat> EventData = CLIENT_NETWORK->CreateEvent_Chat(id, message);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Custom(SPtr_Session session, const FBProtocol::SPkt_Custom& pkt)
{
	int				player_id   = pkt.player_id();
	std::string		trooperskin = pkt.trooperskin()->c_str();

	sptr<NetworkEvent::Game::Event_Contents::Custom> EventData = CLIENT_NETWORK->CreateEvent_Custom(player_id, trooperskin);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_NetworkLatency(SPtr_Session session, const FBProtocol::SPkt_NetworkLatency& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_NetworkLatency
	/// > {
	/// > 	timestamp: long;	// 8 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	// 패킷으로부터 long long으로 시간을 받음
	long long timestamp = pkt.timestamp();

	// 현재 시간 구하기
	auto currentTime = CLIENT_NETWORK->GetCurrentTimeMilliseconds();

	// 패킷의 타임스탬프와 현재 시간의 차이를 구하기
	auto latency = currentTime - timestamp;
	latency = latency / 2;

	TotalLatency += latency;
	LatencyCount += 1;

	// 변환된 값을 출력 - 5개씩 Latency의 평균을 구한다. 
	if (LatencyCount.load() >= 5) {
		CurrLatency.store(TotalLatency.load() / LatencyCount.load()); /* Latency 의 평균 저장 */
		//std::cout << std::chrono::milliseconds(CurrLatency.load()) << '\n';

		LatencyCount.store(0);
		TotalLatency.store(0);
	}

	return true;
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ PLAYER ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_NewPlayer(SPtr_Session session, const FBProtocol::SPkt_NewPlayer& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_NewPlayer
	/// > {
	/// > 	newplayer: Player; // 새로운 플레이어가 접속했음을 기존의 세션들에게 알린다. 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	GamePlayerInfo NewPInfo = GetPlayerInfo(pkt.newplayer());

	LOG_MGR->SetColor(TextColor::BrightGreen);
	LOG_MGR->Cout("▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣\n");
	LOG_MGR->Cout("[NEW REMOTE] NAME : ", NewPInfo.Name, " ", " SESSION ID : ", NewPInfo.Id, '\n');
	LOG_MGR->Cout("▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣\n");
	LOG_MGR->SetColor(TextColor::Default);

	sptr<NetworkEvent::Game::Event_RemotePlayer::Add> EventData = CLIENT_NETWORK->CreateEvent_Add_RemotePlayer(NewPInfo);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_RemovePlayer(SPtr_Session session, const FBProtocol::SPkt_RemovePlayer& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_RemovePlayer
	/// > {
	/// > 	player_id: uint; // 4 bytes // 삭제할 플레이어의 아이디 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	int32_t removeID = pkt.player_id();

	LOG_MGR->SetColor(TextColor::BrightRed);
	LOG_MGR->Cout("▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷\n");
	LOG_MGR->Cout("[REMOVE REMOTE] NAME SESSION ID : ", removeID, '\n');
	LOG_MGR->Cout("▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷\n");
	LOG_MGR->SetColor(TextColor::Default);

	sptr<NetworkEvent::Game::Event_RemotePlayer::Remove> EventData = CLIENT_NETWORK->CreateEvent_Remove_RemotePlayer(removeID);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_Transform(SPtr_Session session, const FBProtocol::SPkt_Player_Transform& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Player_Transform
	/// > {
	/// > 	player_id: uint;						// 8 bytes				// uint64
	/// > 
	/// > 	move_state: PLAYER_MOTION_STATE_TYPE;	// 1 byte
	/// > 
	/// > 	latency: long;							// 8 bytes
	/// > 	velocity: float;						// 4 bytes
	/// > 	movedir: Vector3;						// 12 bytes (3 * 4 bytes)
	/// > 	trans: Transform;						// 24 bytes (Vector3 * 2)
	/// > 
	/// > 
	/// > 	spine_look: Vector3;					// 12 bytes (3 * 4 bytes)
	/// > 	animparam_h: float;						// 4 bytes
	/// > 	animparam_v: float;						// 4 bytes
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint32_t  player_id = pkt.player_id();

	int32_t   movestate = pkt.move_state();

	long long latency = pkt.latency();
	float	  velocity = pkt.velocity();
	Vec3	  movedir = GetVector3(pkt.movedir());
	Vec3	  Packetpos = GetVector3(pkt.trans()->position());
	Vec3	  rot = GetVector3(pkt.trans()->rotation());

	Vec3	  spine_look = GetVector3(pkt.spine_look());
	float     animparam_h = pkt.animparam_h();
	float     animparam_v = pkt.animparam_v();

	ExtData::MOVESTATE mState;
	if (movestate == PLAYER_MOVE_STATE::Start)			mState = ExtData::MOVESTATE::Start;
	else if (movestate == PLAYER_MOVE_STATE::Progress)	mState = ExtData::MOVESTATE::Progress;
	else if (movestate == PLAYER_MOVE_STATE::End)		mState = ExtData::MOVESTATE::End;
	else if (movestate == PLAYER_MOVE_STATE::Default)	mState = ExtData::MOVESTATE::Default;


	sptr<NetworkEvent::Game::Event_RemotePlayer::Move> Move_EventData = CLIENT_NETWORK->CreateEvent_Move_RemotePlayer(player_id, Packetpos, mState);
	CLIENT_NETWORK->RegisterEvent(Move_EventData);

	/// +---------------------------
	///	Extrapolate Next Packet Pos 
	/// ---------------------------+

	/* CurrPos --------------- PacketPos --------------------------> TargetPos */

	ExtData data = {};
	/* [Get Next Packet Duration] = (PKt Interval) + (Remote Cl Latency) + (My Latency) */
	data.PingTime = static_cast<long long>((PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000) + (latency / 1000.0) + (CurrLatency.load() / 1000.0));
	data.MoveDir = movedir;
	data.MoveState = mState;

	//LOG_MGR->Cout(data.PingTime, " ms ping \n");
	SPtr_ServerSession serversession = std::static_pointer_cast<ServerSession>(session);

	if (mState == ExtData::MOVESTATE::End) {
		data.TargetPos.x = Packetpos.x;
		data.TargetPos.z = Packetpos.z;

	}
	else if (data.MoveState == ExtData::MOVESTATE::Default) {
		data.TargetPos.x = Packetpos.x;
		data.TargetPos.z = Packetpos.z;
	}
	else {
		//data.TargetPos = Packetpos + (data.MoveDir * vel * ((data.PingTime) / 1000.0));
		data.TargetPos.x = static_cast<float>(Packetpos.x + (data.MoveDir.x * velocity * ((data.PingTime) / 1000.0)));
		data.TargetPos.z = static_cast<float>(Packetpos.z + (data.MoveDir.z * velocity * ((data.PingTime) / 1000.0)));

	}

	//LOG_MGR->Cout_Vec3("Packet Pos : ", Packetpos);
	//LOG_MGR->Cout_Vec3("Target Pos : ", data.TargetPos);
	//LOG_MGR->Cout_Vec3(" MOVE DIR ", moveDir);

	data.TargetRot = rot;
	data.Velocity = velocity;

	data.Animdata.AnimParam_h = animparam_h;
	data.Animdata.AnimParam_v = animparam_v;

	sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate> Ext_EventData = CLIENT_NETWORK->CreateEvent_Extrapolate_RemotePlayer(player_id, data);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);


	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_Animation(SPtr_Session session, const FBProtocol::SPkt_Player_Animation& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Player_Animation
	/// > {
	/// > 	player_id: uint;	// 8 bytes
	/// > 
	/// > 	animation_upper_index: int;		// 4 bytes
	/// > 	animation_lower_index: int;		// 4 bytes
	/// > 	animation_param_h: float;	// 4 bytes
	/// > 	animation_param_v: float;	// 4 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint32_t ObjectID = pkt.player_id();

	int32_t  animation_upper_idx = pkt.animation_upper_index();
	int32_t  animation_lower_idx = pkt.animation_lower_index();
	float    animation_param_h = pkt.animation_param_h();
	float    animation_param_v = pkt.animation_param_v();

	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation> EventData = CLIENT_NETWORK->CreateEvent_UpdateAnimation_RemotePlayer(ObjectID
		, animation_upper_idx
		, animation_lower_idx
		, animation_param_h
		, animation_param_v);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_Weapon(SPtr_Session session, const FBProtocol::SPkt_Player_Weapon& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Player_Weapon
	/// {
	/// 	player_id: uint;			// 8 bytes
	/// 	weapon_type: WEAPON_TYPE;	// 1 byte
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint32_t				player_id = pkt.player_id();
	FBProtocol::ITEM_TYPE weapon_type = pkt.weapon_type();

	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon> EventData = CLIENT_NETWORK->CreateEvent_UpdateWeapon_RemotePlayer(player_id, weapon_type);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_OnSkill(SPtr_Session session, const FBProtocol::SPkt_PlayerOnSkill& pkt)
{
	int								player_id = pkt.player_id();
	float							phero_amount = pkt.phero_amount();
	FBProtocol::PLAYER_SKILL_TYPE	skill_type = pkt.skill_type();
	int mindcontrol_monster_id = pkt.mindcontrol_monster_id();
	
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill> EventData = CLIENT_NETWORK->CreateEvent_UpdateOnSkill_RemotePlayer(player_id, skill_type, phero_amount, mindcontrol_monster_id);
	CLIENT_NETWORK->RegisterEvent(EventData);

	LOG_MGR->Cout(player_id, " : ", static_cast<int>(skill_type));

	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_AimRotation(SPtr_Session session, const FBProtocol::SPkt_Player_AimRotation& pkt)
{
	uint32_t player_id = pkt.player_id();
	float aim_rotation = pkt.aim_rotation();
	float spine_angle = pkt.spine_angle();

	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation> EventData = CLIENT_NETWORK->CreateEvent_UpdateAimRotation_RemotePlayer(
		player_id, aim_rotation, spine_angle);

	CLIENT_NETWORK->RegisterEvent(EventData);

	return false;
}

bool FBsPacketFactory::Process_SPkt_Player_State(SPtr_Session session, const FBProtocol::SPkt_Player_State& pkt)
{
	uint32_t					  player_id = pkt.player_id();
	float						  hp = pkt.hp();
	float						  phero = pkt.phero();
	FBProtocol::PLAYER_STATE_TYPE state_type = pkt.state_type();

	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateState> EventData = CLIENT_NETWORK->CreateEvent_UpdateState_RemotePlayer(
		player_id, state_type, hp, phero);

	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ MONSTER ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★



bool FBsPacketFactory::Process_SPkt_NewMonster(SPtr_Session session, const FBProtocol::SPkt_NewMonster& pkt)
{
	auto monsters = pkt.new_monsters();

	std::vector<GameMonsterInfo> infos;
	int newMonstersCnt = pkt.new_monsters()->size();
	for (UINT16 i = 0; i < newMonstersCnt; ++i) {
		auto info = GetMonsterInfo(pkt.new_monsters()->Get(i));
		std::cout << "Packet New Monster" << info.Id << "\n";
		infos.push_back(info);
	}
	sptr<NetworkEvent::Game::Event_Monster::Add> Ext_EventData = CLIENT_NETWORK->CreateEvent_Add_Monster(infos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);
	return true;
}

bool FBsPacketFactory::Process_SPkt_RemoveMonster(SPtr_Session session, const FBProtocol::SPkt_RemoveMonster& pkt)
{
	auto monster_id = pkt.monster_id();

	// TODO : 여러개를 한번에 하는 걸로 바꾸기
	std::vector<uint32_t> remInfos;
	remInfos.push_back(monster_id);

	sptr<NetworkEvent::Game::Event_Monster::Remove> Ext_EventData = CLIENT_NETWORK->CreateEvent_Remove_Monster(remInfos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);
	return true;
}

bool FBsPacketFactory::Process_SPkt_Monster_Transform(SPtr_Session session, const FBProtocol::SPkt_Monster_Transform& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Monster_Transform
	/// {
	/// 	monster_id: ulong;	// 4 bytes
	/// 	trans: Transform;				// 24 bytes (Vector3 * 2)
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint32_t monster_id = pkt.monster_id();

	Vec3	 Position = GetPosition_Vec2(pkt.pos_2());
	float	 Angle = pkt.rot_y();

	std::vector<NetworkEvent::Game::Event_Monster::MonsterMove> infos;

	NetworkEvent::Game::Event_Monster::MonsterMove info;
	info.Id = monster_id;
	info.Pos = Position;
	info.Angle = Angle;
	infos.push_back(info);

	sptr<NetworkEvent::Game::Event_Monster::Move> Ext_EventData = CLIENT_NETWORK->CreateEvent_Move_Monster(infos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);
	return true;
}

bool FBsPacketFactory::Process_SPkt_Monster_HP(SPtr_Session session, const FBProtocol::SPkt_Monster_HP& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Monster_HP
	/// {
	/// 	monster_id: ulong;		// 4 bytes
	/// 	hp: float;		// 4 bytes
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint32_t monster_id = pkt.monster_id();
	float	 monster_hp = pkt.hp();


	return true;
}

bool FBsPacketFactory::Process_SPkt_Monster_State(SPtr_Session session, const FBProtocol::SPkt_Monster_State& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Monster_State
	/// > {
	/// > 	monster_id: ulong;					// 4 bytes
	/// > 	state: MONSTER_STATE_TYPE;	// 1 byte
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○

	uint32_t						monster_id	= pkt.monster_id();
	FBProtocol::MONSTER_BT_TYPE		state_type	= pkt.monster_bt_type();
	int32_t							bt_step		= pkt.monster_bt_step();

	//LOG_MGR->Cout(monster_id, " - ", "Process SPkt Monster State : ", static_cast<int>(state_type), "\n");

	std::vector<NetworkEvent::Game::Event_Monster::MonsterUpdateState> infos;
	NetworkEvent::Game::Event_Monster::MonsterUpdateState info;
	info.Id = monster_id;
	info.state = state_type;
	info.step = bt_step;
	infos.push_back(info);

	sptr<NetworkEvent::Game::Event_Monster::UpdateState> Ext_EventData = CLIENT_NETWORK->CreateEvent_UpdateState_Monster(infos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);


	return true;
}

bool FBsPacketFactory::Process_SPkt_Monster_Target(SPtr_Session session, const FBProtocol::SPkt_MonsterTarget& pkt)
{
	int monster_id = pkt.monster_id();
	int target_monster_id = pkt.target_montser_id();
	int target_player_id = pkt.target_player_id();

	std::vector<NetworkEvent::Game::Event_Monster::MonsterTarget> infos;
	NetworkEvent::Game::Event_Monster::MonsterTarget info;
	info.id = monster_id;
	info.target_monster_id = target_monster_id;
	info.target_player_id = target_player_id;
	infos.push_back(info);

	LOG_MGR->Cout("Target : ", info.id, " - ", info.target_monster_id, " - ", info.target_player_id, "\n");

	sptr<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate> Ext_EventData = CLIENT_NETWORK->CreateEvent_Monster_Target(infos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);

	return true;

}

bool FBsPacketFactory::Process_SPkt_DeadMonster(SPtr_Session session, const FBProtocol::SPkt_DeadMonster& pkt)
{
	Vec3		monster_DeadPoint = GetPosition_Vec2(pkt.dead_point());
	uint32_t	monster_id        = pkt.monster_id();
	std::string pheros			  = pkt.pheros()->c_str();

	sptr<NetworkEvent::Game::Event_Monster::MonsterDead> Ext_EventData = CLIENT_NETWORK->CreateEvent_Dead_Monster(monster_id, monster_DeadPoint, pheros);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);

	return true;
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ PHERO ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_GetPhero(SPtr_Session session, const FBProtocol::SPkt_GetPhero& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_GetPhero
	/// > {
	/// > 	phero_id: uint;
	/// > 	player_id: uint;
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	
	int phero_id	= pkt.phero_id();
	int player_id	= pkt.player_id();

	sptr<NetworkEvent::Game::Event_Phero::GetPhero> Ext_EventData = CLIENT_NETWORK->CreateEvent_GetPhero(player_id, phero_id);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);

	return true;
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ BULLET ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_Bullet_OnShoot(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnShoot& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Bullet_OnShoot
	/// > {
	/// > 	player_id	: uint;			// 4 bytes - 어떤 플레이어가 
	/// > 	gun_id		: int;			// 4 bytes - 어떤 총이고 
	/// > 	bullet_id	: int;			// 4 bytes - 어떤 총알을 쐈는가
	/// > 
	/// > 	ray			: Vector3;		// 12 bytes (4bytes * 3) - 총구 방향은 어떠한가? 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○

	int  bullet_id      = pkt.bullet_id();
	int  gun_id         = pkt.gun_id();
	int  player_id      = pkt.player_id();
	Vec3& fire_dir      = GetVector3(pkt.fire_dir());
	Vec3& fire_pos      = GetVector3(pkt.fire_pos());

	std::cout << "Client : ON SHOOT(" << player_id << ") ray : " << fire_dir.x << ", " << fire_dir.y << ", " << fire_dir.z << "\n";
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot> Ext_EventData = CLIENT_NETWORK->CreateEvent_UpdateOnShoot_RemotePlayer(player_id, bullet_id, gun_id, fire_pos, fire_dir);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Bullet_OnHitEnemy(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnHitEnemy& pkt)
{
	int  bullet_id = pkt.bullet_id();
	int  gun_id = pkt.gun_id();
	int  player_id = pkt.player_id();
	Vec3& ray = GetVector3(pkt.ray());

	return true;
}

bool FBsPacketFactory::Process_SPkt_Bullet_OnCollision(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnCollision& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table SPkt_Bullet_OnCollision
	/// > {
	/// > 	player_id	: int;  // 4 bytes - 어떤 플레이어가 
	/// > 	gun_id		: int;	// 4 bytes - 어떤 총이고 
	/// > 	bullet_id	: int;  // 4 bytes - 어떤 총알이 충돌했는가?
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○

	return true;
}

bool FBsPacketFactory::Process_SPkt_Item_Interact(SPtr_Session session, const FBProtocol::SPkt_Item_Interact& pkt)
{
	uint32_t player_id              = pkt.player_id();
	uint32_t item_id                = pkt.item_id();
	FBProtocol::ITEM_TYPE item_type = pkt.item_type();
	Vec3 drop_pos					= GetVector3(pkt.drop_pos());

	LOG_MGR->Cout("Process_SPkt_Item_Interact :", player_id, " -- ", item_id, " -- ", static_cast<int>(item_type), "\n");

	sptr<NetworkEvent::Game::Event_Item::Item_Interact> Ext_EventData = CLIENT_NETWORK->CreateEvent_Item_Interact(player_id,item_id, item_type, drop_pos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);
	return true;
}

bool FBsPacketFactory::Process_SPkt_Item_ThrowAway(SPtr_Session session, const FBProtocol::SPkt_Item_ThrowAway& pkt)
{
	uint32_t player_id              = pkt.player_id();
	uint32_t item_id                = pkt.item_id();
	FBProtocol::ITEM_TYPE item_type = pkt.item_type();
	Vec3 drop_pos                   = GetVector3(pkt.drop_pos());

	LOG_MGR->Cout("Process_SPkt_Item_ThrowAway :", player_id, " -- ", item_id, " -- ", static_cast<int>(item_type), "\n");


	sptr<NetworkEvent::Game::Event_Item::Item_ThrowAway> Ext_EventData = CLIENT_NETWORK->CreateEvent_Item_ThrowAway(player_id, item_id, item_type, drop_pos);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);

	return true;
}




/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ CREATE FLATBUFFERS CLIENT PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ LogIn, Chat, NetworkLatency ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_LogIn(std::string id, std::string password)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_LogIn
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto input_id       = builder.CreateString(id);
	auto input_password = builder.CreateString(password);

	/* CREATE LOG IN PACKET */
	auto ServerPacket = FBProtocol::CreateCPkt_LogIn(builder, input_id, input_password);
	builder.Finish(ServerPacket);

	/* Create SendBuffer */
	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_LogIn);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_EnterLobby(uint32_t player_id)
{
	flatbuffers::FlatBufferBuilder builder;

	auto ServerPacket = FBProtocol::CreateCPkt_EnterLobby(builder, player_id);

	builder.Finish(ServerPacket);
	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_EnterLobby);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Chat(UINT32 sessionID, std::string msg)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_Chat
	/// > {
	/// > 	message : string;	// 가변 크기
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto msgOffset = builder.CreateString(msg);
	auto ServerPacket = FBProtocol::CreateCPkt_Chat(builder, msgOffset);

	builder.Finish(ServerPacket);
	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Chat);
}


SPtr_SendPktBuf FBsPacketFactory::CPkt_NetworkLatency(long long timestamp)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_NetworkLatency
	/// > {
	/// > 	timestamp: long;	// 8 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;


	auto ServerPacket = FBProtocol::CreateCPkt_NetworkLatency(builder, timestamp);

	builder.Finish(ServerPacket);

	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_NetworkLatency);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Custom(std::string trooperskin)
{
	flatbuffers::FlatBufferBuilder builder;

	auto trooper = builder.CreateString(trooperskin);
	auto ServerPacket = FBProtocol::CreateCPkt_Custom(builder, trooper);

	builder.Finish(ServerPacket);

	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Custom);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_EnterGame(uint32_t player_id)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_EnterGame
	/// > {
	/// > 	player_id: uint;	// 8 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto enterGamePkt = FBProtocol::CreateCPkt_EnterGame(builder, player_id);
	builder.Finish(enterGamePkt);

	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_EnterGame);

}

SPtr_SendPktBuf FBsPacketFactory::CPkt_PlayGame()
{
	flatbuffers::FlatBufferBuilder builder;

	auto cpkt = FBProtocol::CreateCPkt_PlayGame(builder);
	builder.Finish(cpkt);

	return SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_PlayGame);
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ PLAYER ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_NewPlayer()
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_NewPlayer
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_NewPlayer(builder);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_NewPlayer);

	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_RemovePlayer(uint32_t removeSessionID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_RemovePlayer
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	int32_t id = static_cast<int32_t>(removeSessionID);
	auto ServerPacket = FBProtocol::CreateCPkt_RemovePlayer(builder);
	builder.Finish(ServerPacket);

	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_RemovePlayer);

	return sendBuffer;
}


SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Transform(Vec3 Pos, Vec3 Rot, int32_t movestate, Vec3 movedir, float velocity, Vec3 SpineLookDir, long long latency, float animparam_h, float animparam_v)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_Player_Transform
	/// > {
	/// > 	move_state: PLAYER_MOTION_STATE_TYPE; // 1 byte
	/// > 
	/// > 	latency: long;						  // 8 bytes
	/// > 	velocity: float;					  // 4 bytes
	/// > 	movedir: Vector3;					  // 12 bytes (3 * 4 bytes)
	/// > 	trans: Transform;				      // 24 bytes (Vector3 * 2)
	/// > 										  
	/// > 	spine_look: Vector3;				  // 12 bytes (3 * 4 bytes)
	/// > 	animparam_h: float;					  // 4 bytes
	/// > 	animparam_v: float;					  // 4 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	auto moveDir = FBProtocol::CreateVector3(builder, movedir.x, movedir.y, movedir.z);
	auto position = FBProtocol::CreateVector3(builder, Pos.x, Pos.y, Pos.z);
	auto rotation = FBProtocol::CreateVector3(builder, Rot.x, Rot.y, Rot.z);
	auto transform = FBProtocol::CreateTransform(builder, position, rotation);
	auto spine_look = FBProtocol::CreateVector3(builder, SpineLookDir.x, SpineLookDir.y, SpineLookDir.z);

	auto ServerPacket = FBProtocol::CreateCPkt_Player_Transform(builder, static_cast<FBProtocol::PLAYER_MOTION_STATE_TYPE>(movestate), latency, velocity, moveDir, transform, spine_look, animparam_h, animparam_v);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Player_Transform);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Animation(int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_Player_Animation
	/// > {
	/// > 	animation_upper_index	: int;		// 4 bytes
	/// > 	animation_lower_index	: int;		// 4 bytes
	/// > 	animation_param_h		: float;	// 4 bytes
	/// > 	animation_param_v		: float;	// 4 bytes
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	int32_t animation_upper_index = static_cast<int32_t>(anim_upper_idx);
	int32_t animation_lower_index = static_cast<int32_t>(anim_lower_idx);
	float   animation_param_h = static_cast<float>(anim_param_h);
	float   animation_param_v = static_cast<float>(anim_param_v);

	auto ServerPacket = FBProtocol::CreateCPkt_Player_Animation(builder, animation_upper_index, animation_lower_index, animation_param_h, animation_param_v);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Player_Animation);
	return sendBuffer;

}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Weapon(uint32_t item_id, FBProtocol::ITEM_TYPE weaponType)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_Player_Weapon
	/// > {
	/// > 	weapon_type: WEAPON_TYPE;	// 1 byte
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};


	auto ServerPacket = FBProtocol::CreateCPkt_Player_Weapon(builder, item_id, weaponType);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Player_Weapon);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Weapon(uint32_t item_id, WeaponName weaponName)
{
	switch (weaponName)
	{
	case WeaponName::H_Lock:
		return CPkt_Player_Weapon(item_id ,FBProtocol::ITEM_TYPE_WEAPON_H_LOOK);
	case WeaponName::DBMS:
		return CPkt_Player_Weapon(item_id, FBProtocol::ITEM_TYPE_WEAPON_DBMS);
	case WeaponName::SkyLine:
		return CPkt_Player_Weapon(item_id, FBProtocol::ITEM_TYPE_WEAPON_SKYLINE);
	case WeaponName::Burnout:
		return CPkt_Player_Weapon(item_id, FBProtocol::ITEM_TYPE_WEAPON_BURNOUT);
	case WeaponName::PipeLine:
		return CPkt_Player_Weapon(item_id, FBProtocol::ITEM_TYPE_WEAPON_PIPELINE);
	case WeaponName::MineLauncher:
		return CPkt_Player_Weapon(item_id, FBProtocol::ITEM_TYPE_WEAPON_MINE_LAUNCHER);
	default:
		break;
	}
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_OnSkill(FBProtocol::PLAYER_SKILL_TYPE skillType, int mindcontrol_monster_id)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_PlayerOnSkill(builder, skillType, mindcontrol_monster_id);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_PlayerOnSkill);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_AimRotation(float aim_rotation_y, float spine_angle)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_Player_AimRotation(builder, aim_rotation_y, spine_angle);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Player_AimRotation);
	return sendBuffer;
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ MONSTER ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_NewMonster(uint32_t monster_id, FBProtocol::MONSTER_TYPE montser_type)
{
	///  ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_NewMonster
	/// > {
	/// > 
	/// > }
	///  ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○


	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_RemoveMonster(uint32_t monsterID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_NewMonster
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Monster_Transform(uint32_t monsterID, Vec3 Pos, Vec3 Rot)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_Monster_Transform
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Monster_HP(uint32_t monsterID, float hp)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Monster_HP
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Monster_State(uint32_t monsterID, FBProtocol::MONSTER_BT_TYPE state)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○`
	/// table CPkt_Monster_State
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○`
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_DeadMonster(uint32_t monsterID, Vec2 deadPoint)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// > table CPkt_DeadMonster
	/// > {
	/// > 
	/// > }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ BULLET ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_GetPhero(uint32_t phero_id, uint32_t player_id)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Bullet_OnShoot
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnShoot(Vec3 pos, Vec3 ray)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Bullet_OnShoot
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	auto RayDir			= FBProtocol::CreateVector3(builder, ray.x, ray.y, ray.z);
	auto position		= FBProtocol::CreateVector3(builder, pos.x, pos.y, pos.z);
	auto ServerPacket	= FBProtocol::CreateCPkt_Bullet_OnShoot(builder, position, RayDir);

	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Bullet_OnShoot);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnHitEnemy(int32_t monster_id, Vec3 fire_pos, Vec3 ray)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto RayDir = FBProtocol::CreateVector3(builder, ray.x, ray.y, ray.z);
	auto firePos = FBProtocol::CreateVector3(builder, fire_pos.x, fire_pos.y, fire_pos.z);


	auto ServerPacket = FBProtocol::CreateCPkt_Bullet_OnHitEnemy(builder, monster_id, firePos, RayDir);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Bullet_OnHitEnemy);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnHitExpEnemy(int32_t monster_id)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_Bullet_OnHitEnemy(builder, monster_id);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Bullet_OnHitExpEnemy);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnCollision(uint32_t playerID, uint32_t gunID, uint32_t bulletID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Bullet_OnCollisio
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Item_ThrowAway(uint32_t item_id, FBProtocol::ITEM_TYPE item_type)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_Item_ThrowAway(builder, item_id, item_type);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Item_ThrowAway);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Item_Interact(uint32_t item_id, FBProtocol::ITEM_TYPE item_type)
{
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_Item_Interact(builder, item_id, item_type);
	builder.Finish(ServerPacket);
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(builder.GetBufferPointer(), static_cast<uint16_t>(builder.GetSize()), FBProtocol::FBsProtocolID::FBsProtocolID_CPkt_Item_Interact);
	return sendBuffer;
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	▶ UTILITY  
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

GamePlayerInfo FBsPacketFactory::GetPlayerInfo(const FBProtocol::Player* player)
{
	GamePlayerInfo info = {};

	info.Id = player->id();
	info.Name = player->name()->c_str();

	const FBProtocol::Vector3* pos = player->trans()->position();
	info.Pos = Vec3(pos->x(), pos->y(), pos->z());

	const FBProtocol::Vector3* Rot = player->trans()->rotation();
	info.Rot = Vec3(Rot->x(), Rot->y(), Rot->z());

	const FBProtocol::Vector3* SDir = player->spine_look();
	info.SDir = Vec3(SDir->x(), SDir->y(), SDir->z());

	return info;
}

GameMonsterInfo FBsPacketFactory::GetMonsterInfo(const FBProtocol::Monster* monster)
{
	GameMonsterInfo info = {};

	info.Id			= monster->id();
	info.Type		= monster->type();
	info.bt_type	= monster->bt_type();

	info.Pos		= GetPosition_Vec2(monster->pos_2());
	float rot_y		= monster->rot_y();
	info.Rot		= GetRot_y(rot_y);

	info.Target_Player_Id = monster->target_player_id();




	return info;
}

FBProtocol::ITEM_TYPE FBsPacketFactory::GetItemType(ItemType type)
{
	switch (type)
	{
	case ItemType::None:
		break;
	case ItemType::WeaponCrate:
		return FBProtocol::ITEM_TYPE_STATIC_ITEM_CRATE;
		break;
	case ItemType::Weapon:
		break;
	default:
		break;
	}

	
}

Vec3 FBsPacketFactory::GetVector3(const FBProtocol::Vector3* vec3)
{
	Vec3 Vector3 = Vec3(vec3->x(), vec3->y(), vec3->z());

	return Vector3;
}

Vec4 FBsPacketFactory::GetVector4(const FBProtocol::Vector4* vec4)
{
	Vec4 Vector4 = Vec4(vec4->x(), vec4->y(), vec4->z(), vec4->w());

	return Vector4;
}

Vec3 FBsPacketFactory::GetPosition_Vec2(const FBProtocol::Position_Vec2* vec2)
{
	Vec3 Position_xz = Vec3(vec2->x(), 0.f, vec2->z());
	return Position_xz;
}

Vec4 FBsPacketFactory::GetRot_y(const float rot_y)
{
	return Vec3(0, rot_y, 0).ToQuaternion();
}




Vec3 FBsPacketFactory::CalculateDirection(float yAngleRadian)
{
	// x 및 z 방향 벡터 계산
	float xDir = std::sin(yAngleRadian);
	float zDir = std::cos(yAngleRadian);

	Vec3 dir = Vec3(xDir, 0.0f, zDir); // y 방향은 고려하지 않음
	dir.Normalize();
	return dir;
}

Vec3 FBsPacketFactory::lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam)
{
	return Vec3((1.0f - PosLerpParam) * CurrPos.x + PosLerpParam * TargetPos.x,
		(1.0f - PosLerpParam) * CurrPos.y + PosLerpParam * TargetPos.y,
		(1.0f - PosLerpParam) * CurrPos.z + PosLerpParam * TargetPos.z);
}


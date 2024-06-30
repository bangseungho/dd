#include "stdafx.h"
#include "FBsPacketFactory.h"
#include "ClientNetwork/Include/PacketHeader.h"

/* RELATED FLAT BUFFERS HEADER */
#undef max
#include "ClientNetwork/Include/SendBuffersFactory.h"
#include "ClientNetwork/Include/SocketData.h"

#include "ClientNetwork/Contents/Script_PlayerNetwork.h"
#include "ClientNetwork/Contents/Script_RemotePlayer.h"

#include "ServerSession.h"
#include "NetworkEvents.h"
#include "ClientNetworkManager.h"
#include "GameFramework.h"

DEFINE_SINGLETON(FBsPacketFactory);

std::atomic<long long> FBsPacketFactory::TotalLatency = 0;
std::atomic<int> FBsPacketFactory::LatencyCount       = 0;
std::atomic<long long> FBsPacketFactory::CurrLatency         = 0;



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
	case FBsProtocolID::SPkt_LogIn:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_LogIn ]\n");
		const FBProtocol::SPkt_LogIn* packet = flatbuffers::GetRoot<FBProtocol::SPkt_LogIn>(DataPtr);
		if (!packet) return false;
		Process_SPkt_LogIn(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_EnterGame:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_EnterGame ]\n");
		const FBProtocol::SPkt_EnterGame* packet = flatbuffers::GetRoot<FBProtocol::SPkt_EnterGame>(DataPtr);
		if (!packet) return false;
		Process_SPkt_EnterGame(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Chat:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_Chat ]\n");
		const FBProtocol::SPkt_Chat* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Chat>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Chat(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_NetworkLatency:
	{
		//LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_NetworkLatency ]\n");
		const FBProtocol::SPkt_NetworkLatency* packet = flatbuffers::GetRoot<FBProtocol::SPkt_NetworkLatency>(DataPtr);
		if (!packet) return false;
		Process_SPkt_NetworkLatency(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Player_Transform:
	{
		//LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_Transform ]\n");
		const FBProtocol::SPkt_Player_Transform* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_Transform>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_Transform(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_NewPlayer:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_NewPlayer ]\n");
		const FBProtocol::SPkt_NewPlayer* packet = flatbuffers::GetRoot<FBProtocol::SPkt_NewPlayer>(DataPtr);
		if (!packet) return false;
		Process_SPkt_NewPlayer(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_RemovePlayer:
	{
		LOG_MGR->Cout(session->GetID(), " - RECV - ", "[ SPkt_RemovePlayer ]\n");
		const FBProtocol::SPkt_RemovePlayer* packet = flatbuffers::GetRoot<FBProtocol::SPkt_RemovePlayer>(DataPtr);
		if (!packet) return false;
		Process_SPkt_RemovePlayer(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Player_Animation:
	{
		const FBProtocol::SPkt_Player_Animation* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Player_Animation>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Player_Animation(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Monster_Transform:
	{
		const FBProtocol::SPkt_Monster_Transform* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_Transform>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_Transform(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Monster_HP:
	{
		const FBProtocol::SPkt_Monster_HP* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_HP>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_HP(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Monster_State:
	{
		const FBProtocol::SPkt_Monster_State* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Monster_State>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Monster_State(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Bullet_OnShoot:
	{
		const FBProtocol::SPkt_Bullet_OnShoot* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Bullet_OnShoot>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Bullet_OnShoot(session, *packet);
	}
	break;
	case FBsProtocolID::SPkt_Bullet_OnCollision:
	{
		const FBProtocol::SPkt_Bullet_OnCollision* packet = flatbuffers::GetRoot<FBProtocol::SPkt_Bullet_OnCollision>(DataPtr);
		if (!packet) return false;
		Process_SPkt_Bullet_OnCollision(session, *packet);
	}
	break;
	default:
	{

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

	GamePlayerInfo MyInfo = GetPlayerInfo(pkt.myinfo());
	GameFramework::I->InitPlayer(static_cast<int>(MyInfo.Id)); /* INIT PLAYER */


	LOG_MGR->SetColor(TextColor::BrightGreen);
	LOG_MGR->Cout("♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠\n");
	LOG_MGR->Cout("[MY] NAME : ", MyInfo.Name, " ", " SESSION ID : ", MyInfo.Id, '\n');
	LOG_MGR->Cout("♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠♠\n");
	LOG_MGR->SetColor(TextColor::Default);

	int PlayersCnt = pkt.players()->size();
	for (UINT16 i = 0; i < PlayersCnt; ++i) {
		GamePlayerInfo RemoteInfo = GetPlayerInfo(pkt.players()->Get(i));

		if (RemoteInfo.Id == MyInfo.Id) continue;
		LOG_MGR->SetColor(TextColor::BrightGreen);
		LOG_MGR->Cout("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
		LOG_MGR->Cout("[REMOTE] NAME : ", RemoteInfo.Name, " ", " SESSION ID : ", RemoteInfo.Id, '\n');
		LOG_MGR->Cout("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
		LOG_MGR->SetColor(TextColor::Default);

		sptr<NetworkEvent::Game::Add_RemotePlayer> EventData = CLIENT_NETWORK->CreateEvent_Add_RemotePlayer(RemoteInfo);
		CLIENT_NETWORK->RegisterEvent(EventData);
	}

	auto CPkt = FBS_FACTORY->CPkt_EnterGame(0); /* 0 : 의미없음 */
	session->Send(CPkt);

	return true;
}

bool FBsPacketFactory::Process_SPkt_EnterGame(SPtr_Session session, const FBProtocol::SPkt_EnterGame& pkt)
{

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_EnterGame
	/// {
	/// 	success: bool;			// 1 byte
	/// }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○

	return true;
}


bool FBsPacketFactory::Process_SPkt_Chat(SPtr_Session session, const FBProtocol::SPkt_Chat& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Chat
	/// {
	/// 	player_id: uint;		// uint64
	/// 	message: string;	// 가변 크기
	/// }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	std::cout << "SPkt CHAT [" << session->GetID() << "] - SESSION : " << session.get() << " DATA : " <<
		pkt.message()->c_str() << std::endl;
	return true;
}

bool FBsPacketFactory::Process_SPkt_NetworkLatency(SPtr_Session session, const FBProtocol::SPkt_NetworkLatency& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_NetworkLatency
	/// {
	/// 	timestamp: long;	// 8 bytes
	/// }
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
	/// table SPkt_NewPlayer
	/// {
	/// 	newplayer: Player; // 새로운 플레이어가 접속했음을 기존의 세션들에게 알린다. 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	GamePlayerInfo NewPInfo = GetPlayerInfo(pkt.newplayer());

	LOG_MGR->SetColor(TextColor::BrightGreen);
	LOG_MGR->Cout("▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣\n");
	LOG_MGR->Cout("[NEW REMOTE] NAME : ", NewPInfo.Name, " ", " SESSION ID : ", NewPInfo.Id, '\n');
	LOG_MGR->Cout("▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣▣\n");
	LOG_MGR->SetColor(TextColor::Default);

	sptr<NetworkEvent::Game::Add_RemotePlayer> EventData = CLIENT_NETWORK->CreateEvent_Add_RemotePlayer(NewPInfo);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_RemovePlayer(SPtr_Session session, const FBProtocol::SPkt_RemovePlayer& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_RemovePlayer
	/// {
	/// 	player_id: uint; // 4 bytes // 삭제할 플레이어의 아이디 
	/// }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	int32_t removeID = pkt.player_id();

	LOG_MGR->SetColor(TextColor::BrightRed);
	LOG_MGR->Cout("▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷\n");
	LOG_MGR->Cout("[REMOVE REMOTE] NAME SESSION ID : ", removeID, '\n');
	LOG_MGR->Cout("▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷▷\n");
	LOG_MGR->SetColor(TextColor::Default);

	sptr<NetworkEvent::Game::Remove_RemotePlayer> EventData = CLIENT_NETWORK->CreateEvent_Remove_RemotePlayer(removeID);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_Transform(SPtr_Session session, const FBProtocol::SPkt_Player_Transform& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Player_Transform
	/// {
	/// 	player_id: uint;					// 8 bytes				// uint64
	/// 
	/// 	move_state: PLAYER_MOTION_STATE_TYPE; // 1 byte
	/// 
	/// 	latency: long;						// 8 bytes
	/// 	velocity: float;					// 4 bytes
	/// 	movedir: Vector3;					// 12 bytes (3 * 4 bytes)
	/// 	trans: Transform;				// 24 bytes (Vector3 * 2)
	/// 
	/// 
	/// 	spine_look: Vector3;					// 12 bytes (3 * 4 bytes)
	/// 	animparam_h: float;					// 4 bytes
	/// 	animparam_v: float;					// 4 bytes
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	long long latency = pkt.latency();
	uint64_t id = pkt.player_id();


	float	vel       = pkt.velocity();
	Vec3	moveDir   = GetVector3(pkt.movedir());
	Vec3	Packetpos = GetVector3(pkt.trans()->position());
	Vec3	rot       = GetVector3(pkt.trans()->rotation());
	int32_t movestate = pkt.move_state();
	Vec3	SDir      = GetVector3(pkt.spine_look());

	float   animparam_h = pkt.animparam_h();
	float   animparam_v = pkt.animparam_v();

	ExtData::MOVESTATE mState;
	if (movestate == PLAYER_MOVE_STATE::Start)			mState = ExtData::MOVESTATE::Start;
	else if (movestate == PLAYER_MOVE_STATE::Progress)	mState = ExtData::MOVESTATE::Progress;
	else if (movestate == PLAYER_MOVE_STATE::End)		mState = ExtData::MOVESTATE::End;


	sptr<NetworkEvent::Game::Move_RemotePlayer> Move_EventData = CLIENT_NETWORK->CreateEvent_Move_RemotePlayer(id, Packetpos, mState);
	CLIENT_NETWORK->RegisterEvent(Move_EventData);

	//LOG_MGR->Cout("MOVE DIR PKT : ", moveDir.x, " ", moveDir.y, " ", moveDir.z, '\n');

	/// +---------------------------
	///	Extrapolate Next Packet Pos 
	/// ---------------------------+

	/* CurrPos --------------- PacketPos --------------------------> TargetPos */

	ExtData data                  = {};
	/* [Get Next Packet Duration] = (PKt Interval) + (Remote Cl Latency) + (My Latency) */
	data.PingTime                 = (PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000) + (latency / 1000.0) + (CurrLatency.load() / 1000.0);
	data.MoveDir                  = moveDir;
	data.MoveState                = mState;
	//LOG_MGR->Cout(data.PingTime, " ", data.PingTime / 1000.0, '\n');

	/* 위치 예측 ( TargetPos ) */
	data.TargetPos.x = static_cast<float>(Packetpos.x + (data.MoveDir.x * vel * ((data.PingTime) / 1000.0)));
	data.TargetPos.z = static_cast<float>(Packetpos.z + (data.MoveDir.z * vel * ((data.PingTime) / 1000.0)));

	data.TargetRot = rot;
	data.Velocity = vel;

	data.Animdata.AnimParam_h = animparam_h;
	data.Animdata.AnimParam_v = animparam_v;

	sptr<NetworkEvent::Game::Extrapolate_RemotePlayer> Ext_EventData = CLIENT_NETWORK->CreateEvent_Extrapolate_RemotePlayer(id, data);
	CLIENT_NETWORK->RegisterEvent(Ext_EventData);


	return true;
}

bool FBsPacketFactory::Process_SPkt_Player_Animation(SPtr_Session session, const FBProtocol::SPkt_Player_Animation& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Player_Animation
	/// {
	/// 	player_id: uint;	// 8 bytes
	/// 
	/// 	animation_upper_index: int;		// 4 bytes
	/// 	animation_lower_index: int;		// 4 bytes
	/// 	animation_param_h: float;	// 4 bytes
	/// 	animation_param_v: float;	// 4 bytes
	/// }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	uint64_t ObjectID = pkt.player_id();
	int32_t animation_upper_idx = pkt.animation_upper_index();
	int32_t animation_lower_idx = pkt.animation_lower_index();
	float animation_param_h = pkt.animation_param_h();
	float animation_param_v = pkt.animation_param_v();

	sptr<NetworkEvent::Game::ChangeAnimation_RemotePlayer> EventData = CLIENT_NETWORK->CreateEvent_ChangeAnimation_RemotePlayer(ObjectID
		, animation_upper_idx
		, animation_lower_idx
		, animation_param_h
		, animation_param_v);
	CLIENT_NETWORK->RegisterEvent(EventData);

	return false;
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
	return false;
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ MONSTER ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_Monster_Transform(SPtr_Session session, const FBProtocol::SPkt_Monster_Transform& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Monster_Transform
	/// {
	/// 	monster_id: ulong;	// 4 bytes
	/// 	trans: Transform;				// 24 bytes (Vector3 * 2)
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return false;
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
	return false;
}

bool FBsPacketFactory::Process_SPkt_Monster_State(SPtr_Session session, const FBProtocol::SPkt_Monster_State& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Monster_State
	/// {
	/// 	monster_id: ulong;					// 4 bytes
	/// 	state: MONSTER_STATE_TYPE;	// 1 byte
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return false;
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ PROCESS [ BULLET ] Server PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

bool FBsPacketFactory::Process_SPkt_Bullet_OnShoot(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnShoot& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Bullet_OnShoot
	/// {
	/// 	player_id: uint; // 4 bytes - 어떤 플레이어가 
	/// 	gun_id: int; // 4 bytes - 어떤 총이고 
	/// 	bullet_id: int; // 4 bytes - 어떤 총알을 쐈는가
	/// 
	/// 	ray: Vector3; // 12 bytes (4bytes * 3) - 총구 방향은 어떠한가? 
	/// }

	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return false;
}

bool FBsPacketFactory::Process_SPkt_Bullet_OnCollision(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnCollision& pkt)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Bullet_OnCollision
	/// {
	/// 	player_id: int; // 4 bytes - 어떤 플레이어가 
	/// 	gun_id: int; // 4 bytes - 어떤 총이고 
	/// 	bullet_id: int; // 4 bytes - 어떤 총알이 충돌했는가?
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return false;
}




/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ CREATE FLATBUFFERS CLIENT PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ LogIn, Chat, NetworkLatency ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_LogIn()
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_LogIn
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	/* CREATE LOG IN PACKET */
	auto ServerPacket = FBProtocol::CreateCPkt_LogIn(builder);
	builder.Finish(ServerPacket);

	/* Create SendBuffer */
	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;

	return SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_LogIn);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Chat(UINT32 sessionID, std::string msg)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Chat
	/// {
	/// 	message: string;	// 가변 크기
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto msgOffset = builder.CreateString(msg);
	auto ServerPacket = FBProtocol::CreateCPkt_Chat(builder, msgOffset);

	builder.Finish(ServerPacket);

	const uint8_t* bufferPtr = builder.GetBufferPointer();
	const uint16_t serializedDataSize = static_cast<uint16_t>(builder.GetSize());

	return SENDBUF_FACTORY->CreatePacket(bufferPtr, serializedDataSize, FBsProtocolID::CPkt_Chat);
}


SPtr_SendPktBuf FBsPacketFactory::CPkt_NetworkLatency(long long timestamp)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_NetworkLatency
	/// {
	/// 	timestamp: long;	// 8 bytes
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto ServerPacket = FBProtocol::CreateCPkt_NetworkLatency(builder, timestamp);

	builder.Finish(ServerPacket);

	const uint8_t* bufferPtr = builder.GetBufferPointer();
	const uint16_t serializedDataSize = static_cast<uint16_t>(builder.GetSize());

	return SENDBUF_FACTORY->CreatePacket(bufferPtr, serializedDataSize, FBsProtocolID::CPkt_NetworkLatency);
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_EnterGame(uint32_t playerIdx)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_EnterGame
	/// {
	/// 	player_id: uint;	// 8 bytes
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder;

	auto enterGamePkt = FBProtocol::CreateCPkt_EnterGame(builder, playerIdx);
	builder.Finish(enterGamePkt);

	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;

	return SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_EnterGame);

}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ PLAYER ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_NewPlayer()
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_NewPlayer
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	auto ServerPacket = FBProtocol::CreateCPkt_NewPlayer(builder);
	builder.Finish(ServerPacket);

	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_NewPlayer);

	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_RemovePlayer(uint32_t removeSessionID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_RemovePlayer
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	int32_t id        = static_cast<int32_t>(removeSessionID);
	auto ServerPacket = FBProtocol::CreateCPkt_RemovePlayer(builder);
	builder.Finish(ServerPacket);

	const uint8_t* bufferPointer      = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer        = SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_RemovePlayer);

	return sendBuffer;
}


SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Transform(Vec3 Pos, Vec3 Rot, int32_t movestate, Vec3 movedir, float velocity, Vec3 SpineLookDir, long long latency, float animparam_h, float animparam_v)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Player_Transform
	/// {
	/// 	move_state: PLAYER_MOTION_STATE_TYPE; // 1 byte
	/// 
	/// 	latency: long;						// 8 bytes
	/// 	velocity: float;					// 4 bytes
	/// 	movedir: Vector3;					// 12 bytes (3 * 4 bytes)
	/// 	trans: Transform;				// 24 bytes (Vector3 * 2)
	/// 
	/// 	spine_look: Vector3;					// 12 bytes (3 * 4 bytes)
	/// 	animparam_h: float;					// 4 bytes
	/// 	animparam_v: float;					// 4 bytes
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	auto moveDir = FBProtocol::CreateVector3(builder, movedir.x, movedir.y, movedir.z);
	auto position = FBProtocol::CreateVector3(builder, Pos.x, Pos.y, Pos.z);
	auto rotation = FBProtocol::CreateVector3(builder, Rot.x, Rot.y, Rot.z);
	auto transform = FBProtocol::CreateTransform(builder, position, rotation);
	auto Spine_LookDir = FBProtocol::CreateVector3(builder, SpineLookDir.x, SpineLookDir.y, SpineLookDir.z);

	auto ServerPacket = FBProtocol::CreateCPkt_Player_Transform(builder, static_cast<FBProtocol::PLAYER_MOTION_STATE_TYPE>(movestate), latency, velocity, moveDir, transform, Spine_LookDir, animparam_h, animparam_v);
	builder.Finish(ServerPacket);


	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_Player_Transform);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Animation(int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Player_Animation
	/// {
	/// 	animation_upper_index: int;		// 4 bytes
	/// 	animation_lower_index: int;		// 4 bytes
	/// 	animation_param_h: float;	// 4 bytes
	/// 	animation_param_v: float;	// 4 bytes
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	flatbuffers::FlatBufferBuilder builder{};

	int32_t animation_upper_index = static_cast<int32_t>(anim_upper_idx);
	int32_t animation_lower_index = static_cast<int32_t>(anim_lower_idx);
	float animation_param_h = static_cast<float>(anim_param_h);
	float animation_param_v = static_cast<float>(anim_param_v);

	auto ServerPacket = FBProtocol::CreateCPkt_Player_Animation(builder, animation_upper_index, animation_lower_index, animation_param_h, animation_param_v);
	builder.Finish(ServerPacket);

	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer = SENDBUF_FACTORY->CreatePacket(bufferPointer, SerializeddataSize, FBsProtocolID::CPkt_Player_Animation);
	return sendBuffer;
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Player_Weapon(FBProtocol::WEAPON_TYPE weaponType)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Player_Weapon
	/// {
	/// 	weapon_type: WEAPON_TYPE;	// 1 byte
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ MONSTER ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_NewMonster()
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table SPkt_Monster_State
	/// {
	/// 	monster_id: int;					// 4 bytes
	/// 	state: MONSTER_STATE_TYPE;	// 1 byte
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_RemoveMonster(uint32_t monsterID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_NewMonster
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Monster_Transform(uint32_t monsterID, Vec3 Pos, Vec3 Rot)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Monster_Transform
	/// {
	/// 
	/// }
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

SPtr_SendPktBuf FBsPacketFactory::CPkt_Monster_State(uint32_t monsterID, FBProtocol::MONSTER_STATE_TYPE state)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Monster_State
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	◈ SEND [ BULLET ] PACKET ◈
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnShoot(uint32_t playerID, uint32_t gunID, uint32_t bulletID, Vec3 ray)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Bullet_OnShoot
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}

SPtr_SendPktBuf FBsPacketFactory::CPkt_Bullet_OnCollision(uint32_t playerID, uint32_t gunID, uint32_t bulletID)
{
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	/// table CPkt_Bullet_OnCollision
	/// {
	/// 
	/// }
	/// ○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○
	return SPtr_SendPktBuf();
}


/// ★---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///	▶ UTILITY  
/// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------★

GamePlayerInfo FBsPacketFactory::GetPlayerInfo(const FBProtocol::Player* player)
{
	GamePlayerInfo info = {};

	info.Id   = player->id();
	info.Name = player->name()->c_str();

	const FBProtocol::Vector3* pos  = player->trans()->position();
	info.Pos = Vec3(pos->x(), pos->y(), pos->z());

	const FBProtocol::Vector3* Rot  = player->trans()->rotation();
	info.Rot = Vec3(Rot->x(), Rot->y(), Rot->z());

	const FBProtocol::Vector3* SDir = player->spine_look();
	info.SDir = Vec3(SDir->x(), SDir->y(), SDir->z());

	return info;
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


#pragma once

/// +-------------------------------------------------
///				 Client Network Manager 
/// __________________________________________________
///		         Ŭ���̾�Ʈ ��Ʈ��ũ �Ŵ���
/// 
/// ��� : �������� ��� �� ���Ź��� ��Ŷ�� ó���ϴ� ���� 
///        Lock�� �ִ��� ���⼭�� �� ���� �����Ѵ�.  
/// (Lock�� �߱����� ������ ���߿� ���׿����� ã�� ������ �����.)
/// 
/// [ Front Events Queue ] -------> Process Events  ( GameLoop(main) Thread )
///		��
///    LOCK -- ( Change )
///		��
/// [ Back  Events Queue ] <------  Register Events ( Worker(Server) Threads)
/// -------------------------------------------------+
#undef max
#include "ClientNetwork/Contents/NetworkEvents.h"
#include "ClientNetwork/Contents/GamePlayer.h"
#include "ClientNetwork/Contents/Script_RemotePlayer.h"
#include "Scene.h"
#include "InputMgr.h"


#define CLIENT_NETWORK ClientNetworkManager::GetInst()
struct NetSceneEventQueue 
{
	Concurrency::concurrent_queue<sptr<NetworkEvent::Game::EventData>> EventsQueue{};
};


class ClientNetworkManager
{
	DECLARE_SINGLETON(ClientNetworkManager);

private:
	Lock::SRWLock		mSRWLock{};
	SPtr_ClientNetwork  mClientNetwork{};
	std::wstring		mServerIP		= L"127.0.0.1";

	Concurrency::concurrent_unordered_map<UINT32, sptr<GridObject>> mRemotePlayers{}; /* sessionID, RemotePlayer */
	NetSceneEventQueue	mSceneEvnetQueue[2];		// FRONT <-> BACK 
	std::atomic_int	    mFrontSceneEventIndex = 0;	// FRONT SCENE EVENT QUEUE INDEX 
	std::atomic_int	    mBackSceneEventIndex = 1;	// BACK SCENE EVENT QUEUE INDEX 

	int mAnimationIndex = -1;

public:
	ClientNetworkManager();
	~ClientNetworkManager();


public:
	void Init(std::wstring ip, UINT32 port);
	void Launch(int ThreadNum);

	void ProcessEvents();
	void SwapEventsQueue(); 
	void RegisterEvent(sptr<NetworkEvent::Game::EventData> data);


public:
	/* Send Client Packet */
	void Send(SPtr_PacketSendBuf pkt);

public:
	sptr<NetworkEvent::Game::Add_RemotePlayer>					CreateEvent_Add_RemotePlayer(GamePlayerInfo info);
	sptr<NetworkEvent::Game::Remove_RemotePlayer>				CreateEvent_Remove_RemotePlayer(int32_t remID);
	sptr<NetworkEvent::Game::Move_RemotePlayer>					CreateEvent_Move_RemotePlayer(int32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate);
	sptr<NetworkEvent::Game::Extrapolate_RemotePlayer>			CreateEvent_Extrapolate_RemotePlayer(int32_t remID, ExtData extdata);
	sptr<NetworkEvent::Game::ChangeAnimation_RemotePlayer>		CreateEvent_ChangeAnimation_RemotePlayer(int32_t remID, int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v);

	long long GetCurrentTimeMilliseconds();
	long long GetTimeStamp();


};

﻿#include "stdafx.h"
#include "GameFramework.h"

#include "Resource.h"
#include "X-Engine.h"

#include "ObjectMgr.h"
#include "InputMgr.h"
#include "Imgui/ImGuiMgr.h"

#include "Scene.h"
#include "Timer.h"
#include "Object.h"

#include "ObjectPool.h"

#include "Component/Camera.h"
#include "Component/ParticleSystem.h"


#include "Script_MainCamera.h"
#include "Script_Player.h"
#include "Script_GameManager.h"

#include "Log/LogMgr.h"
#include "FlatBuffers/ServerFBsPktFactory.h"
#include "ServerSession.h"
#include "IocpLibrary/include/Service.h"
#include "IocpLibrary/include/Session.h"
#include "IocpLibrary/include/SocketUtils.h"
#include "InputMgr.h"
#include "PacketFactory.h"

//#define SERVER_COMMUNICATION


HINSTANCE GameFramework::mhInst = nullptr;
HWND      GameFramework::mhWnd = nullptr;


GameFramework::GameFramework()
{

}

GameFramework::~GameFramework()
{
}

void GameFramework::KeyInputBroadcast()
{
#ifndef SERVER_COMMUNICATION
	return;
#endif
	Vec2 mouseDelta = InputMgr::I->GetMouseDelta();

	/* 키 입력은 메인쓰레드에서 동작해야한다. */
	/* TAP */
	UINT8 MoveKeyState = 0;
	if (KEY_PRESSED('A')) {
		std::string Chatting = "LEFT 키를 눌렀습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		MoveKeyState |= static_cast<UINT8>(GameKeyInfo::MoveKey::A);
		/*auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(KEY::A, KEY_STATE::PRESSED);
		mClientNetworkService->Broadcast(CPktBuf_Key);*/

	}
	if (KEY_PRESSED('D')) {
		std::string Chatting = "RIGHT 키를 눌렀습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		MoveKeyState |= static_cast<UINT8>(GameKeyInfo::MoveKey::D);

		/*auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(KEY::D, KEY_STATE::PRESSED);
		mClientNetworkService->Broadcast(CPktBuf_Key);*/


	}
	if (KEY_PRESSED('W')) {
		std::string Chatting = "UP 키를 눌렀습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		MoveKeyState |= static_cast<UINT8>(GameKeyInfo::MoveKey::W);

		/*auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(KEY::W, KEY_STATE::PRESSED);
		mClientNetworkService->Broadcast(CPktBuf_Key);*/


	}
	if (KEY_PRESSED('S')) {
		std::string Chatting = "DOWN 키를 눌렀습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		MoveKeyState |= static_cast<UINT8>(GameKeyInfo::MoveKey::S);

		/*auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(KEY::S, KEY_STATE::PRESSED);
		mClientNetworkService->Broadcast(CPktBuf_Key);*/


	}
	/* Send Move KEy 'wasd' */
	std::bitset<8> bits(MoveKeyState); // result 값을 비트로 변환합니다
	//std::cout << "KeyMove State = Result bits: " << bits << std::endl;


	auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::S
																   , GameKeyInfo::KEY_STATE::PRESSED
																   , static_cast<GameKeyInfo::MoveKey>(MoveKeyState)
																   , mouseDelta);

	mClientNetworkService->Broadcast(CPktBuf_Key);

	if (KEY_TAP(VK_SHIFT)) {
		std::string Chatting = "SHIFT 키를 눌렀습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::LSHFT, GameKeyInfo::KEY_STATE::TAP, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);


	}

	/* AWAY */
	if (KEY_AWAY(VK_SHIFT)) {
		std::string Chatting = "SHIFT 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::LSHFT, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);

	}
	if (KEY_AWAY('A')) {
		std::string Chatting = "LEFT 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::LSHFT, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);


	}
	if (KEY_AWAY('D')) {
		std::string Chatting = "RIGHT 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::D, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);
	}
	if (KEY_AWAY('W')) {
		std::string Chatting = "UP 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::W, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);
	}
	if (KEY_AWAY('S')) {
		std::string Chatting = "DOWN 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::S, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);
	}
	if (KEY_AWAY(VK_SHIFT)) {
		std::string Chatting = "SHIFT 키를 뗐습니다.\n";
		auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Chat(Chatting);
		mClientNetworkService->Broadcast(CPktBuf);

		auto CPktBuf_Key = PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY::LSHFT, GameKeyInfo::KEY_STATE::AWAY, static_cast<GameKeyInfo::MoveKey>(MoveKeyState), mouseDelta);
		mClientNetworkService->Broadcast(CPktBuf_Key);

	}

	
	
}

bool GameFramework::Init(HINSTANCE hInstance, short width, short height)
{
	/* mhWnd 초기화 */
	mhInst = hInstance;
	mResolution.Width = width;
	mResolution.Height = height;
	CreateGameClientWindow();

	// Init //
	MainCamera::I->AddComponent<Script_MainCamera>();
	Engine::I->Init(hInstance, mhWnd, static_cast<short>(width), static_cast<short>(height));
	GAME_MGR->AddComponent<Script_GameManager>();
	ObjectMgr::I->InitObjectsScript();
	InitPlayer();

#ifdef SERVER_COMMUNICATION
	// Communication //
	SocketUtils::Init();
	ServerFBsPktFactory::Init();
	/// +--------------------------------------------------------------
	///					        CLIENT NETWORK
	/// --------------------------------------------------------------+
	mClientNetworkService = MakeShared<ClientService>(
										NetAddress(L"127.0.0.1", 7777),
										MakeShared<Iocp>(),
										MakeShared<ServerSession>, // TODO : SessionManager 등
										1);
	mClientNetworkService->Start();
	THREAD_MGR->InitTLS();
#endif

	return true;
}

void GameFramework::Release()
{
	Engine::I->Release();
}




int GameFramework::GameLoop()
{
	static HACCEL hAccelTable = LoadAccelerators(mhInst, MAKEINTRESOURCE(IDC_XMACHINA));
	static MSG msg{};

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue; // ImGui 

			}
		}
		else
		{
			GameFramework::I->Update();
		}
	}

	::DestroyWindow(mhWnd);
	::UnregisterClass(L"X-MACHINA", mhInst);

	return (int)msg.wParam;
}

void GameFramework::Update()
{
	Timer::I->Tick();
	//Timer::I->Tick(60.f);
	
	Engine::I->Update();

	KeyInputBroadcast(); 
}


void GameFramework::Launch()
{
#ifdef SERVER_COMMUNICATION
	LogMgr::I->SetColor(TextColor::BrightCyan);
	LogMgr::I->Cout("+--------------------------------------\n");
	LogMgr::I->Cout("       X-MACHINA CLIENT NETWORK        \n");
	LogMgr::I->Cout("--------------------------------------+\n");
	LogMgr::I->SetColor(TextColor::Default);

	/* Server Thread */
	for (int32 i = 0; i < 2; i++)
	{
		THREAD_MGR->Launch([=]()
			{
				while (true)
				{
					mClientNetworkService->GetIocp()->Dispatch();
				}
			});
	}
#endif

	/* main Thread */
	GameLoop();

#ifdef SERVER_COMMUNICATION
	THREAD_MGR->Join();
#endif
}



LRESULT GameFramework::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (Engine::I->WndProc(hWnd, message, wParam, lParam)) {
		return 0;
	}

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		ProcessMouseMsg(hWnd, message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		ProcessKeyboardMsg(hWnd, message, wParam, lParam);
		break;

	default:
		break;
	}

	return 0;
}

void GameFramework::ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_F1:
			Timer::I->Stop();
			break;
		case VK_F2:
			Timer::I->Start();
			break;
		case VK_F5:
			Scene::I->ToggleDrawBoundings();
			break;
		case VK_F6:
			ImGuiMgr::I->ToggleImGui();
			break;
		case VK_F8:
			Scene::I->ToggleFilterOptions();
			break;
		case 192:	// '`'
			::SetFocus(NULL);
			break;

		default:
			break;
		}
	}

	break;
	default:
		break;
	}

	if (const auto& playerScript = mPlayerScript.lock()) {
		playerScript->ProcessKeyboardMsg(message, wParam, lParam);
	}
}

void GameFramework::ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (const auto& playerScript = mPlayerScript.lock()) {
		playerScript->ProcessMouseMsg(message, wParam, lParam);
	}
}

ATOM GameFramework::CreateGameClientWindow()
{
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mhInst;
	wcex.hIcon = LoadIcon(mhInst, MAKEINTRESOURCE(IDI_XMACHINA));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_CLIENT);
	wcex.lpszClassName = L"X-MACHINA";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	RECT rc = { 0, 0, mResolution.Width, mResolution.Height };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);

	mhWnd = ::CreateWindowW(L"X-MACHINA", L"X-MACHINA", WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT, 0, mResolution.Width, mResolution.Height
		, nullptr, nullptr, mhInst, nullptr);


	::ShowWindow(mhWnd, SW_SHOWDEFAULT);
	::UpdateWindow(mhWnd);

	return ::RegisterClassEx(&wcex);

}

LRESULT GameFramework::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GameFramework::I->ProcessMessage(hWnd, message, wParam, lParam)) {
		return 0;
	}

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(mhInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR GameFramework::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void GameFramework::InitPlayer()
{
	sptr<GridObject> player = Engine::I->GetPlayer();
	player->ResetCollider();
	mPlayerScript = player->AddComponent<Script_GroundPlayer>();
	player->SetModel("EliteTrooper");

	//player->AddComponent<ParticleSystem>()->Load("Green")->SetTarget("Humanoid_ R Hand");
	//player->AddComponent<ParticleSystem>()->Load("Fire")->SetTarget("Humanoid_ L Hand");
	//player->AddComponent<ParticleSystem>()->Load("Fountain")->SetTarget("Humanoid_ Head");
}
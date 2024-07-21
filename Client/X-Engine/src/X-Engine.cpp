﻿#include "EnginePch.h"
#include "X-Engine.h"

#include "DXGIMgr.h"
#include "InputMgr.h"

#include "Timer.h"
#include "BattleScene.h"
#include "LobbyScene.h"
#include "Object.h"
#include "FrameResource.h"

#pragma region  - 장재문 -
#include "../Imgui/ImguiCode/imgui.h"
#include "../Imgui/ImGuiMgr.h"
#pragma endregion

Engine::Engine()
	:
	mTitle(L"LabProject")
{
	mUpdateFunc = &Engine::UpdateLobby;
}


void Engine::Init(HINSTANCE hInstance, HWND hWnd)
{
	InputMgr::I->Init();

	WindowInfo windowInfo{ hWnd, Engine::I->GetWindowWidth(), Engine::I->GetWindowHeight() };
	DXGIMgr::I->Init(hInstance, windowInfo);

	LobbyScene::I->Init();

#pragma region Imgui - 장재문 - 
	ImGuiMgr::I->Init();
#pragma endregion
#pragma region Log - 장재문 -
	//LogMgr::I->Init(""); // 이름을 지을 수 있다. 
#pragma endregion
}


void Engine::Release()
{
	FRAME_RESOURCE_MGR->WaitForGpuComplete();

	BattleScene::I->Release();
	DXGIMgr::I->Release();
}


void Engine::Update()
{
	// update dxgi
	DXGIMgr::I->Update();

	// update scene
	(this->*mUpdateFunc)();

	// update input
	InputMgr::I->Update();

	DXGIMgr::I->RenderScene();

	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + Timer::I->GetFrameRate();
	::SetWindowText(DXGIMgr::I->GetHwnd(), title.data());
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Engine::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// 미사용 메시지 처리x //
	switch (msg) {
	case WM_SETTEXT:
	case WM_SETCURSOR:
	case WM_GETICON:
	case WM_NCHITTEST:
	case WM_NCMOUSEMOVE:
	case 174:
		return true;

	default:
		break;
	}

	// ImGui 메시지 처리
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	// 개선 필요
	// ImGui가 포커싱되어 있다면 마우스 커서를 보이게 한다.
	if (ImGuiMgr::I->IsFocused()) {
		ShowCursor(TRUE);
		ImGuiMgr::I->FocusOff();	// ImGui의 포커싱을 없앤다.
		return true;
	}

	switch (msg)
	{
	case WM_SETFOCUS:
		WindowFocusOn();
		break;
	case WM_KILLFOCUS:
		WindowFocusOff();
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (!mIsWindowFocused) {
			::SetFocus(hWnd);
		}
	break;
	default:
		break;
	}

	if (mIsWindowFocused) {
		InputMgr::I->WndProc(hWnd, msg, wParam, lParam);
	}

	return false;
}

void Engine::LoadScene(SceneType sceneType)
{
	switch (sceneType) {
	case SceneType::Lobby:
		mUpdateFunc = &Engine::UpdateLobby;
		break;
	case SceneType::Battle:
		mUpdateFunc = &Engine::UpdateBattle;

		BattleScene::I->Build();

		//Scene::I->ReleaseUploadBuffers();
		break;
	default:
		assert(0);
		break;
	}

	DXGIMgr::I->SwitchScene(sceneType);
}

void Engine::UpdateLobby()
{
	LobbyScene::I->Update();
}

void Engine::UpdateBattle()
{
	BattleScene::I->Update();
}

void Engine::WindowFocusOn()
{
	InputMgr::I->WindowFocusOn();
	while (ShowCursor(FALSE) >= 0);
	mIsWindowFocused = true;
}

void Engine::WindowFocusOff()
{
	InputMgr::I->WindowFocusOff();
	while (ShowCursor(TRUE) <= 0);
	mIsWindowFocused = false;
}

#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_LobbyUI.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "SoundMgr.h"
#include "Timer.h"
#include "Scene.h"
#include "Light.h"
#include "Object.h"
#include "LobbyScene.h"
#include "GameFramework.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "Component/Camera.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	MAIN_CAMERA->SetOffset(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetPosition(Vec3(9.88f, 2.16f, 6.93f));
	MainCamera::I->SetLocalRotation(Vec3(15.25f, -124.f, 0.f));
	//MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MainCamera::I->MoveForward(1.f);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);

	mObject->AddComponent<Script_LobbyUI>();
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
	trooper->SetPosition(7.4f, 0, 5.27f);
	trooper->SetLocalRotation(Vec3(0.f, 53.41f, 0.f));

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-0.3f, -0.6f, -0.6));
	LobbyScene::I->GetLight()->SetSunlightColor(Vector3::One * 0.52f);

	SoundMgr::I->Play("BGM", "Lobby", 1.0f, true);
}

void Script_LobbyManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		ChangeToBattleScene();
	}
}

void Script_LobbyManager::Reset()
{
	base::Reset();

	mObject->RemoveComponent<Script_LobbyUI>();
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}

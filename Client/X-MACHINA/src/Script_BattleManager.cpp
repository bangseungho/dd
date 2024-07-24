#include "stdafx.h"
#include "Script_BattleManager.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"
#include "Script_Arack.h"
#include "Script_Ceratoferox.h"
#include "Script_Anglerox.h"
#include "Script_MiningMech.h"
#include "Script_Aranobot.h"
#include "Script_Gobbler.h"
#include "Script_Rapax.h"
#include "Script_LightBipedMech.h"

#include "Script_MainCamera.h"
#include "Script_Item.h"
#include "Component/ParticleSystem.h"
#include "Component/Camera.h"

#include "GameFramework.h"

#include "BattleScene.h"
#include "Object.h"
#include "ScriptExporter.h"
#include "TextMgr.h"
#include "InputMgr.h"
#include "X-Engine.h"



void Script_BattleManager::Awake()
{
	base::Awake();

	MainCamera::I->AddComponent<Script_MainCamera>();
	GameFramework::I->InitPlayer();
	GameFramework::I->ConnectServer();

	InitSceneObjectScripts();
	InitCustomObjectScripts();
}

void Script_BattleManager::Start()
{
	base::Start();

	mMainCamera = MainCamera::I->GetComponent<Script_MainCamera>();

	// Hello
	{
		TextOption textOption;
		textOption.FontSize = 40.f;
		textOption.FontStyle = TextFontStyle::Italic;

		TextMgr::I->CreateText("HELLO", Vec2(0.f, 500.f), textOption);
	}

	// Game Title
	{
		TextOption textOption;
		textOption.Font = "ISOCPEUR";
		textOption.FontSize = 70.f;
		textOption.FontWeight = TextFontWeight::BOLD;
		textOption.VAlignment = TextParagraphAlign::Near;

		TextBox* text = TextMgr::I->CreateText("X-MACHINA TEST", Vec2::Zero, textOption);
		text->SetColor(TextFontColor::Type::Orange);
	}
}

void Script_BattleManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		Engine::I->LoadScene(SceneType::Lobby);
	}
}

void Script_BattleManager::Reset()
{
	base::Reset();

	GameFramework::I->DisconnectServer();
	MainCamera::I->RemoveComponent<Script_MainCamera>();
	mMainCamera = nullptr;
	GameFramework::I->ResetPlayer();
}

void Script_BattleManager::InitSceneObjectScripts()
{
	BattleScene::I->ProcessInitScriptOjbects(std::bind(&Script_BattleManager::ProcessSceneObjectScript, this, std::placeholders::_1));
}

void Script_BattleManager::InitCustomObjectScripts()
{
	// hard coding
}

void Script_BattleManager::ProcessSceneObjectScript(sptr<Object> object)
{
	const auto& exporter = object->GetComponent<ScriptExporter>();
	if (!exporter) {
		return;
	}

	switch (Hash(exporter->GetName())) {
	case Hash("WeaponCrate"):
	case Hash("WeaponCrate2"):
		object->AddComponent<Script_Item_WeaponCrate>()->LoadData(exporter);
		break;
	case Hash("AdvancedCombatDroid"):
		object->AddComponent<Script_AdvancedCombatDroid_5>();
		break;
	case Hash("Onyscidus"):
		object->AddComponent<Script_Onyscidus>();
		break;
	case Hash("Ursacetus"):
		object->AddComponent<Script_Ursacetus>();
		break;
	case Hash("Arack"):
		object->AddComponent<Script_Arack>();
		break;
	case Hash("Ceratoferox"):
		object->AddComponent<Script_Ceratoferox>();
		break;
	case Hash("Anglerox"):
		object->AddComponent<Script_Anglerox>();
		break;
	case Hash("MiningMech"):
		object->AddComponent<Script_MiningMech>();
		break;
	case Hash("Aranobot"):
		object->AddComponent<Script_Aranobot>();
		break;
	case Hash("Gobbler"):
		object->AddComponent<Script_Gobbler>();
		break;
	case Hash("Rapax"):
		object->AddComponent<Script_Rapax>();
		break;
	case Hash("LightBipedMech"):
		object->AddComponent<Script_LightBipedMech>();
		break;
	default:
		throw std::runtime_error("[Error] Couldn't import script");
		break;
	}
}

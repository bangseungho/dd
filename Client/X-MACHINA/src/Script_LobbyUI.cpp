#include "stdafx.h"
#include "Script_LobbyUI.h"

#include "Script_AimController.h"
#include "Script_LobbyManager.h"

#include "Component/UI.h"
#include "InputMgr.h"
#include "TextMgr.h"

void Script_LobbyUI::Start()
{
	base::Start();

	// Panels
	{
		const auto& top = Canvas::I->CreateUI<UI>(0, "Logo", Vec2(-1600, 900));
		const auto& bottm = Canvas::I->CreateUI<UI>(0, "BottomPanel", Vec2(0, -940));
	}

	// Buttons
	{
		constexpr float x    = -1450;
		constexpr float y    = -300;
		constexpr float yGap = -110;
				
		const auto& playButton    = Canvas::I->CreateUI<Button>(1, "PlayButton", Vec2(x, y));
		const auto& customButton  = Canvas::I->CreateUI<Button>(1, "CustomButton", Vec2(x, y + (yGap * 1)));
		const auto& settingButton = Canvas::I->CreateUI<Button>(1, "SettingButton", Vec2(x, y + (yGap * 2)));
		const auto& quitButton    = Canvas::I->CreateUI<Button>(1, "QuitButton", Vec2(x, y + (yGap * 3)));
	}

	mCursorNormal = Canvas::I->CreateUI<UI>(4, "Cursor_Normal", Vec2::Zero, Vec2(60, 60));
	mCursorClick = Canvas::I->CreateUI<UI>(4, "Cursor_Click", Vec2::Zero, Vec2(60, 60));
	mCursorClick->SetActive(false);

	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetUI(mCursorNormal);
}

void Script_LobbyUI::Update()
{
	base::Update();

	if (KEY_TAP(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();
		Canvas::I->CheckClick(pos);

		mCursorNormal->SetActive(false);
		mCursorClick->SetActive(true);
		mAimController->SetUI(mCursorClick);
		;
	}
	else if (KEY_AWAY(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();

		mCursorNormal->SetActive(true);
		mCursorClick->SetActive(false);
		mAimController->SetUI(mCursorNormal);
	}
}

void Script_LobbyUI::OnDestroy()
{
	base::OnDestroy();

	mObject->RemoveComponent<Script_AimController>();
}

void Script_LobbyUI::ChangeToBattleScene() const
{
	//Script_LobbyManager::ChangeToBattleScene();
}
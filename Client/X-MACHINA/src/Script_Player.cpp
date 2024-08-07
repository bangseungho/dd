#include "stdafx.h"
#include "Script_Player.h"

#include "Script_MainCamera.h"
#include "Script_Item.h"

#include "Component/Camera.h"
#include "Component/Rigidbody.h"
#include "Component/UI.h"

#include "Object.h"

#include "SliderBarUI.h"
#include "ChatBoxUI.h"

void Script_Player::Start()
{
	base::Start();

	mCamera = MainCamera::I->GetComponent<Script_MainCamera>().get();
}

void Script_Player::Update()
{
	base::Update();

	mIsInteracted = false;
}

void Script_Player::SetSpawn(const Vec3& pos)
{
	mObject->SetPosition(pos);
	XMStoreFloat4x4(&mSpawnTransform, _MATRIX(mObject->GetWorldTransform()));
}

bool Script_Player::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'E':
			Interact();
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void Script_Player::Rotate(float pitch, float yaw, float roll)
{
	mObject->Rotate(pitch, yaw, roll);
}

void Script_Player::Dead()
{
	Respawn();
}

void Script_Player::Respawn()
{
	Resurrect();
	mObject->SetWorldTransform(mSpawnTransform);
}

void Script_Player::Interact()
{
	const auto& collisionObjects = mObject->GetCollisionObjects();
	for (const auto& other : collisionObjects) {
		switch (other->GetTag()) {
		case ObjectTag::Crate:
		case ObjectTag::Item:
			if (!mIsInteracted) {
				mIsInteracted = other->GetComponent<Script_Item>()->Interact(mObject);
				if (mIsInteracted) {
					return;
				}
			}
			break;
		default:
			break;
		}
	}
}

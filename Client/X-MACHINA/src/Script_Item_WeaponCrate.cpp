﻿#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"

#include "ScriptExporter.h"
#include "BattleScene.h"
#include "Timer.h"
#include "Object.h"

void Script_Item_WeaponCrate::Awake()
{
	base::Awake();

	mObject->RemoveComponent<ScriptExporter>();
	mCap = mObject->FindFrame("box_cap");
}

void Script_Item_WeaponCrate::Animate()
{
	base::Animate();

	if (!mIsOpend) {
		return;
	}

	constexpr float openSpeed = 250.f;
	constexpr float maxPitch = 190.f;

	if (mCapPitch > maxPitch) {
		return;
	}

	const float openAmount = DeltaTime() * openSpeed;

	mCapPitch += openAmount;
	mCap->Rotate(Vector3::Down, openAmount);
}

void Script_Item_WeaponCrate::OnCollisionEnter(Object& other)
{
	if (mIsOpend) {
		return;
	}

	base::OnCollisionEnter(other);
}

void Script_Item_WeaponCrate::LoadData(rsptr<ScriptExporter> exporter)
{
	std::string weaponName;
	exporter->GetData("Name", weaponName);
	mWeaponName = gkWeaponNameMap.at(Hash(weaponName));

	// create weapon from name
	std::string weaponModelName = Script_Weapon::GetWeaponModelName(mWeaponName);
	mWeapon = BattleScene::I->Instantiate(weaponModelName, ObjectTag::Item, ObjectLayer::Default, false);
	mWeapon->SetWorldTransform(mObject->GetWorldTransform());
	mWeapon->AddComponent<Script_Item_Weapon>();
	
	switch(mWeaponName) {
	case WeaponName::H_Lock:
		mWeapon->AddComponent<Script_Weapon_Pistol>();
		break;
	case WeaponName::SkyLine:
		mWeapon->AddComponent<Script_Weapon_Skyline>();
		break;
	case WeaponName::DBMS:
		mWeapon->AddComponent<Script_Weapon_DBMS>();
		break;
	case WeaponName::Burnout:
		mWeapon->AddComponent<Script_Weapon_Burnout>();
		break;
	case WeaponName::PipeLine:
		mWeapon->AddComponent<Script_Weapon_PipeLine>();
		break;
	default:
		assert(0);
		break;
	}
}

void Script_Item_WeaponCrate::DisableInteract()
{
	base::DisableInteract();
}

bool Script_Item_WeaponCrate::Interact(Object* user)
{
	if (mIsOpend) {
		return false;
	}
	mIsOpend = true;
	DisableInteract();

	if (mWeapon) {
		mWeapon->SetActive(true);
		mWeapon->GetComponent<Script_Item_Weapon>()->StartOpen();
	}

	return true;
}
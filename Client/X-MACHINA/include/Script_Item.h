﻿#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

class GameObject;
class Rigidbody;


enum class ItemType {
	None,
	WeaponCrate,
	Weapon
};



class Script_Item abstract : public Component {
	COMPONENT_ABSTRACT(Script_Item, Component)

public:
	virtual void Interact(Object* user) abstract;
	virtual ItemType GetItemType() abstract;
};



class Script_Item_WeaponCrate : public Script_Item, SceneScript {
	COMPONENT(Script_Item_WeaponCrate, Script_Item)

public:
	WeaponName mWeaponName;
	Transform* mCap{};

	float mCapPitch{};
	bool mIsOpend{};

	sptr<GameObject> mWeapon{};

public:
	virtual void Awake() override;
	virtual void Animate() override;

public:
	virtual void Interact(Object* user) override;
	virtual ItemType GetItemType() override { return ItemType::WeaponCrate; }

public:
	virtual void LoadData(rsptr<ScriptExporter> exporter) override;
};



class Script_Item_CrateItem : public Script_Item {
	COMPONENT_ABSTRACT(Script_Item_CrateItem, Script_Item)

private:
	sptr<Rigidbody> mRigid{};
	Vec3 mDir{};
	WeaponName mWeaponName;

	bool mOpened{ false };

	int mSign{ 1 };
	float mDeltaTime{};
	float mMaxFloatingSpeed{};

public:
	virtual void Awake() override;
	virtual void Animate() override;

public:
	virtual void Interact(Object* user) override;
	virtual ItemType GetItemType() override { return ItemType::Weapon; }
	void SetWeaponName(WeaponName weaponName) { mWeaponName = weaponName; }

public:
	virtual void StartOpen();
};
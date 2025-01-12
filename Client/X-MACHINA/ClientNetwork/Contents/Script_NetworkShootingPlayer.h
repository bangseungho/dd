#pragma once

#include "Script_NetworkPlayer.h"
#include "Component/Component.h"

#include "Script_Weapon.h"

class Script_NetworkShootingPlayer : public Script_NetworkPlayer {
	COMPONENT(Script_NetworkShootingPlayer, Script_NetworkPlayer)

private:
	int							mCrntWeaponNum{};
	int							mNextWeaponNum{};
	bool						mIsInDraw{};
	bool						mIsInPutback{};

protected:
	GameObject*					mWeapon{};
	sptr<Script_Weapon>			mWeaponScript{};
	std::vector<GameObject*>	mWeapons{};
	Transform*					mMuzzle{};
		
public:
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	bool IsInGunChangeMotion() const	{ return IsInDraw() || IsInPutBack(); }
	bool IsInDraw() const				{ return mIsInDraw; }
	bool IsInPutBack() const			{ return mIsInPutback; }

	virtual void BulletFired() {}

protected:
	int GetCrntWeaponIdx() const { return mCrntWeaponNum - 1; }
	int GetCrntWeaponNum() const { return mCrntWeaponNum; }
	int GetNextWeaponNum() const { return mNextWeaponNum; }

	virtual void DrawWeapon(int weaponNum);

	virtual void DrawWeaponStart(int weaponNum, bool isDrawImmed);// abstract;
	virtual void DrawWeapon();
	virtual void DrawWeaponEnd();
	virtual void PutbackWeapon() abstract;
	virtual void PutbackWeaponEnd();
	virtual void DropWeapon(int weaponIdx);
	virtual void ResetWeaponAnimation() {}


	virtual void StartFire();
	virtual void StopFire();
	virtual bool Reload();

private:
	void SetWeapon(int weaponNum);
	void ResetNextWeaponNum() { mNextWeaponNum = 0; }

};


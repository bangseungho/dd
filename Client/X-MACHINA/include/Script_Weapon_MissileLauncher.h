#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion


// base rifle class
class Script_Weapon_MissileLauncher abstract : public Script_BulletWeapon {
	COMPONENT_ABSTRACT(Script_Weapon_MissileLauncher, Script_BulletWeapon)

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::MissileLauncher; }
	virtual WeaponName GetWeaponName() const abstract;

public:
	virtual void Awake() override;

private:
	virtual void CreateBulletPool() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
};




class Script_Weapon_Burnout : public Script_Weapon_MissileLauncher {
	COMPONENT(Script_Weapon_Burnout, Script_Weapon_MissileLauncher)

protected:
	static constexpr float mkBulletSpeed = 20.f;

private:
	static constexpr int   mkBulletCntPerMag = 4;
	static constexpr float mkExplosionDamage = 84.0f / mkBulletCntPerMag;
	static constexpr float mkBulletDamage    = 0.f;
	static constexpr float mkRPM             = 0.1f;

	std::vector<Transform*> mMuzzles{};		// 총구 여러개를 가진다.

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::Burnout; }

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }
	virtual void FireBullet() override;

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};




class Script_Weapon_Deus_Rifle : public Script_Weapon_MissileLauncher {
	COMPONENT(Script_Weapon_Deus_Rifle, Script_Weapon_MissileLauncher)

protected:
	static constexpr float mkBulletSpeed = 10.f;

private:
	static constexpr int   mkBulletCntPerMag = 10;
	static constexpr float mkExplosionDamage = 0.f;
	static constexpr float mkBulletDamage = 0.f;
	static constexpr float mkRPM = 0.1f;
	
	Object* mTarget{};

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::Burnout; }

public:
	void SetTarget(Object* target) { mTarget = target; }

public:
	virtual void FireBullet() override;

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};





class Script_Weapon_LightBiped : public Script_Weapon_MissileLauncher {
	COMPONENT(Script_Weapon_LightBiped, Script_Weapon_MissileLauncher)

protected:
	static constexpr float mkBulletSpeed = 10.f;

private:
	static constexpr int   mkBulletCntPerMag = 10;
	static constexpr float mkExplosionDamage = 0.f;
	static constexpr float mkBulletDamage = 0.f;
	static constexpr float mkRPM = 0.1f;

	Object* mTarget{};

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::Burnout; }

public:
	void SetTarget(Object* target) { mTarget = target; }

public:
	virtual void FireBullet() override;

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void CreateBulletPool() override;
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};
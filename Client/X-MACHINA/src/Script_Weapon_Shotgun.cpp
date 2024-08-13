#include "stdafx.h"
#include "Script_Weapon_Shotgun.h"

#include "Script_Bullet.h"

#include "Object.h"
#include "ObjectPool.h"

#include "SoundMgr.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

void Script_Weapon_Shotgun::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_Shotgun::FireBullet()
{
	Script_Weapon::FireBullet();	// Script_BulletWeapon은 단일 총알 발사이기 때문에 무시한다.

	if (mFireSound != "") {
		SoundMgr::I->Play("Gun", mFireSound);
	}

	const auto& bullets = mBulletPool->GetMulti(mBulletCntPerShot, true);
	for (auto& bullet : bullets) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();

		Vec2 err = Vec2(Math::RandFloat(mErrX.x, mErrX.y), Math::RandFloat(mErrY.x, mErrY.y));
		const float bulletSpeedErr = Math::RandFloat(0, mSpeerErr);

		bulletScript->SetSpeed(mkBulletSpeed - bulletSpeedErr);
		bulletScript->Fire(*mMuzzle, err);
	}

	if (IsPlayerWeapon()) {
		auto cpkt = FBS_FACTORY->CPkt_Bullet_OnShoot(bullets.front()->GetPosition(), bullets.front()->GetLook());
		CLIENT_NETWORK->Send(cpkt);
	}
}






void Script_Weapon_DBMS::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 2.2f;
	mMaxDistance      = 8.f;
	mBulletCntPerMag  = 2;
	mMaxMag           = 10;
	mBulletCntPerShot = mkBulletCntPerShot;

	mSpeerErr = 20.f;
	mErrX = Vec2(-20.f, 20.f);
	mErrY = Vec2(-10.f, 6.f);

	SetFireSound("DBMS Fire");
	SetReloadSound("DBMS Reload");
}

void Script_Weapon_DBMS::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}

void Script_Weapon_DBMS::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

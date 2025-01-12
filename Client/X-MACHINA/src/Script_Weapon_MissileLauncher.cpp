#include "stdafx.h"
#include "Script_Weapon_MissileLauncher.h"

#include "Script_Missile.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"

void Script_Weapon_MissileLauncher::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_MissileLauncher::CreateBulletPool()
{
	mBulletPool = BattleScene::I->CreateObjectPool("SM_Missile", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_Weapon_MissileLauncher::BulletInitFunc, this, std::placeholders::_1));
}






void Script_Weapon_Burnout::FireBullet()
{
	base::FireBullet();

	if (mCurBulletCnt > 0) {
		mMuzzle = mMuzzles[mCurBulletCnt - 1];
	}
	else {
		mMuzzle = mMuzzles[mBulletCntPerMag - 1];
	}
}

void Script_Weapon_Burnout::InitValues()
{
	mMaxFireDelay     = 0.1f;	 // 발사 딜레이 거의 없음
	mMaxReloadTime    = 5.9f;
	mMaxDistance      = 43.f;
	mBulletCntPerMag  = mkBulletCntPerMag;
	mMaxMag           = 3;

	mErrX = Vec2(-2.f, 2.f);
	mErrY = Vec2(-2.f, 2.f);

	mMuzzles.resize(mBulletCntPerMag);
	mMuzzles[mBulletCntPerMag - 1] = mObject->FindFrame("FirePos_1");
	mMuzzles[mBulletCntPerMag - 2] = mObject->FindFrame("FirePos_2");
	mMuzzles[mBulletCntPerMag - 3] = mObject->FindFrame("FirePos_3");
	mMuzzles[mBulletCntPerMag - 4] = mObject->FindFrame("FirePos_4");
	mMuzzle = mMuzzles[mBulletCntPerMag - 1];

	SetFireSound("Buronout Fire");
}

void Script_Weapon_Burnout::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed, BulletType::Missile);
	const auto& missile = bullet->GetComponent<Script_Missile>();
	missile->SetExplosionDamage(mkExplosionDamage);
	missile->SetImpactSound("Burnout Impact");
}

void Script_Weapon_Burnout::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Missile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Missile_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Missile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

void Script_Weapon_Deus_Rifle::FireBullet()
{
	if (!mTarget) {
		return;
	}

	const Vec3 look = mObject->GetLook();
	const Vec3 toTarget = Vector3::Normalized(mTarget->GetPosition() - mMuzzle->GetPosition());
	const float angle = Vector3::Angle(look, toTarget);

	mMuzzle->SetLocalRotation(Vec3{ angle, 0.f, 0.f });

	base::FireBullet();
}

void Script_Weapon_Deus_Rifle::InitValues()
{
	mMaxFireDelay = 0.1f;	 // 발사 딜레이 거의 없음
	mMaxReloadTime = 5.9f;
	mMaxDistance = 43.f;
	mBulletCntPerMag = mkBulletCntPerMag;
	mMaxMag = 3;

	mErrX = Vec2(-2.f, 2.f);
	mErrY = Vec2(-2.f, 2.f);

	mMuzzle = mObject->FindFrame("FirePos");

	SetFireSound("Buronout Fire");
}

void Script_Weapon_Deus_Rifle::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed, BulletType::DeusMissile);
	const auto& missile = bullet->GetComponent<Script_DeusMissile>();
	missile->SetExplosionDamage(mkExplosionDamage);
	missile->SetImpactSound("Burnout Impact");
}

void Script_Weapon_Deus_Rifle::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Explosion_Add2");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

void Script_Weapon_LightBiped::FireBullet()
{
	base::FireBullet();
}

void Script_Weapon_LightBiped::CreateBulletPool()
{
	mBulletPool = BattleScene::I->CreateObjectPool("SM_Width_Missile", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_Weapon_LightBiped::BulletInitFunc, this, std::placeholders::_1));
}

void Script_Weapon_LightBiped::InitValues()
{
	mMaxFireDelay = 0.1f;	 // 발사 딜레이 거의 없음
	mMaxReloadTime = 5.9f;
	mMaxDistance = 43.f;
	mBulletCntPerMag = mkBulletCntPerMag;
	mMaxMag = 3;

	mErrX = Vec2(-2.f, 2.f);
	mErrY = Vec2(-2.f, 2.f);

	mMuzzle = mObject->FindFrame("FirePos");

	SetFireSound("Buronout Fire");
}

void Script_Weapon_LightBiped::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed, BulletType::DeusMissile);
	const auto& missile = bullet->GetComponent<Script_DeusMissile>();
	missile->SetExplosionDamage(mkExplosionDamage);
	missile->SetImpactSound("Burnout Impact");
}

void Script_Weapon_LightBiped::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Missile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Explosion_Add2");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_DeusMissile_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

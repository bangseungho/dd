#include "stdafx.h"
#include "Script_Player.h"
#include "Scene.h"
#include "Object.h"
#include "Shader.h"

void Script_TankPlayer::Start()
{
	base::Start();

	mShellFirePos = mPlayer->FindFrame("fire_pos");
	mTurret = mPlayer->FindFrame("turret")->GetObj<GameObject>();
	mGun = mPlayer->FindFrame("gun")->GetObj<GameObject>();
}


void Script_TankPlayer::Update()
{
	base::Update();


}


void Script_TankPlayer::FixTurret()
{

}


void Script_TankPlayer::Rotate(DWORD rotationDir, float angle)
{
	//Vec3 mMovingDir = mPlayer->GetMovingDir();

	angle *= mRotationSpeed;
	//if (!Vector3::IsZero(mMovingDir) && sign(mMovingDir.z) != sign(mObject->GetLook().z)) {
	//	angle *= -1;
	//}

	if (rotationDir) {
		float yRotation = 0.f;
		if (rotationDir & (WORD)Dir::Left) {
			yRotation -= angle;
		}
		if (rotationDir & (WORD)Dir::Right) {
			yRotation += angle;
		}

		if (yRotation != 0.f) {
			mObject->Rotate(0.f, yRotation, 0.f);
			mTurret->Rotate(0.f, -yRotation, 0.f);
			mPrevTurretYaw = -yRotation;
		}
	}
}


void Script_TankPlayer::RotateTurret(float pitch, float yaw)
{
	mTurret->Rotate(0.f, yaw, 0.f);
	RotateGun(pitch);
}


void Script_TankPlayer::RotateGun(float pitch)
{
	if (pitch == 0.f) {
		return;
	}

	constexpr float minPitch = XMConvertToRadians(-45.0f);
	constexpr float maxPitch = XMConvertToRadians(5.5f);
	float gunPitch = -GetGunPitch();
	if (gunPitch + pitch > maxPitch) {
		pitch = maxPitch - gunPitch;
	}
	else if (gunPitch + pitch < minPitch) {
		pitch = minPitch - gunPitch;
	}

	mGun->Rotate(Vec3(1.f, 0.f, 0.f), pitch);
}



void Script_TankPlayer::Move(DWORD dwDirection)
{
	dwDirection &= ~Dir::Up;
	dwDirection &= ~Dir::Down;

	base::Move(dwDirection);
}

void Script_TankPlayer::FireBullet()
{
	Vec3 position = mShellFirePos->GetPosition();
	Vec3 dir = mShellFirePos->GetLook();
	Vec3 up = mShellFirePos->GetUp();

	mBulletShader->FireBullet(position, dir, mObject->GetUp(), mBulletSpeed);
}



float Script_TankPlayer::GetGunPitch() const
{
	Vec3 look = mGun->GetLook();
	return atan2(look.y, sqrt(look.x * look.x + look.z * look.z));
}
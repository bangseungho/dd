#include "stdafx.h"
#include "Script_Player.h"
#include "Scene.h"
#include "Shader.h"
#include "Object.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Rigidbody.h"






void Script_AirplanePlayer::Start()
{
	Script_ShootingPlayer::Start();

	mGunFirePos = mPlayer->FindFrame("gunfire_pos");

	SetFireDelay(0.1f);
	mPlayerType = PlayerType::Airplane;
	mRotationSpeed = 90.f;

	SetSpawn(Vec3(50.f, 100.f, 50.f));
	SetHP(150.f);
	SetDamage(10.f);
	mObject->GetComponent<Rigidbody>()->SetMass(100.f);
	mObject->GetComponent<Rigidbody>()->SetFriction(30.f);

	base::SetAcc(5000.f);
	base::SetMaxSpeed(100.f);
	mBulletSpeed = 300.f;
}


void Script_AirplanePlayer::Update()
{
	base::Update();

	base::ProcessInput();
	ProcessInput();

	Vec3 pos = mObject->GetPosition();
	float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);

	if (pos.y < terrainHeight) {
		Explode();
	}
}

void Script_AirplanePlayer::ProcessInput()
{
	DWORD dwDirection = 0;
	DWORD rotationDir = 0;
	if (KEY_PRESSED('W'))			dwDirection |= (WORD)Dir::Front;
	if (KEY_PRESSED('S'))			dwDirection |= (WORD)Dir::Back;
	if (KEY_PRESSED(VK_SPACE))		dwDirection |= (WORD)Dir::Up;
	if (KEY_PRESSED(VK_LSHIFT))		dwDirection |= (WORD)Dir::Down;
	if (dwDirection) {
		base::Move(dwDirection);
	}

	if (KEY_PRESSED('A')) rotationDir |= (WORD)Dir::Left;
	if (KEY_PRESSED('D')) rotationDir |= (WORD)Dir::Right;
	if (rotationDir) {
		Rotate(rotationDir, DeltaTime());
	}

	if (KEY_PRESSED(VK_LBUTTON)) {
		Vec2 mouseDelta = InputMgr::Inst()->GetMouseDelta();
		mPlayer->Rotate(mouseDelta.y, mouseDelta.x);
	}
}



void Script_AirplanePlayer::Rotate(DWORD rotationDir, float angle)
{
	angle *= mRotationSpeed;
	if (rotationDir) {
		float zRotation = 0.f;
		if (rotationDir & (WORD)Dir::Left) {
			zRotation += angle;
		}
		if (rotationDir & (WORD)Dir::Right) {
			zRotation -= angle;
		}

		if (zRotation != 0.f) {
			mObject->Rotate(0.f, 0.f, zRotation);
		}
	}
}


void Script_AirplanePlayer::FireBullet()
{
	if (mGunFirePos) {
		Vec3 position = mGunFirePos->GetPosition();
		Vec3 dir = mGunFirePos->GetLook();
		Vec3 up = mGunFirePos->GetUp();

		mBulletShader->FireBullet(position, dir, up, mBulletSpeed);
	}
}


void Script_AirplanePlayer::OnCollisionStay(Object& other)
{
	switch (other.GetTag())
	{
	case ObjectTag::Building:
		Explode();
		break;
	default:
		break;
	}
}
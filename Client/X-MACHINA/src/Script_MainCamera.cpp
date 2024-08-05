#include "stdafx.h"
#include "Script_MainCamera.h"

#include "GameFramework.h"

#include "Object.h"
#include "Timer.h"
#include "BattleScene.h"
#include "X-Engine.h"

#include "Component/Camera.h"
#include "Component/Collider.h"


namespace {
	Vec3 kCameraOffset = Vec3(0.f, 14.f, -7.f);
}


void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	mMainOffset = offset;
	MAIN_CAMERA->SetOffset(mMainOffset);
}

void Script_MainCamera::SetCameraTarget(GameObject* target)
{
	if (target) {
		mTarget = target;
	}
}


void Script_MainCamera::Awake()
{
	base::Awake();

	constexpr Vec2 maxOffset = Vec2(3, 3);
	const Vec2 resolution = Vec2(MAIN_CAMERA->GetWidth(), MAIN_CAMERA->GetHeight());
	// corr = 0.002 : 1280 * 1080 -> 2.56, 2.16
	constexpr float corr = 0.002f;
	mMaxOffset = Vec2(resolution.x * corr, resolution.y * corr);

	mMaxOffset.x = mMaxOffset.x > maxOffset.x ? maxOffset.x : mMaxOffset.x;
	mMaxOffset.y = mMaxOffset.y > maxOffset.y ? maxOffset.y : mMaxOffset.y;
}

void Script_MainCamera::Start()
{
	base::Start();

	//Init();
}

void Script_MainCamera::OnEnable()
{
	base::OnEnable();

	Init();
}

void Script_MainCamera::Update()
{
	base::Update();

	Shake();

	if (mTarget) {
		Vec3 offset = mMainOffset + Vec3(mExtraOffset.x, 0.f, mExtraOffset.y) + mShakeOffset;
		offset *= mZoomAmount;
		mObject->SetPosition(mTarget->GetPosition() + offset);

		Matrix noLagViewMtx = Matrix::CreateLookAt(mTarget->GetPosition() + offset, mTarget->GetPosition(), mTarget->GetUp());
		MAIN_CAMERA->SetNoLagViewMtx(noLagViewMtx);

		RecoverExtraOffset();
	}
}

void Script_MainCamera::LateUpdate()
{
	base::LateUpdate();

	//HideObstacles();
}

void Script_MainCamera::Move(Vec2 dir, Vec2 weight, float maxOffset_t)
{
	maxOffset_t = std::clamp(maxOffset_t, 0.f, 1.f);
	const Vec2 maxOffset = mMaxOffset * maxOffset_t;

	constexpr float originSpeed = 1.f;

	auto CalculateDir = [&](float dir, float extraOffset) -> float {
		if (Math::IsZero(dir)) {
			return 0.f;
		}
		else {
			return static_cast<float>(Math::Sign(dir));
		}
		};

	auto CalculateSpeed = [&](float dir, float extraOffset, float maxOffset) -> float {
		// 반대 방향 이동시 기본 속도S
		if (Math::Sign(extraOffset) != Math::Sign(dir)) {
			return originSpeed;
		}
		// 정방향 이동 시 거리가 멀어질 수록 속도를 줄인다.
		else {
			constexpr float ratioAffect = 2.f;
			float ratio = (fabs(extraOffset) / fabs(maxOffset)) * ratioAffect;
			return originSpeed / (1 + ratio);
		}
		};

	dir.x = CalculateDir(dir.x, mExtraOffset.x);
	dir.y = CalculateDir(dir.y, mExtraOffset.y);

	weight.x = std::clamp(weight.x, 0.f, 1.f);
	weight.y = std::clamp(weight.y, 0.f, 1.f);

	const float speedX = CalculateSpeed(dir.x, mExtraOffset.x, maxOffset.x) * weight.x;
	const float speedY = CalculateSpeed(dir.y, mExtraOffset.y, maxOffset.y) * weight.y;


	auto MoveOffset = [&](float& offset, float maxOffset, float dir, float speed) {
		const float movement = (dir * speed) * DeltaTime();

		// 최대치보다 작거나 반대방향 이동 시 speed 그대로 적용
		if (fabs(offset) < maxOffset || Math::Sign(offset) != Math::Sign(dir)) {
			offset += movement;
		}
		// maxOffset을 넘어선 경우 maxOffset까지 도달하게 한다.
		else {
			offset -= movement;
			if (fabs(offset) < maxOffset) {
				offset = maxOffset * Math::Sign(offset);
			}
		}
		};

	MoveOffset(mExtraOffset.x, maxOffset.x, dir.x, speedX);
	MoveOffset(mExtraOffset.y, maxOffset.y, dir.y, speedY);

	mExtraOffset.x = std::clamp(mExtraOffset.x, -mMaxOffset.x, mMaxOffset.x);
	mExtraOffset.y = std::clamp(mExtraOffset.y, -mMaxOffset.y, mMaxOffset.y);
}

void Script_MainCamera::ZoomIn()
{
	mZoomAmount -= DeltaTime();
	if (mZoomAmount < mkMaxZoomIn) {
		mZoomAmount = mkMaxZoomIn;
	}
}

void Script_MainCamera::ZoomOut()
{
	mZoomAmount += DeltaTime();
	if (mZoomAmount > mkMaxZoomOut) {
		mZoomAmount = mkMaxZoomOut;
	}
}

void Script_MainCamera::Init()
{
	constexpr float kMaxPlaneDistance = 500.f;

	mTarget = GameFramework::I->GetPlayer();

	SetCameraOffset(kCameraOffset);
	mObject->SetPosition(mTarget->GetPosition() + mMainOffset);
	LookTarget();

	MAIN_CAMERA->SetProjMtx(0.01f, kMaxPlaneDistance, 60.f);
}


void Script_MainCamera::LookTarget()
{
	if (mTarget) {
		Vec3 targetPos = mTarget->GetPosition();
		targetPos.y += 1.f;
		MAIN_CAMERA->LookAt(targetPos, Vector3::Up);
	}
}

void Script_MainCamera::RecoverExtraOffset()
{
	auto Recover = [](float& offset) {
		constexpr float speedDecDistance = 1.f;
		float recoverSpeed = 0.3f;
		// 거리가 [speedDecDistance] 미만일 때, 0에 근접할 수록(offset에 비례) 속도를 감소한다.
		if (fabs(offset) < speedDecDistance) {
			recoverSpeed *= fabs(offset) / speedDecDistance;
		}
		if (fabs(offset) > 0.1f) {
			offset -= Math::Sign(offset) * recoverSpeed * DeltaTime();
		}
		};

	Recover(mExtraOffset.x);
	Recover(mExtraOffset.y);
}

void Script_MainCamera::Shake()
{
	if (mCrntShakeTime <= 0.f) {
		mShakeOffset = Vector3::Zero;
		return;
	}

	mCrntShakeTime -= DeltaTime();

	mShakeOffset.x += ((rand() % 60) - 30) * mShakeAmount;
	mShakeOffset.y += ((rand() % 60) - 30) * mShakeAmount;
	mShakeOffset.z += ((rand() % 60) - 30) * mShakeAmount;

	mShakeOffset.x = std::clamp(mShakeOffset.x, -0.05f, 0.05f);
	mShakeOffset.y = std::clamp(mShakeOffset.y, -0.05f, 0.05f);
	mShakeOffset.z = std::clamp(mShakeOffset.z, -0.05f, 0.05f);
}

void Script_MainCamera::HideObstacles()
{
	Vec3 rayDir = MAIN_CAMERA->ScreenToWorldRay(Vec2::One);
	rayDir.Normalize();
	Ray rayCenter{};
	rayCenter.Position = mObject->GetPosition();
	rayCenter.Direction = rayDir;

	constexpr float kSideWidth = 2.f;
	const Ray rayLeft = { mObject->GetPosition() + Vec3(-kSideWidth, 0, 0), rayDir};
	const Ray rayRight = { mObject->GetPosition() + Vec3(kSideWidth, 0, 0), rayDir};

	const std::vector<sptr<Grid>>& grids = BattleScene::I->GetNeighborGrids(BattleScene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
	for (const auto& grid : grids) {
		for (const auto& object : grid->GetObjectsFromTag(ObjectTag::Building)) {
			const auto& collider = object->GetCollider();

			float dist = 0.f;
			if (collider->Intersects(rayCenter, dist)) {
				if (collider->Intersects(rayLeft, dist) || collider->Intersects(rayRight, dist)) {
					object->Hide();
				}
			}
		}
	}
}

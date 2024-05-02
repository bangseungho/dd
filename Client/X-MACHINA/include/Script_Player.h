#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Animator;
class AnimatorMotion;
class AnimatorController;
class Script_Weapon;
class Script_AimController;
class Script_MainCamera;
#pragma endregion


#pragma region EnumClass
// 플레이어 객체 타입
enum class PlayerType {
	Unspecified = 0,
	Tank,
	Airplane,
	Human
};
#pragma endregion


#pragma region Class
class Script_Player abstract : public Component {
	COMPONENT_ABSTRACT(Script_Player, Component)

protected:
	Script_MainCamera* mCamera{};

	PlayerType		mPlayerType{};
	GameObject*		mPlayer{};		// self GameObject
	Matrix			mSpawnTransform{};	// 리스폰 지점

	int		mScore{};
	float	mMaxHP{};
	float	mHP{};

public:
	PlayerType GetPlayerType() const { return mPlayerType; }

	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	// player를 [pos]로 위치시키고 해당 위치를 리스폰 지점으로 설정한다.
	void SetSpawn(const Vec3& pos);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() {}
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}

	virtual void Rotate(float pitch, float yaw, float roll);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void AddScore(int score);
};





// 총알을 발사할 수 있는 플레이어
class Script_ShootingPlayer abstract : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

private:
	int mCrntWeaponIdx{};
	int mNextWeaponIdx{};
	bool mIsInDraw{};
	bool mIsInPutback{};

protected:
	sptr<GameObject> mWeapon{};
	sptr<Script_Weapon> mWeaponScript{};
	std::vector<sptr<GameObject>> mWeapons{};
	Transform* mMuzzle{};

public:
	virtual void Start() { base::Start();  InitWeapons(); }

public:
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	bool IsInGunChangeMotion() const { return IsInDraw() || IsInPutBack(); }
	bool IsInDraw() const { return mIsInDraw; }
	bool IsInPutBack() const { return mIsInPutback; }

	virtual void BulletFired() {}

protected:
	int GetCrntWeaponIdx() const { return mCrntWeaponIdx; }
	int GetNextWeaponIdx() const { return mNextWeaponIdx; }

	virtual void InitWeapons() abstract;

	virtual void SetWeapon(int weaponIdx);

	virtual void DrawWeaponStart(int weaponIdx, bool isDrawImmed) abstract;
	virtual void DrawWeapon();
	virtual void DrawWeaponEnd();
	virtual void PutbackWeapon() abstract;
	virtual void PutbackWeaponEnd();

	virtual void StartFire();
	virtual void StopFire();
	virtual void Reload();
};




// 지상 플레이어
class Script_GroundPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_GroundPlayer, Script_ShootingPlayer)

private:
	// constants //
	static const float mkSitWalkSpeed;
	static const float mkStandWalkSpeed;
	static const float mkRunSpeed;
	static const float mkSprintSpeed;

	static const float mkStartRotAngle;

	// animator //
	float mParamV{}, mParamH{};
	sptr<Animator> mAnimator{};
	sptr<AnimatorController> mController{};

	std::unordered_map<int, sptr<AnimatorMotion>> mReloadMotions;

	// movement //
	PlayerMotion mPrevMovement{};
	float mMovementSpeed{};
	float mRotationSpeed{};

	// aim //
	bool mIsAim{};
	bool mIsInBodyRotation{};
	float mCrntSpineAngle{};
	float mSpineDstAngle{};
	float mAimingDeltaTime{};
	float mReloadingDeltaTime{};
	Transform* mSpineBone{};
	sptr<Script_AimController> mAimController{};

	// recoil //
	int   mRecoilSign{};
	float mCurRecoil{};
	float mMaxRecoil{20.f};

public:
	PlayerMotion GetPrevState() const  { return PlayerMotion::GetState(mPrevMovement); }
	PlayerMotion GetPrevMotion() const { return PlayerMotion::GetMotion(mPrevMovement); }
	Transform* GetSpineBone() { return mSpineBone; }

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(Dir dir, float v, float h, float rotAngle);
	virtual void ProcessInput() override;

	// direction 방향으로 이동한다.
	virtual void Move(Dir dir);
	// [dir]방향을 바라보도록 회전한다.
	void RotateTo(Dir dir);

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	// called by weapon //
	void StartReload();
	// called by callback //
	void EndReload();

	virtual void BulletFired() override;

private:
	void InitWeapons();
	virtual void DrawWeaponStart(int weaponIdx, bool isDrawImmed) override;
	virtual void DrawWeaponCallback();
	virtual void DrawWeaponEndCallback();
	virtual void PutbackWeapon() override;
	virtual void PutbackWeaponEndCallback();
	void UpdateParam(float val, float& param);

	void UpdateMovement(Dir dir);

	float GetAngleMuzzleToAim(const Vec3& aimWorldPos) const;
	float GetAngleSpineToAim(const Vec3& aimWorldPos) const;
	Vec3 GetAimWorldPos(const Vec2& aimScreenPos) const;
	void RotateToAim(Dir dir, float& rotAngle);

	// angle 만큼 서서히 회전한다.
	void Rotate(float angle) const;
	void RotateMuzzleToAim();

	void OnAim();
	void OffAim();

	// called by itself
	virtual void Reload() override;
	void StopReload();

	void SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState);
	void SetMotion(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState, PlayerMotion& crntMotion);

	void StopReloadCallback();
	void ChangeReloadCallback();
	void EndReloadCallback();

	void ResetAimingTime() { mAimingDeltaTime = 0.f; }
	void RecoverRecoil();

	void MoveCamera(Dir dir);
};

#pragma endregion
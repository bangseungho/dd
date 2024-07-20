#pragma once


#pragma region Include
#include "Script_LiveObject.h"
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
class UI;
class SliderBarUI;
class ChatBoxUI;
#pragma endregion


#pragma region Class
class Script_Player abstract : public Script_LiveObject {
	COMPONENT_ABSTRACT(Script_Player, Script_LiveObject)

protected:
	Script_MainCamera* mCamera{};

	GameObject*		mTarget{};		// self GameObject
	Matrix			mSpawnTransform{};	// 리스폰 지점

	int				mScore{};
	sptr<SliderBarUI> mHpBarUI{};
	sptr<ChatBoxUI> mChatBoxUI{};

public:
	bool IsActiveChatBox() const;

	// player를 [pos]로 위치시키고 해당 위치를 리스폰 지점으로 설정한다.
	void SetSpawn(const Vec3& pos);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual bool ProcessInput();
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	virtual void Rotate(float pitch, float yaw, float roll);

	virtual bool Hit(float damage, Object* instigator) override;
	virtual void Dead() override;
	virtual void Respawn();

	void AddScore(int score);
};





// 총알을 발사할 수 있는 플레이어
class Script_ShootingPlayer abstract : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

private:
	int mCrntWeaponNum{};
	int mNextWeaponNum{};
	bool mIsInDraw{};
	bool mIsInPutback{};

protected:
	sptr<GameObject> mWeapon{};
	sptr<Script_Weapon> mWeaponScript{};
	std::vector<sptr<GameObject>> mWeapons{};
	Transform* mMuzzle{};

public:
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	bool IsInGunChangeMotion() const { return IsInDraw() || IsInPutBack(); }
	bool IsInDraw() const { return mIsInDraw; }
	bool IsInPutBack() const { return mIsInPutback; }

	virtual void BulletFired() {}

protected:
	int GetCrntWeaponIdx() const { return mCrntWeaponNum - 1; }
	int GetCrntWeaponNum() const { return mCrntWeaponNum; }
	int GetNextWeaponNum() const { return mNextWeaponNum; }

	virtual void DrawWeapon(int weaponNum);

	virtual void DrawWeaponStart(int weaponNum, bool isDrawImmed) abstract;
	virtual void DrawWeapon();
	virtual void DrawWeaponEnd();
	virtual void PutbackWeapon() abstract;
	virtual void PutbackWeaponEnd();
	virtual void DropWeapon(int weaponIdx);


	virtual void StartFire();
	virtual void StopFire();
	virtual bool Reload();
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
	float mCrntYawAngle{};
	Transform* mSpineBone{};
	sptr<Script_AimController> mAimController{};

	// recoil //
	int   mRecoilSign{};
	float mCurRecoil{};
	float mMaxRecoil{20.f};

	// sliding vector //
	Vec3 mDirVec{};
	Vec3 mPrevDirVec{};
	Vec3 mSlideVec{};

	// others
	bool mIsInteracted{};

	// For Network
	Vec3 mPrevPos{};
	Vec3 mCurrPos{};
public:
	Vec3 GetPrevPos() { return mPrevPos; }
	Vec3 GetcurrPos() { return mCurrPos; }

public:
	PlayerMotion GetPrevState() const  { return PlayerMotion::GetState(mPrevMovement); }
	PlayerMotion GetPrevMotion() const { return PlayerMotion::GetMotion(mPrevMovement); }
	bool IsReloading() const;
	Transform* GetSpineBone() { return mSpineBone; }

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(Dir dir, float v, float h, float rotAngle);
	virtual bool ProcessInput() override;

	// direction 방향으로 이동한다.
	virtual void Move(Dir dir);
	// [dir]방향을 바라보도록 회전한다.
	void RotateTo(Dir dir);

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	void InitWeaponAnimations();
	// called by weapon //
	void StartReload();
	void BoltAction();
	// called by callback //
	void EndReload();

	virtual void BulletFired() override;

	float GetMovementSpeed() const { return mMovementSpeed; }
	float GetRotationSpeed() const { return mRotationSpeed; }

	void AquireNewWeapon(WeaponName weaponName);
	void TakeWeapon(rsptr<Script_Weapon> weapon);

	Vec3  GetMoveDir()			   { return mDirVec; }
	Vec3  GetPrevMoveDir()		   { return mPrevDirVec; }
private:
	virtual void DrawWeaponStart(int weaponNum, bool isDrawImmed) override;
	virtual void DrawWeaponCallback();
	virtual void DrawWeaponEndCallback();
	virtual void PutbackWeapon() override;
	virtual void PutbackWeaponEndCallback();
	virtual void DropWeapon(int weaponIdx) override;
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
	virtual bool Reload() override;
	void StopReload();

	void SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState);
	void SetMotion(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState, PlayerMotion& crntMotion);

	void StopReloadCallback();
	void ChangeReloadCallback();
	void EndReloadCallback();
	void BoltActionCallback();

	void ResetAimingTime() { mAimingDeltaTime = 0.f; }
	void RecoverRecoil();

	void MoveCamera(Dir dir);

	// [time] 내에 [motion]이 끝나도록 [motion]의 속도를 변경한다.
	void SetMotionSpeed(rsptr<AnimatorMotion> motion, float time);
	void ComputeSlideVector(Object& other);

	void SwitchWeapon(int weaponIdx, rsptr<GameObject> weapon);

	void Interact();
};


// 페로 능력을 가진 플레이어
class Script_PheroPlayer : public Script_GroundPlayer {
	COMPONENT(Script_PheroPlayer, Script_GroundPlayer)

protected:
	float mStartPheroAmount{};
	float mCurrPheroAmount{};
	float mMaxPheroAmount{};
	float mPheroRegenRate{};
	sptr<SliderBarUI> mPheroBarUI{};

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void Respawn() override;

public:
	virtual void AddPheroAmount(float pheroAmount);
	virtual bool ReducePheroAmount(float pheroCost, bool checkOnly = false);
};
#pragma endregion
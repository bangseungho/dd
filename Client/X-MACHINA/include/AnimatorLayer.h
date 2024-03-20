#pragma once
#include "HumanBone.h"

class AnimatorMotion;
class AnimatorStateMachine;
class AnimatorController;

class AnimatorLayer {
private:
	bool mIsReverse{};
	bool mIsSyncSM{};
	sptr<const AnimatorMotion> mSyncState{};

	std::string mName{};
	HumanBone mBoneMask{};

	const AnimatorController* mController{};
	sptr<AnimatorStateMachine> mRootStateMachine{};

	sptr<AnimatorMotion>				mCrntState{};
	std::vector<sptr<AnimatorMotion>>	mNextStates{};

public:
	AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	std::string GetName() const { return mName; }
	sptr<const AnimatorMotion> GetSyncState() const { return mNextStates.empty() ? mCrntState : mNextStates.back(); }
	Vec4x4 GetTransform(int boneIndex, HumanBone boneType) const;

	void SetCrntStateLength(float length) const;
	void SetSyncStateMachine(bool val) { mIsSyncSM = val; }

public:
	void Init(const AnimatorController* controller);

	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	void CheckTransition(const AnimatorController* controller);

	void SyncAnimation(rsptr<const AnimatorMotion> srcState);

private:
	void SyncComplete();
	void ChangeState(rsptr<AnimatorMotion> state);
};
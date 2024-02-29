#pragma once

struct AnimatorTransition;
class AnimationClip;
class AnimatorStateMachine;
 
// AnimationClip의 재생을 관리한다.
class AnimatorState {
private:
	std::string mName{};

	const AnimatorStateMachine* mStateMachine{};
	sptr<const AnimationClip> mClip{};

	std::vector<sptr<const AnimatorTransition>> mTransitions{};

	float 	mSpeed      = .5f;
	float 	mCrntLength = 0.f;
	float 	mWeight     = 1.f;

public:
	AnimatorState(rsptr<const AnimatorStateMachine> staeMachine, rsptr<const AnimationClip> clip, const std::vector<sptr<const AnimatorTransition>>& transitions);
	AnimatorState(const AnimatorState& other);
	virtual ~AnimatorState() = default;

	Vec4x4 GetSRT(int boneIndex) const;
	float GetCrntLength() const { return mCrntLength; }
	float GetWeight() const { return mWeight; }
	rsptr<const AnimationClip> GetClip() const { return mClip; }
	std::string GetName() const { return mName; }

	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	void Init();
	bool Animate();

private:
	bool IsEndAnimation();
};
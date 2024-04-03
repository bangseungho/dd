#include "EnginePch.h"
#include "AnimatorMotion.h"

#include "AnimatorStateMachine.h"
#include "AnimationClip.h"
#include "Timer.h"

AnimatorMotion::AnimatorMotion(const AnimatorMotionInfo& info)
	:
	mName(info.Name),
	mkOriginSpeed(info.Speed),
	mCrntSpeed(info.Speed),
	mCrntLength(0),
	mMaxLength(info.Length),
	mWeight(1),
	mStateMachine(info.StateMachine.get()),
	mTransitions(info.Transitions)
{

}

AnimatorMotion::AnimatorMotion(const AnimatorMotion& other)
	:
	mName(other.mName),
	mkOriginSpeed(other.mkOriginSpeed),
	mCrntSpeed(other.mCrntSpeed),
	mCrntLength(other.mCrntLength),
	mMaxLength(other.mMaxLength),
	mWeight(other.mWeight),
	mStateMachine(other.mStateMachine),
	mTransitions(other.mTransitions)
{
}

void AnimatorMotion::ResetLength()
{
	if (IsReverse()) {
		mCrntLength = mMaxLength;
	}
	else {
		mCrntLength = 0.f;
	}
}

void AnimatorMotion::SetLength(float length)
{
	mCrntLength = length;
	if (IsEndAnimation()) {
		ResetLength();
	}
}

void AnimatorMotion::AddCallback(const std::function<void()>& callback, int frame)
{
	MotionCallback motionCallback{ GetFrameTime(frame), callback };

	if (mCallbackList.empty()) {
		mCallbackList.push_back(motionCallback);
		return;
	}

	for (auto& it = mCallbackList.begin(); ; ++it) {
		if (it == mCallbackList.end()) {
			mCallbackList.push_back(motionCallback);
			break;
		}
		else if (it->Time > motionCallback.Time) {
			mCallbackList.insert(it, motionCallback);
			break;
		}
	}
}

void AnimatorMotion::Reset()
{
	mCrntLength = 0.f;
	mWeight     = 1.f;
	mCrntSpeed  = mkOriginSpeed;
	mIsReverse  = 1;
}


bool AnimatorMotion::IsEndAnimation() const
{
	return (mCrntLength >= mMaxLength) || (mCrntLength <= 0);
}

bool AnimatorMotion::IsSameStateMachine(rsptr<const AnimatorMotion> other) const
{
	return mStateMachine == other->mStateMachine;
}

bool AnimatorMotion::Animate()
{
	mCrntLength += (mCrntSpeed * mIsReverse) * DeltaTime();
	if (IsEndAnimation()) {
		ResetLength();
		for (auto& callback : mCallbackList) {
			callback.Reset();
		}
		return true;
	}

	for (auto& callback : mCallbackList) {
		if (mCrntLength > callback.Time) {
			if (!callback.Called) {
				callback.Callback();
				callback.Called = true;
				break;
			}
		}
	}

	return false;
}

void AnimatorMotion::DecWeight()
{
	mWeight -= DeltaTime() * mkTransitionSpeed;
}

void AnimatorMotion::IncWeight()
{
	mWeight += DeltaTime() * mkTransitionSpeed;
}


AnimatorTrack::AnimatorTrack(rsptr<const AnimationClip> clip)
	:
	mClip(clip)
{

}

AnimatorTrack::AnimatorTrack(const AnimatorTrack& other)
{
	mClip = other.mClip;
}

Matrix AnimatorTrack::GetSRT(int boneIndex, float length) const
{
	return mClip->GetSRT(boneIndex, length);
}
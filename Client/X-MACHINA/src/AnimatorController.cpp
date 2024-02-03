#include "stdafx.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorState.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Timer.h"

AnimatorController::AnimatorController(const std::unordered_map<std::string, AnimatorParameter>& parameters, const std::unordered_map<std::string, sptr<AnimatorState>>& states)
	:
	mParameters(parameters),
	mStates(states)
{
	mCrntState = mStates.begin()->second;
}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	mParameters(other.mParameters)
{
	for (const auto& otherState : other.mStates) {
		sptr<AnimatorState> state = std::make_shared<AnimatorState>(*otherState.second);
		mStates.insert(std::make_pair(otherState.first, state));
	}
	mCrntState = mStates.begin()->second;
}

void AnimatorController::Animate()
{
	mCrntState->Animate();

	if (mNextState) {
		bool isEndAnimation = mNextState->Animate();

		if (isEndAnimation) {
			ChangeToNextState();
		}
		else {
			constexpr float kMaxDuration = .25f;
			const float crntDuration = mNextState->GetCrntLength();

			const float t = crntDuration / kMaxDuration;
			if (t < 1.f) {
				mCrntState->SetWeight(1 - t);
				mNextState->SetWeight(t);
			}
			else {
				ChangeToNextState();
			}
		}
	}
}

Vec4x4 AnimatorController::GetTransform(int boneIndex)
{
	Vec4x4 transform = Matrix4x4::Scale(mCrntState->GetSRT(boneIndex), mCrntState->GetWeight());
	if (mNextState) {
		transform = Matrix4x4::Add(transform, Matrix4x4::Scale(mNextState->GetSRT(boneIndex), mNextState->GetWeight()));
	}

	return transform;
}

void AnimatorController::SetBool(const std::string& name, bool value)
{
	if (mNextState) {
		return;
	}
	if (!mParameters.contains(name)) {
		return;
	}

	mParameters[name].val.b = value;

	const std::string destination = mCrntState->CheckTransition(name, value);
	if (destination != "") {
		mNextState = mStates[destination];
		mNextState->Init();
	}
}


void AnimatorController::ChangeToNextState()
{
	if (!mNextState) {
		return;
	}

	mCrntState->Init();
	mNextState->SetWeight(1.f);
	mCrntState = mNextState;
	mNextState = nullptr;
}
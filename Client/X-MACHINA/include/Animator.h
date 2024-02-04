#pragma once

class Transform;
class AnimatorController;
class AnimationSet;
class GameObject;
class SkinMesh;

// 한 모델(MasterModel)의 파일(정보)을 읽을 때의 Animation 정보
struct AnimationLoadInfo {
	std::vector<sptr<SkinMesh>> SkinMeshes{};
	std::string					AnimatorControllerFile{};
};

// Animation의 재생 및 상태 전이 등의 전반을 관리한다.
class Animator {
private:
	sptr<AnimatorController> mController{};

	std::vector<std::vector<Transform*>>	mBoneFramesList{};
	std::vector<sptr<SkinMesh>>				mSkinMeshes{};

public:
	Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar);
	~Animator();

public:
	void UpdateShaderVariables();

	void Animate();

	void SetBool(const std::string& name, bool value);

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar);
};

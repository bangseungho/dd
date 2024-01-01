#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class MasterModel;
class Texture;
class ObjectInstanceBuffer;
class ObjectCollider;
#pragma endregion


#pragma region Struct
struct SB_StandardInst {
	Vec4x4 LocalTransform{};
};
#pragma endregion


#pragma region Class
class GameObject : public Object {
private:
	using base = Object;
	using Transform::ReturnTransform;

public:
	sptr<const MasterModel> mMasterModel{};

protected:
	bool mIsInstancing{ false };

private:
	bool mIsAwake{ false };
	bool mIsActive{ true };
	bool mIsFlyable{ false };
	bool mIsDrawBounding{ false };

	std::unordered_set<int> mGridIndices{};
	int mCurrGridIndex{ -1 };

	std::vector<const Transform*> mMergedTransform{};

	sptr<ObjectCollider> mCollider{};

public:
	GameObject();
	virtual ~GameObject() = default;

	int GetGridIndex() const { return mCurrGridIndex; }
	const std::unordered_set<int>& GetGridIndices() const { return mGridIndices; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }
	rsptr<Texture> GetTexture() const;

	rsptr<ObjectCollider> GetCollider() const { return mCollider; }

	bool IsActive() const { return mIsActive; }
	bool IsTransparent() const { return mLayer == ObjectLayer::Transparent; }
	bool IsInstancing() const { return mIsInstancing; }

	void SetInstancing() { mIsInstancing = true; }
	void SetFlyable(bool isFlyable) { mIsFlyable = isFlyable; }
	void SetGridIndex(int index) { mCurrGridIndex = index; }
	void SetGridIndices(const std::unordered_set<int>& indices) { mGridIndices = indices; }
	void SetModel(rsptr<const MasterModel> model);

public:
	virtual void Render();
	virtual void RenderBounds();

	virtual void Update();

	virtual void Enable(bool isUpdateObjectGrid = true);
	virtual void Disable(bool isUpdateObjectGrid = true);

	void ToggleDrawBoundings() { mIsDrawBounding = !mIsDrawBounding; }
	void ClearGridIndices() { mGridIndices.clear(); }

	Transform* FindFrame(const std::string& frameName);
	GameObject* FindObject(const std::string& frameName);

private:
	void AttachToGround();
	void TiltToGround();
};





class InstancinObject : public GameObject {
private:
	using base = GameObject;

	using GameObject::Render;

private:
	sptr<ObjectInstanceBuffer> mBuffer{};

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };

public:
	InstancinObject()          = default;
	virtual ~InstancinObject() = default;

	void SetBuffer(rsptr<ObjectInstanceBuffer> buffer);

public:
	virtual void Render() override;
	virtual void Update() override { mUpdateFunc(); }

private:
	void Push();
	void Reset() { mIsPushed = false; }

	void UpdateStatic();
	void UpdateDynamic();
};





class ObjectInstanceBuffer {
protected:
	UINT mCurrBuffIdx{};
	bool mIsStatic{ true };

private:
	UINT mObjectCnt{};
	sptr<const MasterModel> mMasterModel{};

	std::vector<const Transform*> mMergedTransform{};
	ComPtr<ID3D12Resource> mInstBuff{};
	SB_StandardInst* mMap_Buff{};	

public:
	ObjectInstanceBuffer()          = default;
	virtual ~ObjectInstanceBuffer() = default;

	bool IsStatic() { return mIsStatic; }
	UINT GetObjectCount() const { return mObjectCnt; }
	UINT GetInstanceCount() const { return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	void SetDynamic() { mIsStatic = false; }
	void SetModel(rsptr<const MasterModel> model);

public:
	void CreateShaderVars(UINT objectCount);
	void UpdateShaderVars() const;

	void PushObject(InstancinObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};
#pragma endregion
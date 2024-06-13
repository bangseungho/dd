#pragma once


#pragma region ClassForwardDecl
class Mesh;
class Shader;
class GameObject;
class Object;
#pragma endregion


#pragma region Ability
class Ability {
protected:
	Object*		mObject{};
	float		mCooldownTime{};
	float		mActiveTime{};
	bool		mIsToggleAbility{};

	std::function<void()> mTerminateCallback{};

public:
	Ability(float cooldownTime = 0.f, float activeTime = 0.f) : mCooldownTime(cooldownTime), mActiveTime(activeTime) {}

public:
	float GetCooldownTime() const { return mCooldownTime; }
	float GetActiveTime() const { return mActiveTime; }
	bool IsToggleAbility() const { return mIsToggleAbility; }

public:
	void SetObject(Object* object) { mObject = object; }
	void SetTerminateCallback(const std::function<void()>& callback) { mTerminateCallback = callback; }

public:
	virtual void Update(float activeTime) abstract;
	virtual void Activate() { }
	virtual void DeActivate() { }
};


class RenderedAbility : public Ability, public std::enable_shared_from_this<RenderedAbility> {
protected:
	D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	sptr<GameObject>	mRenderedObject;
	sptr<Mesh>			mRenderedMesh;

	sptr<Shader>		mShader{};
	int					mLayer{};

	int					mAbilityCBIdx = -1;
	AbilityConstants	mAbilityCB{};

public:
	RenderedAbility(float cooldownTime = 0.f, float activeTime = 0.f) : Ability(cooldownTime, activeTime) {}


public:
	int GetAbilityCBIdx() const { return mAbilityCBIdx; }
	void UpdateAbilityCB(float activeTime = 0.f);

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;
	
public:
	virtual void Render();
};
#pragma endregion


#pragma region AbilityMgr
class AbilityMgr : public Singleton<AbilityMgr> {
	friend Singleton;

private:
	enum  { MaxAbilityLayer = 10 };
	std::array<std::unordered_set<sptr<RenderedAbility>>, MaxAbilityLayer> mRenderedAbilities;

public:
	void AddRenderedAbilities(int layer, sptr<RenderedAbility> ability) { mRenderedAbilities[layer].insert(ability); }
	void RemoveRenderedAbilities(int layer, sptr<RenderedAbility> ability) { mRenderedAbilities[layer].erase(ability); }

	void Render();
};
#pragma endregion


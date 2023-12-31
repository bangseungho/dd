#include "stdafx.h"
#include "Component.h"
#include "Collider.h"



namespace {
	DWORD gkDynamiObjects {
		static_cast<DWORD>(ObjectTag::Player)			|
		static_cast<DWORD>(ObjectTag::Tank)				|
		static_cast<DWORD>(ObjectTag::Helicopter)		|
		static_cast<DWORD>(ObjectTag::Bullet)			|
		static_cast<DWORD>(ObjectTag::ExplosiveBig)		|
		static_cast<DWORD>(ObjectTag::ExplosiveSmall)
	};

	DWORD gkDynamicMoveObjects {
		static_cast<DWORD>(ObjectTag::Player)			|
		static_cast<DWORD>(ObjectTag::Tank)				|
		static_cast<DWORD>(ObjectTag::Helicopter)		|
		static_cast<DWORD>(ObjectTag::Bullet)
	};

	DWORD gkEnvironmentObjects{
		static_cast<DWORD>(ObjectTag::Unspecified)		|
		static_cast<DWORD>(ObjectTag::Background)		|
		static_cast<DWORD>(ObjectTag::Billboard)		|
		static_cast<DWORD>(ObjectTag::Terrain)
	};
}



#pragma region Object
void Object::SetTag(ObjectTag tag)
{
	mTag  = tag;
	mType = GetObjectType(tag);
}

void Object::CopyComponents(const Object& src)
{
	const auto& components = src.GetAllComponents();
	for (auto& component : components) {
		sptr<Component> copy = GetCopyComponent(component);
		mComponents.emplace_back(copy);
	}
}

void Object::Start()
{
	Transform::Update();
	StartComponents();
}

void Object::Update()
{
	Transform::Update();
	UpdateComponents();
}

void Object::Release()
{
	ReleaseComponents();
}
void Object::ReleaseUploadBuffers()
{
	ProcessComponents([](sptr<Component> component) {
		component->ReleaseUploadBuffers();
		});
}

void Object::OnCollisionStay(Object& other)
{
	if (mCollisionObjects.count(&other)) {
		return;
	}
	mCollisionObjects.insert(&other);

	ProcessComponents([&other](sptr<Component> component) {
		component->OnCollisionStay(other);
		});
}



void Object::ProcessComponents(std::function<void(sptr<Component>)> processFunc) {
	for (auto& component : mComponents) {
		if (component) {
			processFunc(component);
		}
	}
}

void Object::StartComponents()
{
	ProcessComponents([](sptr<Component> component) {
		component->Start();
		});
}

void Object::UpdateComponents()
{
	mCollisionObjects.clear();
	ProcessComponents([](sptr<Component> component) {
		component->Update();
		});
}

void Object::ReleaseComponents()
{
	ProcessComponents([](sptr<Component> component) {
		component->Release();
		});
}

namespace {
	template<class T>
	sptr<T> CopyComponent(rsptr<Component> src)
	{
		sptr<T> result = std::make_shared<T>(src->mObject);
		rsptr<T> other = static_pointer_cast<T>(src);
		*result        = *other;
		return result;
	}
}

sptr<Component> Object::GetCopyComponent(rsptr<Component> component)
{
	sptr<Component> result{};

	switch (component->GetID()) {
	case BoxCollider::ID:
		result = CopyComponent<BoxCollider>(component);
		break;
	case SphereCollider::ID:
		result = CopyComponent<SphereCollider>(component);
		break;
	case ObjectCollider::ID:
		result = CopyComponent<ObjectCollider>(component);
		break;
	default:
		assert(0);
		break;
	}

	if (result) {
		result->mObject = this;
	}

	return result;
}
#pragma endregion


#pragma region Functions
ObjectTag GetTagByName(const std::string& name)
{
	switch (Hash(name)) {
	case Hash("Building"):
		return ObjectTag::Building;

	case Hash("Explosive_small"):
	case Hash("Explosive_static"):
		return ObjectTag::ExplosiveSmall;

	case Hash("Explosive_big"):
		return ObjectTag::ExplosiveBig;

	case Hash("Tank"):
		return ObjectTag::Tank;

	case Hash("Helicopter"):
		return ObjectTag::Helicopter;

	case Hash("Background"):
		return ObjectTag::Background;

	case Hash("Billboard"):
		return ObjectTag::Billboard;

	case Hash("Sprite"):
		return ObjectTag::Sprite;

	default:
		//assert(0);
		break;
	}

	return ObjectTag::Unspecified;
}

ObjectLayer GetLayerByNum(int num)
{
	switch (num) {
	case 0:
		return ObjectLayer::Default;

	case 3:
		return ObjectLayer::Transparent;

	case 4:
		return ObjectLayer::Water;

	default:
		assert(0);
		break;
	}

	return ObjectLayer::Default;
}

ObjectType GetObjectType(ObjectTag tag)
{
	if (static_cast<DWORD>(tag) & gkDynamicMoveObjects) {
		return ObjectType::DynamicMove;
	}
	else if (static_cast<DWORD>(tag) & gkDynamiObjects) {
		return ObjectType::Dynamic;
	}
	else if (static_cast<DWORD>(tag) & gkEnvironmentObjects) {
		return ObjectType::Environment;
	}

	return ObjectType::Static;
}
#pragma endregion

#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class BoxCollider;
class SphereCollider;
class ObjectCollider;
#pragma endregion


#pragma region Class
class Collider abstract : public Component {
	COMPONENT_ABSTRACT(Collider, Component)

	friend ObjectCollider;

public:
	enum class Type { None, Box, Sphere };
	virtual Type GetType() const abstract;
	virtual Vec3 GetCenter() const abstract;

	BoxCollider* GetBoxCollider();
	SphereCollider* GetSphereCollider();

public:
	virtual void OnEnable() override;

public:
	virtual void Render(Vec4 color = Vec4::Zero) const {};

	virtual bool Intersects(rsptr<Collider> other) const abstract;
	virtual bool Intersects(const BoundingBox& bb) const abstract;
	virtual bool Intersects(const BoundingOrientedBox& obb) const abstract;
	virtual bool Intersects(const BoundingSphere& bs) const abstract;
	virtual bool Intersects(const Ray& ray, float& dist) const abstract;

protected:
	virtual void UpdateTransform() abstract;
};




// basic cuboid-shaped collision primitive.
class BoxCollider : public Collider {
	COMPONENT(BoxCollider, Collider)

public:
	MyBoundingOrientedBox mBox{};

public:
	BoxCollider& operator=(const BoxCollider& other);

public:
	virtual Type GetType() const { return Type::Box; }
	virtual Vec3 GetCenter() const { return mBox.Center; }

public:
	virtual void Update() override;
	virtual void Render(Vec4 color = Vec4::Zero) const override;

	virtual bool Intersects(rsptr<Collider> other) const override;
	virtual bool Intersects(const BoundingBox& bb) const override;
	virtual bool Intersects(const BoundingOrientedBox& obb) const override;
	virtual bool Intersects(const BoundingSphere& bs) const override;
	virtual bool Intersects(const Ray& ray, float& dist) const override;

protected:
	virtual void UpdateTransform() override;
};




// basic sphere-shaped collision primitive.
class SphereCollider : public Collider {
	COMPONENT(SphereCollider, Collider)

public:
	MyBoundingSphere mBS{};

public:
	SphereCollider& operator=(const SphereCollider& other);

public:
	virtual Type GetType() const { return Type::Sphere; }
	virtual Vec3 GetCenter() const { return mBS.Center; }

public:
	virtual void Update() override;
	virtual void Render(Vec4 color = Vec4::Zero) const override;

	virtual bool Intersects(rsptr<Collider> other) const override;
	virtual bool Intersects(const BoundingBox& bb) const override;
	virtual bool Intersects(const BoundingOrientedBox& obb) const override;
	virtual bool Intersects(const BoundingSphere& bs) const override;
	virtual bool Intersects(const Ray& ray, float& dist) const override;

protected:
	virtual void UpdateTransform() override;
};



// for collision check
class ObjectCollider : public Component {
	COMPONENT(ObjectCollider, Component)

	template <typename T>
	static constexpr bool is_valid_collider_type = (std::is_same<T, BoundingBox>::value
												 || std::is_same<T, BoundingOrientedBox>::value
												 || std::is_same<T, MyBoundingOrientedBox>::value
												 || std::is_same<T, BoundingSphere>::value
												 || std::is_same<T, MyBoundingSphere>::value);

private:
	sptr<SphereCollider>		mSphereCollider{};	// (객체 전체를 감싸는)SphereCollider가 반드시 있어야 하며 하나만 존재해야 한다.
	std::vector<sptr<Collider>>	mColliders{};		// 모든 colliders

public:
	const MyBoundingSphere& GetBS() const { return mSphereCollider->mBS; }
	const std::vector<sptr<Collider>>& GetColliders() const { return mColliders; }

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	void Render(Vec4 color = Vec4::Zero) const;

	bool Intersects(const ObjectCollider* other) const;
	bool Intersects(const BoundingFrustum& frustum) const { return frustum.Intersects(mSphereCollider->mBS); }
	bool Intersects(rsptr<Collider> collider) const;

	template<class T, typename std::enable_if<is_valid_collider_type<T>>::type* = nullptr>
	bool Intersects(const T& bounding) const
	{
		for (auto& collider : mColliders) {
			if (collider->Intersects(bounding)) {
				return true;
			}
		}

		return false;
	}

	// 두 객체의 충돌 여부를 반환한다.
	static bool Intersects(const GridObject& a, const GridObject& b);
};
#pragma endregion
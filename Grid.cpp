#include "stdafx.h"
#include "Grid.h"
#include "Object.h"
#include "Collider.h"

// [ Grid ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Grid::Grid()
{
}
Grid::~Grid()
{

}

void Grid::Init(int index, const BoundingBox& bb)
{
	mIndex = index;
	mBB = bb;
}

void Grid::AddObject(GameObject* object)
{
	mObjects.insert(object);

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamiObjects.insert(object);
		break;
	case ObjectType::Environment:
		mEnvObjects.insert(object);
		break;
	default:
		mStatiObjects.insert(object);
		break;
	}
}
void Grid::RemoveObject(GameObject* object)
{
	mObjects.erase(object);
	mEnvObjects.erase(object);
	mDynamiObjects.erase(object);
	mStatiObjects.erase(object);
}

bool ProcessCollision(GameObject* objectA, GameObject* objectB)
{
	if (objectA->GetComponent<ObjectCollider>()->Intersects(*objectB)) {
		objectA->OnCollisionStay(*objectB);
		objectB->OnCollisionStay(*objectA);

		return true;
	}

	return false;
}

// check collision for each object in objects
void CheckCollisionObjects(std::unordered_set<GameObject*> objects)
{
	for (auto& a = objects.begin(); a != std::prev(objects.end()); ++a) {
		auto& objectA = *a;

		for (auto& b = std::next(a); b != objects.end(); ++b) {
			auto& objectB = *b;

			ProcessCollision(objectA, objectB);
		}
	}
}

// check collision for each object in each objects
void CheckCollisionObjects(std::unordered_set<GameObject*> objects1, std::unordered_set<GameObject*> objects2)
{
	for (auto& a = objects1.begin(); a != objects1.end(); ++a) {
		auto& objectA = *a;

		for (auto& b = objects2.begin(); b != objects2.end(); ++b) {
			auto& objectB = *b;

			ProcessCollision(objectA, objectB);
		}
	}
}

void Grid::CheckCollisions()
{
	if (!mDynamiObjects.empty()) {
		::CheckCollisionObjects(mDynamiObjects);
		if (!mStatiObjects.empty()) {
			::CheckCollisionObjects(mDynamiObjects, mStatiObjects);
		}
	}
}
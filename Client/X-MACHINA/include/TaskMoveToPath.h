#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskMoveToPath : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

	float mMoveSpeed{};
	float mReturnSpeed{};
	std::stack<Vec3>* mPath;

public:
	TaskMoveToPath(Object* object);
	virtual ~TaskMoveToPath() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
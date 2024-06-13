#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region Class
class Script_MindControlledEnemyBT : public Script_BehaviorTree {
	COMPONENT(Script_MindControlledEnemyBT, Script_BehaviorTree)

protected:
	BT::Node* SetupTree() override;
};
#pragma endregion


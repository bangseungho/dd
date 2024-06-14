#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class UI;
struct UITexture;
#pragma endregion

#pragma region Class
class Script_AimController : public Component {
	COMPONENT(Script_AimController, Component)

private:
	sptr<UI> mUI{};
	Vec2 mMousePos{};

	Vec2 mMaxPos{};

	float mouseSensitivity{ 2.f };

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	Vec2 GetAimDirection() const { return Vector2::Normalized(mMousePos); }
	Vec2 GetAimPos() const { return mMousePos; }
	Vec2 GetScreenAimPos() const;
	
	rsptr<UITexture> GetUITexture();

public:
	void ChangeAimUITexture(rsptr<UITexture> newUITexture);
};
#pragma endregion
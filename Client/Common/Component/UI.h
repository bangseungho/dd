#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Canvas;
class Texture;
class Shader;
class ModelObjectMesh;
#pragma endregion


#pragma region Class
// 2D object (not entity)
class UI : public Transform {
protected:
	bool			mIsActive = true;
	sptr<Texture>	mTexture{};		// image
	sptr<Shader>	mShader{};
	float			mWidth{};
	float			mHeight{};
	ObjectConstants mObjectCB{};

public:
	// [texture]를 설정하고, [pos]위치에 [width * height] 크기의 UI를 생성한다.
	UI(rsptr<Texture> texture, Vec2 pos, float width, float height, rsptr<Shader> shader = nullptr);
	virtual ~UI() = default;

public:
	virtual void Update() {}
	virtual void Render();

	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }
	void SetWidth(float width) { mWidth = width; }
	void SetHeight(float height) { mHeight = height; }

	void SetPosition(float x, float y, float z);
	void SetPosition(const Vec2& pos);
	void SetPosition(const Vec3& pos);

	void SetActive(bool val) { mIsActive = val; }

protected:
	virtual void UpdateShaderVars();
};



// for render font
// comdef.h의 Font와 중복되어 이름 변경 Font -> MyFont
class MyFont : public UI {
private:
	std::string mText{};	// "YOUR SCORE IS "
	std::string mScore{};

public:
	// [pos]위치에 [width * height] 크기의 UI를 생성한다.
	MyFont(const Vec2& pos, float width, float height);
	virtual ~MyFont() = default;

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }

public:
	virtual void Render() override;

private:
	void UpdateShaderVars(char ch, int cnt) const;
};


class SliderUI : public UI {
private:
	float mMinValue = 0.f;
	float mMaxValue = 1.f;
	float mValue{};			// 0~1 normalize value

public:
	SliderUI(rsptr<Texture> texture, const Vec2& pos, float width, float height, rsptr<Shader> shader = nullptr);
	virtual ~SliderUI() = default;

public:
	void SetMinMaxValue(float min = 0.f, float max = 1.f) { mMinValue = min, mMaxValue = max; }
	void SetValue(float value) { mValue = value / mMaxValue; }

protected:
	virtual void UpdateShaderVars() override;
};



// Canvas 위에 UI를 그리도록 한다.
class Canvas : public Singleton<Canvas> {
	friend Singleton;
	using Layer = int;

private:
	static constexpr UINT8 mkLayerCnt = 5;
	std::array<std::unordered_set<sptr<UI>>, mkLayerCnt> mUIs{}; // all UIs
	sptr<MyFont> mFont{};

	float mWidth{};
	float mHeight{};

private:
	Canvas() = default;
	virtual ~Canvas() = default;

public:
	void SetScore(int score);

	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }

public:
	void Init();
	void BuildUIs();

	void Update();
	void Render() const;

	sptr<UI> CreateUI(Layer layer, const std::string& texture, const Vec2& pos, float width, float height, const std::string& shader = "Rect");
	sptr<SliderUI> CreateSliderUI(Layer layer, const std::string& texture, const Vec2& pos, float width, float height, const std::string& shader = "Rect");
};
#pragma endregion

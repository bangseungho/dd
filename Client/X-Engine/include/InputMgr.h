#pragma once

#pragma region Define
#define KEY_NONE(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::None
#define KEY_TAP(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::Tap
#define KEY_PRESSED(key)	InputMgr::Inst()->GetKeyState(key) == KeyState::Pressed
#define KEY_AWAY(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::Away
#pragma endregion



	enum class KEY_STATE : UINT8
	{
		TAP,     // 막 눌림
		PRESSED, // TAP 이후에도 계속 눌려있음
		AWAY,    // 막 뗀 상황
		NONE,    // 안눌린 상태
	};


	enum class KEY : UINT8
	{
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L,
		Z, X, C, V, B, N, M,
		LEFT, RIGHT, UP, DOWN,
		_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
		LSHFT, LALT, LCTRL, SPACE, ENTER, BACK, ESC, TAB, DEL,
		LBTN, RBTN, MBTN,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

		KEY_END,
	};


#pragma region EnumClass
enum class KeyState {
	None = 0,
	Tap,
	Pressed,
	Away
};
#pragma endregion


#pragma region Struct
struct KeyInfo {
	KeyState	State{};
	bool		IsPrevPressed{};
};
#pragma endregion


#pragma region Class
class InputMgr : public Singleton<InputMgr> {
	friend Singleton;

private:
	std::unordered_map<int, KeyInfo> mKeys{};

	Vec2 mMousePos{};		// 현재 마우스 위치
	Vec2 mMousePrevPos{};	// 이전 마우스 위치
	Vec2 mMouseDir{};		// 이전->현재의 마우스 이동 방향

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key).State; }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mMousePrevPos.x, mMousePos.y - mMousePrevPos.y); }

public:

	// 사용할 키들을 설정한다.
	void Init();

	// 각 키들의 정보(KeyInfo)를 업데이트한다.
	void Update();

	void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ProcessWndMsg(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
};
#pragma endregion

﻿#pragma once

class GridObject;

class Engine : public Singleton<Engine> {
	friend Singleton;

private:
	short mWindowWidth{ 1920 };
	short mWindowHeight{ 1080 };
	bool mIsWindowFocused{ true };
	std::wstring mTitle{};	// 윈도우 타이틀 문자열

public:
	Engine();
	virtual ~Engine() = default;

	short GetWindowWidth() const { return mWindowWidth; }
	short GetWindowHeight() const { return mWindowHeight; }

public:
	void Init(HINSTANCE hInstance, HWND hWnd);
	void Release();

	// call per once frame
	void Update();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void BuildObjects();

	void WindowFocusOn();
	void WindowFocusOff();
};

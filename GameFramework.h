#pragma once

#define framework Framework::Inst()

class Framework {
	SINGLETON_PATTERN(Framework)

private:
	std::wstring mTitle{};	// 윈도우 타이틀 문자열

private:
	Framework();
	virtual ~Framework() = default;

public:
	void Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	void BuildObjects();
	void ReleaseObjects();

	// call per once frame
	void FrameAdvance();
};

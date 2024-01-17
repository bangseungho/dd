#pragma once

#pragma region Define
#define dxgi DXGIMgr::Inst()
#define device dxgi->GetDevice()
#define cmdList dxgi->GetCmdList()
#pragma endregion

#pragma region ClassForwardDecl
class PostProcessingShader;
#pragma endregion


#pragma region EnumClass
enum class DrawOption {
	Main = 0,
	Texture,
	Normal,
	Depth,
};
#pragma endregion


#pragma region Class
// device, swapchain 등 DXGI 전반 및 렌더링을 관리한다.
class DXGIMgr : public Singleton<DXGIMgr> {
	friend class Singleton<DXGIMgr>;

private:
	// window
	HINSTANCE	mInstance{};
	HWND		mWnd{};			// 메인 윈도우 핸들

	// screen
	int mClientWidth{};
	int mClientHeight{};

	// device
	ComPtr<IDXGIFactory4>	mFactory{};
	ComPtr<IDXGISwapChain3> mSwapChain{};
	ComPtr<ID3D12Device>	mDevice{};

	bool mIsMsaa4xEnabled{ false };
	UINT mMsaa4xQualityLevels{};

	// swap chain
	static constexpr UINT mSwapChainBuffCnt = 2;
	static constexpr UINT mRtvCnt			= 5;
	static constexpr std::array<DXGI_FORMAT, mRtvCnt>			mRtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };	// formats of multi render target
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, mSwapChainBuffCnt>	mRtvHandles{};

	std::array<ComPtr<ID3D12Resource>, mSwapChainBuffCnt>	mSwapChainBuffers{};
	UINT													mSwapChainBuffCurrIdx{};	// current swap chain buffer index

	// view (descriptor)
	UINT mCbvSrvDescriptorIncSize{};
	UINT mRtvDescriptorIncSize{};
	ComPtr<ID3D12DescriptorHeap>	mRtvHeap{};
	ComPtr<ID3D12DescriptorHeap>	mDsvHeap{};
	ComPtr<ID3D12Resource>			mDepthStencilBuff{};
	D3D12_CPU_DESCRIPTOR_HANDLE		mDsvHandle{};

	// frame resource
	static constexpr UINT mFrameResourceCount = 1;
	static constexpr UINT mMaxObjectCount = 1000;
	std::vector<uptr<struct FrameResource>>		mFrameResources;
	FrameResource*								mCurrFrameResource{};		// 현재 프레임의 프레임 리소스
	int											mCurrFrameResourceIndex{};	// 현재 프레임의 프레임 리소스 인덱스

	// command
	ComPtr<ID3D12CommandAllocator>		mCmdAllocator{};
	ComPtr<ID3D12CommandQueue>			mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList>	mCmdList{};

	// fence
	// fence array -> single object
	ComPtr<ID3D12Fence>						mFence{};
	UINT32									mFenceValues{};
	HANDLE									mFenceEvent{};

	// others
	sptr<PostProcessingShader>	mPostProcessingShader{};

	DrawOption mDrawOption{};

protected:
#pragma region C/Dtor
	DXGIMgr();
	virtual ~DXGIMgr() = default;
#pragma endregion

public:
#pragma region Getter
	HWND GetHwnd() const									{ return mWnd; }
	RComPtr<ID3D12Device> GetDevice() const					{ return mDevice; }
	RComPtr<ID3D12GraphicsCommandList> GetCmdList() const	{ return mCmdList; }
	const auto& GetRtvFormats() const						{ return mRtvFormats; }
	UINT GetCbvSrvDescriptorIncSize() const					{ return mCbvSrvDescriptorIncSize; }
	UINT GetRtvDescriptorIncSize() const					{ return mRtvDescriptorIncSize; }
#pragma endregion

#pragma region Setter
	void SetDrawOption(DrawOption option) { mDrawOption = option; }
#pragma endregion

public:
	void Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	// 강제종료
	void Terminate();

	// update dxgi
	void Update();

	// render scene
	void Render();

	// full screen on/off
	void ToggleFullScreen();

	void ClearDepth();
	void ClearStencil();
	void ClearDepthStencil();

private:
	// reset command 
	// rename StartCommand -> RenderBegin
	void RenderBegin();

	// close command
	// rename StopCommand -> RenderEnd
	void RenderEnd();

	// for CreateDirect3DDevice
	void CreateFactory();
	void CreateDevice();
	void SetMSAA();
	void CreateFence();
	void SetIncrementSize();

	void CreateDirect3DDevice();
	void CreateCmdQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();

	// frame resource를 생성한다.
	void CreateFrameResources();

	void CreateSwapChain();
	// swap chain의 RTV들을 생성한다.
	void CreateSwapChainRTVs();
	void CreateDSV();

	void CreatePostProcessingShader();
	void CreatePostProcessingRTVs();

	// full screen on/off (resize swap chain buffer)
	void ChangeSwapChainState();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void BuildScene();
};
#pragma endregion
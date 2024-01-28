#pragma once

#pragma region Define
#define dxgi DXGIMgr::Inst()
#define device dxgi->GetDevice()
#define cmdList dxgi->GetCmdList()
#define frmResMgr dxgi->GetFrameResourceMgr()
#define mrt dxgi->GetMRT()
#pragma endregion

#pragma region ClassForwardDecl
class PostProcessingShader;
class FrameResourceMgr;
class MultipleRenderTarget;
class BlurFilter;

struct PassConstants;
#pragma endregion

#pragma region EnumClass
enum class DrawOption {
	Main = 0,
	Texture,
	Normal,
	Depth,
};

enum class FilterOption {
	None = 0,
	Blur,
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
	static constexpr UINT mRtvCnt			= MRTCount;
	static constexpr std::array<DXGI_FORMAT, mRtvCnt>			mRtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };	// formats of multi render target
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, mSwapChainBuffCnt>	mRtvHandles{};

	std::array<ComPtr<ID3D12Resource>, mSwapChainBuffCnt>	mSwapChainBuffers{};
	UINT													mSwapChainBuffCurrIdx{};	// current swap chain buffer index

	// view (descriptor)
	UINT mCbvSrvUavDescriptorIncSize{};
	UINT mRtvDescriptorIncSize{};
	ComPtr<ID3D12DescriptorHeap>	mRtvHeap{};
	ComPtr<ID3D12DescriptorHeap>	mDsvHeap{};
	ComPtr<ID3D12Resource>			mDepthStencilBuff{};
	D3D12_CPU_DESCRIPTOR_HANDLE		mDsvHandle{};

	// command
	ComPtr<ID3D12CommandAllocator>		mCmdAllocator{};
	ComPtr<ID3D12CommandQueue>			mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList>	mCmdList{};

	// fence
	// fence array -> single object
	ComPtr<ID3D12Fence>						mFence{};
	UINT32									mFenceValues{};
	HANDLE									mFenceEvent{};

	// frameResource
	uptr<FrameResourceMgr>		mFrameResourceMgr;

	// MRT
	sptr<MultipleRenderTarget>	mMRT{};

	// filter
	FilterOption		mFilterOption{};
	uptr<BlurFilter>	mBlurFilter;

	DrawOption		mDrawOption{};

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
	UINT GetCbvSrvUavDescriptorIncSize() const				{ return mCbvSrvUavDescriptorIncSize; }
	UINT GetRtvDescriptorIncSize() const					{ return mRtvDescriptorIncSize; }
	FrameResourceMgr* GetFrameResourceMgr() const			{ return mFrameResourceMgr.get(); }
	rsptr<MultipleRenderTarget> GetMRT() const				{ return mMRT; }
#pragma endregion

#pragma region Setter
	void SetDrawOption(DrawOption option) { mDrawOption = option; }
	void SetMRTTsPassConstants(PassConstants& passConstants);
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

	void CreateSwapChain();
	// swap chain의 RTV들을 생성한다.
	void CreateSwapChainRTVs();
	void CreateDSV();
	void CreateMRT();

	void CreateFrameResources();

	// full screen on/off (resize swap chain buffer)
	void ChangeSwapChainState();

	void CreateFilter();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void BuildScene();

};
#pragma endregion
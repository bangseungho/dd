#include "stdafx.h"
#include "DescriptorHeap.h"
#include "DXGIMgr.h"


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUCbvLastHandle() const
{
	return { mCbvHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUCbvLastHandle() const
{
	return { mCbvHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUSrvLastHandle() const
{
	return { mSrvHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUSrvLastHandle() const
{
	return { mSrvHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUUavLastHandle() const
{
	return { mUavHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUUavLastHandle() const
{
	return { mUavHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSkyBoxCPUSrvLastHandle() const
{
	return { mSkyBoxSrvHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSkyBoxGPUSrvLastHandle() const
{
	return { mSkyBoxSrvHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
UINT DescriptorHeap::GetGPUCbvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUCbvLastHandle().ptr - mCbvHandle.GpuStart.ptr) / dxgi->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetGPUSrvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUSrvLastHandle().ptr - mSrvHandle.GpuStart.ptr) / dxgi->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetGPUUavLastHandleIndex() const
{
	return 0;
}
UINT DescriptorHeap::GetSkyBoxGPUSrvLastHandleIndex() const
{
	return { static_cast<UINT>((GetSkyBoxGPUSrvLastHandle().ptr - mSkyBoxSrvHandle.GpuStart.ptr) / dxgi->GetCbvSrvUavDescriptorIncSize()) };
}

void DescriptorHeap::Create(int cbvCount, int srvCount, int uavCount, int skyBoxSrvCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = cbvCount + srvCount + uavCount + skyBoxSrvCount; // CBVs + SRVs + UAVs + SkyBoxSRVs
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mHeap));
	AssertHResult(hResult);

	// CBV CPU/GPU Descriptor의 시작 핸들 주소를 받아온다.
	mCbvHandle.CpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
	mCbvHandle.GpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

	// SRV의 CPU/GPU Descriptor의 시작 핸들 주소는 CBV Descriptor의 뒤부터 시작한다.
	mSrvHandle.CpuStart.ptr = mCbvHandle.CpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * cbvCount);
	mSrvHandle.GpuStart.ptr = mCbvHandle.GpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * cbvCount);

	// UAV의 CPU/GPU Descriptor의 시작 핸들 주소는 SRV Descriptor의 뒤부터 시작한다.
	mUavHandle.CpuStart.ptr = mSrvHandle.CpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * srvCount);
	mUavHandle.GpuStart.ptr = mSrvHandle.GpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * srvCount);

	mSkyBoxSrvHandle.CpuStart.ptr = mUavHandle.CpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * uavCount);
	mSkyBoxSrvHandle.GpuStart.ptr = mUavHandle.GpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * uavCount);

	// CPU/GPU의 Next를 Start의 위치로 설정한다.
	mCbvHandle.CpuNext = mCbvHandle.CpuStart;
	mCbvHandle.GpuNext = mCbvHandle.GpuStart;
	mSrvHandle.CpuNext = mSrvHandle.CpuStart;
	mSrvHandle.GpuNext = mSrvHandle.GpuStart;
	mUavHandle.CpuNext = mUavHandle.CpuStart;
	mUavHandle.GpuNext = mUavHandle.GpuStart;
	mSkyBoxSrvHandle.CpuNext = mSkyBoxSrvHandle.CpuStart;
	mSkyBoxSrvHandle.GpuNext = mSkyBoxSrvHandle.GpuStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	// TextureCube일 경우 따로 GPU 핸들에 저장한다.
	if (srvDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE) {
		device->CreateShaderResourceView(resource.Get(), srvDesc, mSkyBoxSrvHandle.CpuNext);
		mSkyBoxSrvHandle.CpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
		mSkyBoxSrvHandle.GpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
		return GetSkyBoxGPUStartSrvHandle();
	}
	else {
		device->CreateShaderResourceView(resource.Get(), srvDesc, mSrvHandle.CpuNext);
		mSrvHandle.CpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
		mSrvHandle.GpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
		return GetGPUSrvLastHandle();
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateUnorderedAccessView(RComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
	// [resource]의 UAV를 [uavDesc]에 따라 생성한다.
	device->CreateUnorderedAccessView(resource.Get(), nullptr, uavDesc, mUavHandle.CpuNext);

	// UAV CPU/GPU의 Next를 다음 위치로 증가시킨다.
	mUavHandle.CpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
	mUavHandle.GpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();

	// 리소스의 UAV의 위치를 반환한다.
	return GetGPUUavLastHandle();
}

void DescriptorHeap::Set()
{
	cmdList->SetDescriptorHeaps(1, mHeap.GetAddressOf());
}
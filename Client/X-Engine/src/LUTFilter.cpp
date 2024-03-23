#include "stdafx.h"
#include "LUTFilter.h"
#include "DXGIMgr.h"

#include "ResourceMgr.h"
#include "DescriptorHeap.h"
#include "Shader.h"
#include "Texture.h"
#include "Timer.h"

LUTFilter::LUTFilter(UINT width, UINT height, DXGI_FORMAT format)
	:
	mWidth(width),
	mHeight(height),
	mFormat(format)
{
}

void LUTFilter::Create()
{
	CreateResources();
	CreateDescriptors();
}

UINT LUTFilter::Execute(rsptr<Texture> input)
{
	res->Get<Shader>("LUT")->Set();

	mElapsedTime += DeltaTime();
	DWORD filterOption = dxgi->GetFilterOption();
	cmdList->SetComputeRoot32BitConstants(0, 1, &mElapsedTime, 0);
	cmdList->SetComputeRoot32BitConstants(0, 1, &filterOption, 1);
	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// LUT �ؽ�ó�� BC7 �������� �����ؾ� ���伥 LUT�� �ִ��� �Ȱ��� ������ ������ �� �ִ�.
	cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::LUT0), res->Get<Texture>("LUT_RGB")->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::LUT1), res->Get<Texture>("LUT_RGB")->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

	UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
	cmdList->Dispatch(numGroupsX, mHeight, 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return mOutput->GetSrvIdx();
}

void LUTFilter::CreateDescriptors()
{
	dxgi->CreateShaderResourceView(mOutput.get());
	dxgi->CreateUnorderedAccessView(mOutput.get());
}

void LUTFilter::CreateResources()
{
	mOutput = std::make_shared<Texture>();
	mOutput->Create(mWidth, mHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
		D3D12_RESOURCE_STATE_COMMON);
}
#include "EnginePch.h"
#include "Object.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "Model.h"
#include "Scene.h"
#include "Component/Collider.h"
#include "ObjectPool.h"
#include "MultipleRenderTarget.h"

#include "Animator.h"
#include "Texture.h"
#include "Component/Camera.h"


#pragma region GameObject
rsptr<Texture> GameObject::GetTexture() const
{
	return mMasterModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	mMasterModel = model;
	mMasterModel->CopyModelHierarchy(this);
	
	sptr<const AnimationLoadInfo> animationInfo = model->GetAnimationInfo();
	if (animationInfo) {
		mIsSkinMesh = true;
		mAnimator = std::make_shared<Animator>(animationInfo, this);
	}

	// �� ��ü�� ���������� [mMergedTransform]�� �����Ѵ� (ĳ��)
	Transform::MergeTransform(mMergedTransform, this);
}

void GameObject::SetModel(const std::string& modelName)
{
	SetModel(RESOURCE<MasterModel>(modelName));
}

void GameObject::Animate()
{
	base::Animate();

	if (mAnimator) {
		mAnimator->Animate();
	}
}

void GameObject::Render()
{
	if (mAnimator) {
		mAnimator->UpdateShaderVariables();
	}
	if (mMasterModel) {
		mMasterModel->Render(this);
	}
}


void GameObject::AttachToGround()
{
	Vec3 pos = GetPosition();
	const float terrainHeight = Scene::I->GetTerrainHeight(pos.x, pos.z);
	pos.y = terrainHeight;

	SetPosition(pos);
}
#pragma endregion





#pragma region GridObject
GridObject::GridObject()
{
}

void GridObject::Awake()
{
	AddComponent<ObjectCollider>();
	base::Awake();

	const auto& collider = GetComponent<ObjectCollider>();
	if (collider) {
		mCollider = collider.get();
	}	
}

void GridObject::Update()
{
	base::Update();

	if (IsActive()) {
		UpdateGrid();
	}
}

void GridObject::OnEnable()
{
	base::OnEnable();

	UpdateGrid();
}

void GridObject::OnDisable()
{
	base::OnDisable();

	Scene::I->RemoveObjectFromGrid(this);
}

void GridObject::OnDestroy()
{
	base::OnDestroy();
	
	Scene::I->RemoveDynamicObject(this);
}

void GridObject::RenderBounds()
{
	Vec4 color{};
	switch (GetTag()) {
	case ObjectTag::Player:
		color = Vec4(0, 1, 0, 1);
		break;
	case ObjectTag::Enemy:
		color = Vec4(1, 0, 0, 1);
		break;
	default:
		color = Vec4(1, 1, 1, 1);
		break;
	}
	if (mIsDrawBounding && mCollider) {
		GetComponent<ObjectCollider>()->Render(color);
	}
}

void GridObject::UpdateGrid()
{
	Scene::I->UpdateObjectGrid(this);
}


void GridObject::ResetCollider()
{
	mCollider = AddComponent<ObjectCollider>().get();
}
#pragma endregion





#pragma region InstObject
InstObject::InstObject(ObjectPool* pool, int id)
	:
	mObjectPool(pool),
	mPoolID(id)
{
	
}

void InstObject::SetUpdateFunc()
{
	switch (GetType()) {
	case ObjectType::DynamicMove:
		mUpdateFunc = [this]() { UpdateDynamic(); };
		break;

	default:
		mUpdateFunc = [this]() { UpdateStatic(); };
		break;
	}
}

void InstObject::PushFunc(int buffIdx, UploadBuffer<InstanceData>* buffer) const
{
	InstanceData instData;
	instData.MtxWorld = GetWorldTransform().Transpose();

	buffer->CopyData(buffIdx, instData);
}

void InstObject::Return()
{
	mObjectPool->Return(this);
}

void InstObject::PushRender()
{
	mObjectPool->PushRender(this);
}
#pragma endregion





#pragma region InstBulletObject
void InstBulletObject::UpdateGrid()
{
	Scene::I->UpdateObjectGrid(this, false);
}
#pragma endregion





#pragma region DynamicEnvironmentObject
UINT DynamicEnvironmentMappingManager::AddObject(Object* object)
{
	if (!mDynamicEnvironmentObjectMap.contains(object)) {
		for (const auto& mrt : mMRTs) {

			bool isContain = false;
			for (const auto& map : mDynamicEnvironmentObjectMap) {
				if (mrt.get() == map.second) {
					isContain = true;
				}
			}

			if (!isContain) {
				mDynamicEnvironmentObjectMap.insert({ object, mrt.get() });
				object->mObjectCB.UseRefract = true;
				object->mObjectCB.DynamicEnvironmentMapIndex = mrt->GetTexture(0)->GetSrvIdx();
				return mrt->GetTexture(0)->GetSrvIdx();
			}
		}
	}

	return -1;
}

void DynamicEnvironmentMappingManager::RemoveObject(Object* object)
{
	mDynamicEnvironmentObjectMap.erase(object);
	object->mObjectCB.UseRefract = false;
	object->mObjectCB.DynamicEnvironmentMapIndex = 0;
}

void DynamicEnvironmentMappingManager::Init()
{
	for (int i = 0; i < mkMaxMRTCount; ++i) {
		// create depth stencil buffer
		std::string dsName = "DynamicEnvironmentDs_" + std::to_string(i);
		sptr<Texture> depthStencilBuffer = ResourceMgr::I->CreateTexture(dsName, 512, 512,
			DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, Vec4{ 1.f });
		DXGIMgr::I->CreateDepthStencilView(depthStencilBuffer.get());

		// create mrt
		std::vector<RenderTarget> rts(6);
		std::string rtName = "DynamicEnvironmentRT_" + std::to_string(i);
		sptr<Texture> dynamicCubeMap = ResourceMgr::I->CreateTexture(rtName, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{}, D3DResource::TextureCube);

		for (auto& rt : rts) {
			rt.Target = dynamicCubeMap;
		}

		mMRTs[i] = std::make_shared<MultipleRenderTarget>();
		mMRTs[i]->Create(GroupType::DynamicEnvironment, std::move(rts), depthStencilBuffer);
	}

	BuildCubeFaceCamera();
}

void DynamicEnvironmentMappingManager::UpdatePassCB(sptr<Camera> camera, UINT index)
{
	Matrix proj = camera->GetProjMtx();
	PassConstants passCB;
	passCB.MtxView = camera->GetViewMtx().Transpose();
	passCB.MtxProj = camera->GetProjMtx().Transpose();
	passCB.MtxInvProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	passCB.MtxNoLagView = camera->GetNoLagViewtx().Transpose();
	passCB.CameraPos = camera->GetPosition();
	passCB.CameraRight = camera->GetRight();
	passCB.CameraUp = camera->GetUp();
	passCB.FrameBufferWidth = DXGIMgr::I->GetWindowWidth();
	passCB.FrameBufferHeight = DXGIMgr::I->GetWindowHeight();
	passCB.GlobalAmbient = Vec4(0.4f, 0.4f, 0.4f, 1.f);
	passCB.FilterOption = DXGIMgr::I->GetFilterOption();
	passCB.SkyBoxIndex = 1;

	FRAME_RESOURCE_MGR->CopyData(2 + index, passCB);
}

void DynamicEnvironmentMappingManager::Render(const std::set<GridObject*>& objects)
{
	int cnt{};
	for (auto& [object, mrt] : mDynamicEnvironmentObjectMap) {
		auto& cameras = mCameras.at(cnt++);

		for (int i = 0; i < 6; ++i) {
			mrt->ClearRenderTargetView(i, 1.f);
			mrt->OMSetRenderTargets(1, i);

			cameras[i]->SetPosition(object->GetPosition() + Vec3{0.f, 3.f, 0.f});

			auto& cameraScript = cameras[i]->GetCamera();
			cameraScript->UpdateViewMtx();
			UpdatePassCB(cameraScript, i);

			CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(2 + i));

			Scene::I->RenderDynamicEnvironmentMappingObjects();

			mrt->WaitTargetToResource(i);
		}
	}
}

void DynamicEnvironmentMappingManager::BuildCubeFaceCamera()
{
	const Vec3 center{ 0.f };

	const Vec3 targets[6] =
	{
		Vec3(1.0f, 0.f, 0.f), 
		Vec3(-1.0f, 0.f, 0.f),
		Vec3(0.f, 1.0f, 0.f), 
		Vec3(0.f, -1.0f, 0.f),
		Vec3(0.f, 0.f, 1.0f), 
		Vec3(0.f, 0.f, -1.0f) 
	};

	const Vec3 ups[6] =
	{
		Vec3(0.0f, 1.0f, 0.0f), 
		Vec3(0.0f, 1.0f, 0.0f), 
		Vec3(0.0f, 0.0f, -1.0f),
		Vec3(0.0f, 0.0f, +1.0f),
		Vec3(0.0f, 1.0f, 0.0f),	
		Vec3(0.0f, 1.0f, 0.0f)	
	};

	for (int i = 0; i < mkMaxMRTCount; ++i) {
		auto& cameras = mCameras[i];
		
		for (int i = 0; i < 6; ++i) {
			cameras[i] = std::make_shared<CameraObject>();
			cameras[i]->Awake();

			cameras[i]->LookAt(targets[i], ups[i]);
			auto& cameraScript = cameras[i]->GetCamera();
			cameraScript->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		}
	}
}
#pragma endregion



#pragma region Include
#include "stdafx.h"
#include "Scene.h"
#include "DXGIMgr.h"
#include "MultipleRenderTarget.h"
#include "FrameResource.h"

#include "ResourceMgr.h"
#include "UI.h"
#include "Object.h"
#include "Model.h"
#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Timer.h"
#include "FileIO.h"
#include "Light.h"
#include "Collider.h"
#include "SkyBox.h"
#include "Texture.h"
#include "DescriptorHeap.h"
#include "ObjectPool.h"

#include "Animator.h"
#include "AnimatorController.h"

#include "Script_Player.h"
#include "Script_ExplosiveObject.h"
#include "Script_Billboard.h"
#include "Script_Sprite.h"

#include "TestCube.h"
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region C/Dtor
namespace {
	constexpr int kGridWidthCount = 20;						// all grid count = n*n
	constexpr Vec3 kBorderPos = Vec3(256, 200, 256);		// center of border
	constexpr Vec3 kBorderExtents = Vec3(1500, 500, 1500);		// extents of border

	constexpr D3D12_PRIMITIVE_TOPOLOGY kObjectPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kUIPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kTerrainPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kBoundsPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
}

Scene::Scene()
	:
	mMapBorder(kBorderPos, kBorderExtents),
	mGridhWidth(static_cast<int>(mMapBorder.Extents.x / kGridWidthCount)),
	mLight(std::make_shared<Light>())
{

}

void Scene::Release()
{
	mainCameraObject->Destroy();
	canvas->Release();

	Destroy();
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Getter
float Scene::GetTerrainHeight(float x, float z) const
{
	assert(mTerrain);

	return mTerrain->GetHeight(x, z);
}

rsptr<const MasterModel> Scene::GetModel(const std::string& modelName) const
{
	assert(mModels.contains(modelName));

	return mModels.at(modelName);
}

sptr<AnimatorController> Scene::GetAnimatorController(const std::string& controllerFile) const
{
	return std::make_shared<AnimatorController>(*mAnimatorControllerMap.at(controllerFile));
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DirectX
void Scene::ReleaseUploadBuffers()
{
	ProcessObjects([](sptr<GameObject> object) {
		object->ReleaseUploadBuffers();
		});

	MeshRenderer::ReleaseUploadBuffers();
}

void Scene::UpdateShaderVars()
{
	// TODO : AnimateMaterials(); 
	UpdateMainPassCB();
	UpdateMaterialBuffer();
}

void Scene::UpdateMainPassCB()
{
	static float timeElapsed{};
	timeElapsed += DeltaTime();

	PassConstants passConstants;
	passConstants.MtxView				= XMMatrixTranspose(_MATRIX(mainCamera->GetViewMtx()));
	passConstants.MtxProj				= XMMatrixTranspose(_MATRIX(mainCamera->GetProjMtx()));
	passConstants.EyeW					= mainCamera->GetPosition();
	passConstants.DeltaTime				= timeElapsed;
	passConstants.RT0G_PositionIndex	= dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Position)->GetGpuDescriptorHandleIndex();
	passConstants.RT1G_NormalIndex		= dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Normal)->GetGpuDescriptorHandleIndex();
	passConstants.RT2G_DiffuseIndex		= dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Diffuse)->GetGpuDescriptorHandleIndex();
	passConstants.RT3G_EmissiveIndex	= dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Emissive)->GetGpuDescriptorHandleIndex();
	passConstants.RT4G_DistanceIndex	= dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Distance)->GetGpuDescriptorHandleIndex();
	passConstants.RT0L_DiffuseIndex		= dxgi->GetMRT(GroupType::Lighting)->GetTexture(Lighting::Diffuse)->GetGpuDescriptorHandleIndex();
	passConstants.RT1L_SpecularIndex	= dxgi->GetMRT(GroupType::Lighting)->GetTexture(Lighting::Specular)->GetGpuDescriptorHandleIndex();
	passConstants.RT2L_AmbientIndex		= dxgi->GetMRT(GroupType::Lighting)->GetTexture(Lighting::Ambient)->GetGpuDescriptorHandleIndex();
	passConstants.LightCount			= mLight->GetLightCount();
	passConstants.GlobalAmbient			= Vec4(0.05f, 0.05f, 0.05f, 1.f);
	passConstants.FilterOption			= dxgi->GetFilterOption();
	memcpy(&passConstants.Lights, mLight->GetSceneLights().get(), sizeof(passConstants.Lights));
	XMStoreFloat4(&passConstants.FogColor, Colors::Gray);
	
	frmResMgr->CopyData(passConstants);
}

void Scene::UpdateMaterialBuffer()
{
	// TODO : Update only if the numFramesDirty is greater than 0
	for (auto& model : mModels) {
		model.second->GetMesh()->UpdateMaterialBuffer();
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Build
void Scene::BuildObjects()
{
	// load materials
	res->Init();

	// load canvas (UI)
	canvas->Init();

	// load models
	LoadAnimationClips();
	LoadAnimatorControllers();
	LoadSceneObjects("Import/Scene.bin");
	LoadModels();

	// build settings
	BuildPlayers();
	BuildTerrain();
	BuildTestCube();

	// build 
 	BuildShaders();
	// build static meshes
	MeshRenderer::BuildMeshes();

	// skybox
	mSkyBox = std::make_shared<SkyBox>();
}

void Scene::ReleaseObjects()
{
	MeshRenderer::Release();
}


void Scene::BuildShaders()
{
	BuildDeferredShader();
	BuildForwardShader();
}

void Scene::BuildForwardShader()
{
	// 현재 조명 렌더 타겟을 따로 두지 않았기 때문에 foward가 offscreen으로 대체 되었다.
	// 추후에 조명 렌더 타겟 구현 이후 forward를 R8G8B8A8의 쉐이더로 변경해야 한다.
	mWaterShader = std::make_shared<WaterShader>();
	mWaterShader->Create(ShaderType::OffScreen);

	mBillboardShader = std::make_shared<BillboardShader>();
	mBillboardShader->Create(ShaderType::OffScreen);

	mSpriteShader = std::make_shared<SpriteShader>();
	mSpriteShader->Create(ShaderType::OffScreen);

	mFinalShader = std::make_shared<FinalShader>();
	mFinalShader->Create(ShaderType::OffScreen);

	mBoundingShader = std::make_shared<WireShader>();
	mBoundingShader->Create(ShaderType::Forward);

	mOffScreenShader = std::make_shared<OffScreenShader>();
	mOffScreenShader->Create(ShaderType::Forward);

	mLightingShader = std::make_shared<LightingShader>();
	mLightingShader->Create(ShaderType::Lighting);
}

void Scene::BuildDeferredShader()
{
	ShaderType shaderType = ShaderType::Deferred;

	mGlobalShader = std::make_shared<DeferredShader>();
	mGlobalShader->Create(shaderType);

	mInstShader = std::make_shared<ObjectInstShader>();
	mInstShader->Create(shaderType);

	mTransparentShader = std::make_shared<TransparentShader>();
	mTransparentShader->Create(shaderType);

	mBulletShader = std::make_shared<ColorInstShader>();
	mBulletShader->Create(shaderType);

	mSkinnedMeshShader = std::make_shared<SkinMeshShader>();
	mSkinnedMeshShader->Create(shaderType);
}

void Scene::BuildPlayers()
{
	mPlayers.reserve(1);
	sptr<GridObject> airplanePlayer = std::make_shared<GridObject>();
	airplanePlayer->AddComponent<Script_GroundPlayer>()->CreateBullets(GetModel("tank_bullet"));
	//airplanePlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(GetModel("EliteTrooper"));

	mPlayers.push_back(airplanePlayer);

	mPlayer = mPlayers.front();
}

void Scene::BuildTerrain()
{
	mTerrain = std::make_shared<Terrain>("Import/Terrain.bin");

	BuildGrid();
}

void Scene::BuildTestCube()
{
	mTestCubes.resize(2);
	mTestCubes[0] = std::make_shared<TestCube>(Vec2(190, 150));
	mTestCubes[0]->GetMaterial()->SetMatallic(0.f);
	mTestCubes[0]->GetMaterial()->SetRoughness(0.f);
	mTestCubes[0]->GetMaterial()->SetTexture(TextureMap::DiffuseMap0, res->Get<Texture>("Rock_BaseColor"));
	mTestCubes[0]->GetMaterial()->SetTexture(TextureMap::NormalMap, res->Get<Texture>("Rock_Normal"));

	mTestCubes[1] = std::make_shared<TestCube>(Vec2(165, 150));
	mTestCubes[1]->GetMaterial()->SetMatallic(0.f);
	mTestCubes[1]->GetMaterial()->SetRoughness(0.f);
	mTestCubes[1]->GetMaterial()->SetTexture(TextureMap::DiffuseMap0, res->Get<Texture>("Wall_BaseColor"));
	mTestCubes[1]->GetMaterial()->SetTexture(TextureMap::NormalMap, res->Get<Texture>("Wall_Normal"));
}

void Scene::BuildGrid()
{
	constexpr float kMaxHeight = 300.f;	// for 3D grid

	// recalculate scene grid size
	const int adjusted = Math::GetNearestMultiple((int)mMapBorder.Extents.x, mGridhWidth);
	mMapBorder.Extents = Vec3((float)adjusted, mMapBorder.Extents.y, (float)adjusted);

	// set grid start pos
	mGridStartPoint = mMapBorder.Center.x - mMapBorder.Extents.x / 2;

	// set grid count
	mGridCols = adjusted / mGridhWidth;
	const int gridCount = mGridCols * mGridCols;
	mGrids.resize(gridCount);

	// set grid bounds
	const float gridExtent = (float)mGridhWidth / 2.0f;
	for (int y = 0; y < mGridCols; ++y) {
		for (int x = 0; x < mGridCols; ++x) {
			float gridX = (mGridhWidth * x) + ((float)mGridhWidth / 2) + mGridStartPoint;
			float gridZ = (mGridhWidth * y) + ((float)mGridhWidth / 2) + mGridStartPoint;

			BoundingBox bb{};
			bb.Center = Vec3(gridX, kMaxHeight / 2, gridZ);
			bb.Extents = Vec3(gridExtent, kMaxHeight, gridExtent);

			int index = (y * mGridCols) + x;
			mGrids[index].Init(index, bb);
		}
	}
}

void Scene::UpdateGridInfo()
{
	ProcessObjects([this](sptr<GridObject> object) {
		UpdateObjectGrid(object.get());
		});

	mTerrain->UpdateGrid();
}


void Scene::LoadSceneObjects(const std::string& fileName)
{
	FILE* file = nullptr;
	::fopen_s(&file, fileName.c_str(), "rb");
	assert(file);
	::rewind(file);

	mLight->BuildLights(file);
	LoadGameObjects(file);
}

void Scene::LoadGameObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	int objectCount;
	FileIO::ReadString(file, token); // "<GameObjects>:"
	FileIO::ReadVal(file, objectCount);

	mStaticObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<MasterModel> model{};
	sptr<ObjectPool> objectPool{};

	bool isInstancing{};
	ObjectTag tag{};
	ObjectLayer layer{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GridObject> object{};

		if (sameObjectCount <= 0) {
			FileIO::ReadString(file, token); //"<Tag>:"
			FileIO::ReadString(file, token);
			tag = GetTagByString(token);

			int layerNum{};
			FileIO::ReadString(file, token); //"<Layer>:"
			FileIO::ReadVal(file, layerNum);
			layer = GetLayerByNum(layerNum);

			FileIO::ReadString(file, token); //"<FileName>:"

			std::string meshName{};
			FileIO::ReadString(file, meshName);

			model = FileIO::LoadGeometryFromFile("Import/Meshes/" + meshName + ".bin");
			mModels.insert(std::make_pair(meshName, model));

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);

			if (isInstancing) {
				objectPool = std::make_shared<ObjectPool>(model, sameObjectCount, sizeof(SB_StandardInst));
				objectPool->CreateObjects<InstObject>();
				mObjectPools.emplace_back(objectPool);
			}
		}

		if (isInstancing) {
			// 인스턴싱 객체는 생성된 객체를 받아온다.
			object = objectPool->Get(false);
		}
		else {
			object = std::make_shared<GridObject>();
		}

		InitObjectByTag(&tag, object);

		object->SetLayer(layer);
		if (layer == ObjectLayer::Water) {
			mEnvironments.pop_back();
			mWater = object;
		}

		object->SetModel(model);

		Vec4x4 transform;
		FileIO::ReadVal(file, transform);
		object->SetWorldTransform(transform);

		--sameObjectCount;
	}
}

void Scene::LoadModels()
{
	const std::vector<std::string> binModelNames = { "tank_bullet", "sprite_explosion", };

	sptr<MasterModel> model;
	for (auto& name : binModelNames) {
		if (!mModels.contains(name)) {
			model = FileIO::LoadGeometryFromFile("Import/Meshes/" + name + ".bin");
			if (name.substr(0, 6) == "sprite") {
				model->SetSprite();
			}
			mModels.insert(std::make_pair(name, model));
		}
	}

	
}

void Scene::LoadAnimationClips()
{
	const std::string rootFolder = "Import/AnimationClips/";
	for (const auto& clipFolder : std::filesystem::directory_iterator(rootFolder)) {
		std::string clipFolderName = clipFolder.path().filename().string();

		for (const auto& file : std::filesystem::directory_iterator(rootFolder + clipFolderName + '/')) {
			std::string fileName = file.path().filename().string();
			sptr<const AnimationClip> clip = FileIO::LoadAnimationClip(clipFolder.path().string() + '/' + fileName);

			FileIO::RemoveExtension(fileName);
			mAnimationClipMap[clipFolderName].insert(std::make_pair(fileName, clip));
		}
	}
}

void Scene::LoadAnimatorControllers()
{
	const std::string rootFolder = "Import/AnimatorControllers/";
	for (const auto& file : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = file.path().filename().string();
		mAnimatorControllerMap.insert(std::make_pair(FileIO::RemoveExtension(fileName), FileIO::LoadAnimatorController(rootFolder + fileName)));
	}
}

void Scene::InitObjectByTag(const void* pTag, sptr<GridObject> object)
{
	ObjectTag tag = *(ObjectTag*)pTag;
	object->SetTag(tag);

	switch (tag) {
	case ObjectTag::Unspecified:
		break;
	case ObjectTag::ExplosiveSmall:
	{
		mExplosiveObjects.push_back(object);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateSmallExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Tank:
	case ObjectTag::Helicopter:
	case ObjectTag::ExplosiveBig:
	{
		mExplosiveObjects.push_back(object);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateBigExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Environment:
	{
		mEnvironments.push_back(object);
		return;
	}

	break;
	case ObjectTag::Billboard:
	{
		object->AddComponent<Script_Billboard>();
	}

	break;
	case ObjectTag::Sprite:
	{
		object->AddComponent<Script_Sprite>();
		return;
	}

	break;
	default:
		break;
	}

	mStaticObjects.emplace_back(object);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Render
namespace {
	bool IsBehind(const Vec3& point, const Vec4& plane)
	{
		return XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&plane), _VECTOR(point))) < 0.f;
	}
}

void Scene::OnPrepareRender()
{
	// 환경 매핑을 해야 하기 때문에 렌더 전 스카이 박스 텍스처를 설정해야 한다.
	mSkyBox->SetGraphicsRootDescriptorTable();
}

void Scene::RenderShadow()
{

}

void Scene::RenderDeferred()
{
#pragma region PrepareRender
	OnPrepareRender();
	mRenderedObjects.clear();
	mTransparentObjects.clear();
	mBillboardObjects.clear();
#pragma endregion

	mGlobalShader->Set();

	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);
	RenderGridObjects();	
	RenderSkinMeshObjects();
	RenderEnvironments();	
	RenderInstanceObjects();
	RenderBullets();
	RenderTestCubes();

	cmdList->IASetPrimitiveTopology(kTerrainPrimitiveTopology);
	RenderTerrain();
}

void Scene::RenderLights()
{
	cmdList->IASetPrimitiveTopology(kUIPrimitiveTopology);
	mLightingShader->Set();

	if (mLight) {
		mLight->Render();
	}
}

void Scene::RenderFinal()
{
	// 조명에서 출력한 diffuse와 specular를 결합하여 최종 색상을 렌더링한다.
	mFinalShader->Set();

	cmdList->IASetPrimitiveTopology(kUIPrimitiveTopology);
	cmdList->DrawInstanced(6, 1, 0, 0);
}

void Scene::RenderForward()
{
	RenderFXObjects(); 
	RenderBillboards();

	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);
	RenderTransparentObjects(mTransparentObjects); 
	RenderSkyBox();
}

void Scene::RenderUI()
{
	if (RenderBounds(mRenderedObjects)) {
		cmdList->IASetPrimitiveTopology(kUIPrimitiveTopology);
	}

	canvas->Render();
}

void Scene::RenderPostProcessing(int offScreenIndex)
{
	// 포스트 프로세싱에 필요한 상수 버퍼 뷰 설정
	PostPassConstants passConstants;
	passConstants.RT0_OffScreenIndex = offScreenIndex;
	frmResMgr->CopyData(passConstants);
	cmdList->SetGraphicsRootConstantBufferView(dxgi->GetGraphicsRootParamIndex(RootParam::PostPass), frmResMgr->GetPostPassCBGpuAddr());

	// 쉐이더 설정
	mOffScreenShader->Set();

	// 렌더링
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
}

void Scene::RenderTerrain()
{
	if (mTerrain) {
		mTerrain->Render();
	}
}

void Scene::RenderTransparentObjects(const std::set<GridObject*>& transparentObjects)
{
	mTransparentShader->Set();
	for (auto& object : transparentObjects) {
		object->Render();
	}
	if (mWater) {
		mWaterShader->Set();
		mWater->Render();
	}
}

void Scene::RenderSkyBox()
{
	mSkyBox->Render();
}

void Scene::RenderGridObjects()
{
	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

		// TODO : 현재 인스턴싱 쪽에서 깜빡거리는 버그 발견 
		if (mainCamera->IsInFrustum(grid.GetBB())) {
			auto& objects = grid.GetObjects();
			mRenderedObjects.insert(objects.begin(), objects.end());
		}
	}

	for (auto& object : mRenderedObjects) {
		if (object->IsTransparent()) {
			mTransparentObjects.insert(object);
			continue;
		}

		switch (object->GetTag())
		{
		case ObjectTag::Billboard:
		case ObjectTag::Sprite:
			mBillboardObjects.insert(object);
			break;
		default:
			if (object->IsSkinMesh()) {
				mSkinMeshObjects.insert(object);
				break;
			}
			object->Render();
			break;
		}
	}
}

void Scene::RenderSkinMeshObjects()
{
	mSkinnedMeshShader->Set();
	for (auto& object : mSkinMeshObjects) {
		object->Render();
	}
}

void Scene::RenderInstanceObjects()
{
	mInstShader->Set();
	for (auto& buffer : mObjectPools) {
		buffer->Render();
	}
}

void Scene::RenderFXObjects()
{

}

void Scene::RenderTestCubes()
{
	for (const auto& testCube : mTestCubes) {
		testCube->Render();
	}
}

void Scene::RenderEnvironments()
{
	for (auto& env : mEnvironments) {
		env->Render();
	}
}

void Scene::RenderBullets()
{
	mBulletShader->Set();
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			//player->GetComponent<Script_AirplanePlayer>()->RenderBullets();
		}
	}
}

bool Scene::RenderBounds(const std::set<GridObject*>& renderedObjects)
{
	if (!mIsRenderBounds || !mBoundingShader) {
		return false;
	}

	cmdList->IASetPrimitiveTopology(kBoundsPrimitiveTopology);

	mBoundingShader->Set();
	RenderObjectBounds(renderedObjects);
	RenderGridBounds();

	return true;
}

void Scene::RenderObjectBounds(const std::set<GridObject*>& renderedObjects)
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->RenderBounds();
		}
	}

	for (auto& object : renderedObjects) {
		object->RenderBounds();
	}
}

//#define DRAW_SCENE_GRID_3D
void Scene::RenderGridBounds()
{
	for (const auto& grid : mGrids) {
#ifdef DRAW_SCENE_GRID_3D
		MeshRenderer::Render(grid.GetBB());
#else
		constexpr float kGirdHeight = 30.f;
		Vec3 pos = grid.GetBB().Center;
		pos.y = kGirdHeight;
		MeshRenderer::RenderPlane(pos, (float)mGridhWidth, (float)mGridhWidth);
#endif
	}
}

void Scene::RenderBillboards()
{
	mBillboardShader->Set();
	for (auto& object : mBillboardObjects) {
		object->Render();
	}
	mSpriteShader->Set();
	for (auto& object : mSpriteEffectObjects) {
		object->Render();
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void Scene::Start()
{
	/* Awake */
	mainCameraObject->Awake();
	mTerrain->Awake();
	ProcessObjects([](sptr<GameObject> object) {
		object->Awake();
		});

	/* Enable & Start */
	mTerrain->OnEnable();
	ProcessObjects([](sptr<GameObject> object) {
		object->OnEnable();
		});
	mainCameraObject->OnEnable();

	UpdateGridInfo();
}

void Scene::Update()
{
	CheckCollisions();

	UpdateObjects();
	UpdateLights();
	UpdateCamera();
	canvas->Update();

	Animate();

	UpdateShaderVars();
}

void Scene::Animate()
{
	ProcessObjects([this](sptr<GridObject> object) {
		object->Animate();
		});

	UpdateFXObjects();
	UpdateSprites();
}

void Scene::CheckCollisions()
{
	UpdatePlayerGrid();

	for (Grid& grid : mGrids) {
		grid.CheckCollisions();
	}

	DeleteExplodedObjects();
}

void Scene::UpdatePlayerGrid()
{
	for (auto& player : mPlayers) {
		UpdateObjectGrid(player.get());
	}
}

void Scene::UpdateObjects()
{
	ProcessObjects([this](sptr<GridObject> object) {
		object->Update();
		});

	ProcessObjects([this](sptr<GridObject> object) {
		object->Animate();
		});
}

void Scene::UpdateFXObjects()
{

}

void Scene::UpdateSprites()
{
	for (auto it = mSpriteEffectObjects.begin(); it != mSpriteEffectObjects.end(); ) {
		auto& object = *it;
		if (object->GetComponent<Script_Sprite>()->IsEndAnimation()) {
			object->OnDestroy();
			it = mSpriteEffectObjects.erase(it);
		}
		else {
			object->Update();
			++it;
		}
	}
}

void Scene::UpdateLights()
{
	// update dynamic lights here.
}

void Scene::UpdateCamera()
{
	mainCameraObject->Update();
}
#pragma endregion





//////////////////* Others *//////////////////
void Scene::CreateSpriteEffect(Vec3 pos, float speed, float scale)
{
	sptr<GameObject> effect = std::make_shared<GameObject>();
	effect->SetModel(GetModel("sprite_explosion"));
	effect->RemoveComponent<ObjectCollider>();
	const auto& script = effect->AddComponent<Script_Sprite>();
	script->SetSpeed(speed);
	script->SetScale(scale);

	pos.y += 2;	// 보정
	effect->SetPosition(pos);

	effect->OnEnable();
	mSpriteEffectObjects.emplace_back(effect);
}

void Scene::CreateSmallExpFX(const Vec3& pos)
{
	CreateSpriteEffect(pos, 0.0001f);
}

void Scene::CreateBigExpFX(const Vec3& pos)
{
	CreateSpriteEffect(pos, 0.025f, 5.f);
}

int Scene::GetGridIndexFromPos(Vec3 pos) const
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	const int gridX = static_cast<int>(pos.x / mGridhWidth);
	const int gridZ = static_cast<int>(pos.z / mGridhWidth);

	return gridZ * mGridCols + gridX;
}

void Scene::DeleteExplodedObjects()
{
	for (auto it = mExplosiveObjects.begin(); it != mExplosiveObjects.end(); ) {
		auto& object = *it;
		if (object->GetComponent<Script_ExplosiveObject>()->IsExploded()) {
			// remove objects in grid
			for (int index : object->GetGridIndices()) {
				mGrids[index].RemoveObject(object.get());
			}

			object->OnDestroy();
			it = mExplosiveObjects.erase(it);
		}
		else {
			++it;
		}
	}
}

void Scene::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{

}

void Scene::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
			timer->Stop();
			break;
		case VK_END:
			timer->Start();
			break;
		case '0':
			scene->BlowAllExplosiveObjects();
			break;

		case VK_OEM_6:
			ChangeToNextPlayer();
			break;
		case VK_OEM_4:
			ChangeToPrevPlayer();
			break;

		case VK_F5:
			ToggleDrawBoundings();
			break;
		default:
			break;
		}
	}

	break;
	default:
		break;
	}
}

void Scene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessObjects([](sptr<GridObject> object) {
		object->ToggleDrawBoundings();
		});
}

void Scene::CreateFX(FXType type, const Vec3& pos)
{
	switch (type) {
	case FXType::SmallExplosion:
		CreateSmallExpFX(pos);
		break;
	case FXType::BigExplosion:
		CreateBigExpFX(pos);
		break;
	default:
		assert(0);
		break;
	}
}

void Scene::BlowAllExplosiveObjects()
{
	for (auto& object : mExplosiveObjects)
	{
		object->GetComponent<Script_ExplosiveObject>()->Explode();
		for (int index : object->GetGridIndices()) {
			mGrids[index].RemoveObject(object.get());
		}
		object->OnDestroy();
	}
	mExplosiveObjects.clear();
}

void Scene::ChangeToNextPlayer()
{
	++mCurrPlayerIndex;
	if (mCurrPlayerIndex >= mPlayers.size()) {
		mCurrPlayerIndex = 0;
	}

	mPlayer = mPlayers[mCurrPlayerIndex];
}

void Scene::ChangeToPrevPlayer()
{
	--mCurrPlayerIndex;
	if (mCurrPlayerIndex < 0) {
		mCurrPlayerIndex = static_cast<int>(mPlayers.size() - 1);
	}

	mPlayer = mPlayers[mCurrPlayerIndex];
}

void Scene::UpdateObjectGrid(GridObject* object, bool isCheckAdj)
{
	const int gridIndex = GetGridIndexFromPos(object->GetPosition());

	if (IsGridOutOfRange(gridIndex)) {
		RemoveObjectFromGrid(object);

		object->SetGridIndex(-1);
		return;
	}

	// remove object from current grid if move to another grid
	if (gridIndex != object->GetGridIndex()) {
		RemoveObjectFromGrid(object);
	}


	// ObjectCollider가 활성화된 경우
	// 1칸 이내의 "인접 그리드(8개)와 충돌검사"
	const auto& collider = object->GetCollider();
	if(collider) {
		std::unordered_set<int> gridIndices{ gridIndex };
		const auto& objectBS = collider->GetBS();

		// BoundingSphere가 Grid 내부에 완전히 포함되면 "인접 그리드 충돌검사" X
		if (isCheckAdj && mGrids[gridIndex].GetBB().Contains(objectBS) != ContainmentType::CONTAINS) {
			const int gridX = gridIndex % mGridCols;
			const int gridZ = gridIndex / mGridCols;

			for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
				for (int offsetX = -1; offsetX <= 1; ++offsetX) {
					const int neighborX = gridX + offsetX;
					const int neighborZ = gridZ + offsetZ;

					// 인덱스가 전체 그리드 범위 내에 있는지 확인
					if (neighborX >= 0 && neighborX < mGridCols && neighborZ >= 0 && neighborZ < mGridCols) {
						const int neighborIndex = neighborZ * mGridCols + neighborX;

						if (neighborIndex == gridIndex) {
							continue;
						}

						if (mGrids[neighborIndex].GetBB().Intersects(objectBS)) {
							mGrids[neighborIndex].AddObject(object);
							gridIndices.insert(neighborIndex);
						}
						else {
							mGrids[neighborIndex].RemoveObject(object);
						}
					}
				}
			}

			object->SetGridIndices(gridIndices);
		}
	}

	object->SetGridIndex(gridIndex);
	mGrids[gridIndex].AddObject(object);
}

void Scene::RemoveObjectFromGrid(GridObject* object)
{
	for (const int gridIndex : object->GetGridIndices()) {
		if (!IsGridOutOfRange(gridIndex)) {
			mGrids[gridIndex].RemoveObject(object);
		}
	}

	object->ClearGridIndices();
}

void Scene::ProcessObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& player : mPlayers) {
		processFunc(player);
	}

	for (auto& object : mStaticObjects) {
		processFunc(object);
	}

	for (auto& object : mExplosiveObjects) {
		processFunc(object);
	}
}
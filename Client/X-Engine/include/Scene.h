#pragma once


#pragma region Include
#include "Grid.h"
#include "NetworkEvents.h"
#pragma endregion


#pragma region Define
#define GAME_MGR Scene::I->GetGameManager()
#pragma endregion


#pragma region Using
using namespace Path;
#pragma endregion


#pragma region ClassForwardDecl
class Camera;
class Object;
class GameObject;
class GridObject;
class InstObject;
class Terrain;
class Light;
class SkyBox;
class ObjectPool;
class TestCube;
class MasterModel;
class ObjectTag;
#pragma endregion


#pragma region Class
class Scene : public Singleton<Scene> {
	friend Singleton;

public:
	enum class FXType {
		SmallExplosion = 0,
		BigExplosion
	};

private:
	/* Light */
	sptr<Light> mLight{};

	/* SkyBox */
	sptr<SkyBox> mSkyBox{};

	/* Object */
	sptr<Object>					mGameManager{};
	std::vector<sptr<GameObject>>	mEnvironments{};
	std::vector<sptr<GridObject>>	mStaticObjects{};
	std::vector<sptr<GridObject>>	mDynamicObjects{};
	std::vector<sptr<ObjectPool>>	mObjectPools{};
	std::vector<sptr<GridObject>>	mDynamicObjectBuffer{};		// 추가(Instantiate) 대기 버퍼
	std::set<size_t>				mDestroyObjects{};

	std::set<sptr<GridObject>>	mDissolveObjects{};
	std::set<GridObject*>	    mRenderedObjects{};
	std::set<GridObject*>	    mTransparentObjects{};
	std::set<GridObject*>	    mBillboardObjects{};
	std::set<GridObject*>	    mSkinMeshObjects{};

	/* 장재문 - */
	USE_LOCK;
	struct PlayerUpdateInfo {
		sptr<GridObject> player;
		Vec3			 NewPos;
	};


	Concurrency::concurrent_unordered_map<UINT32, sptr<GridObject>> mOtherPlayers{}; /* sessionID, otherPlayer */
	Concurrency::concurrent_queue<sptr<SceneEvent::EventData>> mEventsProcessQueue{};


	//Concurrency::concurrent_queue<sptr<GridObject>> mEventsQueue_AddOtherPlayer;
	//Concurrency::concurrent_queue<PlayerUpdateInfo> mEventsQueue_MoveOtherPlayer;


	/* TestCube */
	std::vector<sptr<GameObject>>	mParticles{};

	/* Map */
	sptr<Terrain>		mTerrain{};
	BoundingBox			mMapBorder{};			// max scene range	(grid will be generated within this border)

	/* Grid */
	std::vector<Grid>	mGrids{};				// all scene grids
	float				mGridStartPoint{};		// leftmost coord of the entire grid
	int					mGridWidth{};			// length of x for one grid
	int					mGridCols{};			// number of columns in the grid

	/* Others */
	bool mIsRenderBounds = false;

	std::vector<Vec3>	mOpenList{};
	std::vector<Vec3>	mClosedList{};

private:
#pragma region C/Dtor
	Scene();
	virtual ~Scene() = default;

public:
	void Release();

private:
#pragma endregion

public:
#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	std::vector<sptr<GameObject>> GetAllObjects() const;
	rsptr<Object> GetGameManager() const { return mGameManager; }
	std::vector<sptr<GameObject>> GetAllPartilceSystems() const;

	int GetGridIndexFromPos(Vec3 pos) const;

	Pos GetTileUniqueIndexFromPos(const Vec3& pos) const;
	Vec3 GetTilePosFromUniqueIndex(const Pos& index) const;

	Tile GetTileFromUniqueIndex(const Pos& index) const;
	void SetTileFromUniqueIndex(const Pos& index, Tile tile);
	Tile GetTileFromPos(const Vec3& index) const;

	std::vector<Vec3>& GetOpenList() { return mOpenList; }
	std::vector<Vec3>& GetClosedList() { return mClosedList; }

#pragma endregion

#pragma region DirectX
public:
	void ReleaseUploadBuffers();

private:
	void UpdateShaderVars();
	void UpdateMainPassCB();
	void UpdateShadowPassCB();
	void UpdateSsaoCB();
	void UpdateMaterialBuffer();
#pragma endregion

#pragma region Build
public:
	void BuildObjects();
	void ReleaseObjects();

private:
	/* Object */
	void BuildTerrain();
	void BuildTest();

	/* Grid */
	// generate grids
	void BuildGrid();
	// update grid indices for all objects
	void UpdateGridInfo();

	/* Load */
	// 씬 파일에서 모든 객체와 조명의 정보를 불러온다.
	void LoadSceneObjects(const std::string& fileName);
	// 씬 파일에서 모든 객체의 정보를 불러온다. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(std::ifstream& file);

	/* Other */
	// 태그별에 따라 객체를 초기화하고 씬 컨테이너에 객체를 삽입한다.(static, explosive, environments, ...)
	void InitObjectByTag(ObjectTag tag, sptr<GridObject> object);

#pragma endregion

#pragma region Render
public:
	// render scene
	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();
	void RenderForward();
	void RenderPostProcessing(int offScreenIndex);
	void RenderUI();

private:
	// 카메라에 보이는 grid만 렌더링한다.
	// 투명, 빌보드 객체는 별도의 Shader를 사용해 렌더링해야 하므로 렌더링하지 않고 그 집합을 반환한다.
	// [renderedObjects]    : 렌더링된 모든 객체 (그리드에 포함된)
	// [transparentObjects] : 투명 객체
	// [billboardObjects]	: 빌보드 객체 (plane)
	void RenderGridObjects(bool isShadowed = false);
	void RenderSkinMeshObjects(bool isShadowed = false);
	void RenderEnvironments();
	void RenderInstanceObjects();

	void RenderTerrain();

	// render [transparentObjects]
	void RenderTransparentObjects(const std::set<GridObject*>& transparentObjects);
	void RenderDissolveObjects();
	void RenderSkyBox();
	void RenderParticles();

	// [renderedObjects]와 grid의 bounds를 rendering한다.
	bool RenderBounds(const std::set<GridObject*>& renderedObjects);
	void RenderObjectBounds(const std::set<GridObject*>& renderedObjects);
	void RenderGridBounds();
#pragma endregion


#pragma region Update
public:
	void Start();
	void Update();

private:
	void CheckCollisions();

	// update all objects
	void UpdateObjects();

#pragma endregion

public:
	float CheckCollisionsRay(int gridIndex, const Ray& ray) const;
	void ToggleDrawBoundings();
	void ToggleFilterOptions();

	// update objects' grid indices
	void UpdateObjectGrid(GridObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GridObject* object);

	// create new game object from model
	sptr<GridObject> Instantiate(const std::string& modelName, bool enable = true);

	void AddDynamicObject(rsptr<GridObject> object) { mDynamicObjects.push_back(object); }
	void RemoveDynamicObject(GridObject* object);

	sptr<ObjectPool> CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);
	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

#pragma region Network
	void ProcessEvents();

	void AddEvent(sptr<SceneEvent::EventData> data);
#pragma endregion

private:
	// do [processFunc] for activated objects
	void ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc);
	// do [processFunc] for all objects
	void ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc);

	void RemoveDesrtoyedObjects();

	// move mObjectBuffer's objects to mDynamicObjects
	void PopObjectBuffer();

	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }
};
#pragma endregion

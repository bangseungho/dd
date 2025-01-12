#pragma once


#pragma region Include
#include "Grid.h"
#include "Scene.h"
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
class BattleScene : public Singleton<BattleScene>, public Scene {
	friend Singleton;
	using base = Scene;

public:
	enum class FXType {
		SmallExplosion = 0,
		BigExplosion
	};

private:
	/* Object */
	std::vector<sptr<GridObject>>	mStaticObjects{};
	std::vector<sptr<GridObject>>	mDynamicObjects{};
	std::vector<sptr<ObjectPool>>	mObjectPools{};
	std::vector<sptr<GridObject>>	mDynamicObjectBuffer{};		// Instantiate ready buffer
	std::vector<sptr<Object>>		mScriptObjects{};			// Objects with scripts in Unity Scene
	sptr<ObjectPool>				mBounds{};					// Manually added bounds
	std::set<size_t>				mDestroyObjects{};

	std::set<sptr<GridObject>>		mDissolveObjects{};
	std::set<GridObject*>			mRenderedObjects{};
	std::set<GridObject*>			mSkinMeshObjects{};
	std::set<GridObject*>			mGridObjects{};
	
	std::map<ObjectTag, std::set<GridObject*>> mObjectsByShader;

	/* Map */
	sptr<Terrain>		mTerrain{};

	/* Grid */
	std::vector<sptr<Grid>>	mGrids{};			// all scene grids
	std::vector<sptr<Grid>>	mSurroundGrids{};	// around player grids

	static constexpr int mkGridXCount = 20;
	static constexpr int mkGridZCount = 10;
	static constexpr int mkMapWidth   = 1000;
	static constexpr int mkMapLength  = 500;
	static constexpr int mkGridWidth  = mkMapWidth / mkGridXCount;
	static constexpr int mkGridLength = mkMapLength / mkGridZCount;

	/* Others */
	bool mIsRenderBounds = false;

	std::vector<Vec3>	mOpenList{};
	std::vector<Vec3>	mClosedList{};

private:
#pragma region C/Dtor
	BattleScene();
	virtual ~BattleScene() = default;

#pragma endregion

public:
#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	std::vector<sptr<GameObject>> GetAllObjects() const;
	std::set<GridObject*> GetRenderedObjects() const { return mRenderedObjects; }

	rsptr<Grid> GetGridFromPos(const Vec3& pos);
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
	void UpdateAbilityCB(int& idx, const AbilityConstants& value);
	void SetAbilityCB(int idx) const;
#pragma endregion

#pragma region Build
public:
	virtual void Build() override;
	virtual void Release() override;

	// ScriptExporter 정보를 로드한다.
	void LoadScriptExporter(std::ifstream& file, rsptr<Object> object);

private:
	/* Object */
	void InitPlayers();
	void BuildTerrain();

	/* Grid */
	// generate grids
	void BuildGrid();
	// update grid indices for all objects
	void UpdateGridInfo();

	/* Load */
	// 씬 파일에서 모든 객체와 조명의 정보를 불러온다.
	void LoadSceneObjects();
	// 씬 파일에서 모든 객체의 정보를 불러온다. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(std::ifstream& file);

	/* Other */
	// 태그별에 따라 객체를 초기화하고 씬 컨테이너에 객체를 삽입한다.(static, explosive, environments, ...)
	void InitObjectByTag(ObjectTag tag, sptr<GridObject> object);

#pragma endregion

#pragma region Render
public:
	// render scene
	virtual void RenderBegin() override;
	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderCustomDepth() override;
	virtual void RenderForward() override;
	virtual void RenderUI() override;
	virtual void ApplyDynamicContext() override;
	virtual void RenderEnd() override;
	void RenderDynamicEnvironmentMappingObjects();

	void RenderDeferredForServer();

private:
	// 카메라에 보이는 grid만 렌더링한다.
	// 투명, 빌보드 객체는 별도의 Shader를 사용해 렌더링해야 하므로 렌더링하지 않고 그 집합을 반환한다.
	// [renderedObjects]    : 렌더링된 모든 객체 (그리드에 포함된)
	// [transparentObjects] : 투명 객체
	// [billboardObjects]	: 빌보드 객체 (plane)
	void ClearRenderedObjects();
	void RenderGridObjects(RenderType type);
	void RenderSkinMeshObjects(RenderType type);
	void RenderInstanceObjects(RenderType type);
	void RenderObjectsWithFrustumCulling(std::set<GridObject*>& objects, RenderType type);
	void RenderTerrain(RenderType type);
	void RenderAfterSkinImage();

	// render [transparentObjects]
	void RenderDissolveObjects();
	void RenderSkyBox(RenderType type);
	void RenderParticles();

	// [renderedObjects]와 grid의 bounds를 rendering한다.
	bool RenderBounds();
	void RenderObjectBounds();
	void RenderGridBounds();
#pragma endregion


#pragma region Update
public:
	void Start();
	virtual void Update() override;

private:
	void ProcessCollisions();

	// update all objects
	void UpdateObjects();
	void AnimateObjects();
	void UpdateRenderedObjects();

#pragma endregion

public:
	// get objects[out] that collide with [collider] (expensive call cost)
	void CheckCollisionCollider(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type = CollisionType::All) const;
	float CheckCollisionsRay(int gridIndex, const Ray& ray) const;
	void ToggleDrawBoundings() { mIsRenderBounds = !mIsRenderBounds; }
	void ToggleFilterOptions();
	void SetFilterOptions(DWORD option);


	// update objects' grid indices
	void UpdateObjectGrid(GridObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GridObject* object);

	void UpdateSurroundGrids();

	// create new game object from model
	GridObject* Instantiate(const std::string& modelName, ObjectTag tag = ObjectTag::Untagged, bool enable = true);

	void AddDynamicObject(rsptr<GridObject> object) { mDynamicObjects.push_back(object); }
	void RemoveDynamicObject(GridObject* object);

	sptr<ObjectPool> CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);
	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

	std::vector<sptr<Grid>> GetNeighborGrids(int gridIndex, bool includeSelf = false) const;

	void ToggleFullScreen();

	std::vector<sptr<GridObject>> FindObjectsByName(const std::string& name);

	// Unity Scene 스크립트 보유 객체에 대해 각각 Script를 Add 및 초기화 하도록 한다.
	void ProcessInitScriptOjbects(const std::function<void(sptr<Object>)>& processFunc);

	void UpdateTag(GridObject* object, ObjectTag beforeTag);

private:
	// do [processFunc] for activated objects
	void ProcessActiveObjects(const std::function<void(sptr<GridObject>)>& processFunc);
	// do [processFunc] for all objects
	void ProcessAllObjects(const std::function<void(sptr<GridObject>)>& processFunc);

	void RemoveDesrtoyedObjects();

	// move mObjectBuffer's objects to mDynamicObjects
	void PopObjectBuffer();

	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	// 유니티의 tag 문자열을 ObjectTag로 변환한다.
	static ObjectTag GetTagByString(const std::string& tag);
};
#pragma endregion
//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once
#include "Object.h"

class ModelObjectMesh;
class Camera;
class Mesh;

class Material;

struct MATERIAL;

struct CB_GAMEOBJECT_INFO
{
	Vec4x4 mWorldTransform{};
};

struct VS_VB_INSTANCE
{
	Vec4x4 mLocalTransform{};
	Vec4 mColor{};
};






// [ Shader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Shader {
private:
	bool mIsClosed{ true };

protected:
	std::vector<ComPtr<ID3D12PipelineState>> mPipelineStates;

	ComPtr<ID3DBlob> mVertexShaderBlob{};
	ComPtr<ID3DBlob> mPixelShaderBlob{};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	mPipelineStateDesc{};

public:
	Shader();
	virtual ~Shader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	constexpr virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; };

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob);

	virtual void CreateShader();
	virtual void CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat);

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void OnPrepareRender(int pipelineStateIndex = 0);
	virtual void Render(int pipelineStateIndex = 0);

	virtual void UpdateShaderVariable(Vec4x4* world) { }
	virtual void UpdateShaderVariable(MATERIAL* material) { }

	void Close();
};










// [ WireShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WireShader : public Shader {
public:
	WireShader();
	virtual ~WireShader();

	virtual void CreateShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	constexpr virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; };
};










// [ InstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InstancingShader : public Shader {
	using INSTANCE_BUFFER = VS_VB_INSTANCE;

protected:
	std::vector<sptr<GameObject>> mObjects{};
	ComPtr<ID3D12Resource> mInstBuffer{};
	INSTANCE_BUFFER* mMappedObjects{};
	sptr<const Mesh> mMesh;

	template<class Pred>
	void DoAllObjects(Pred pred)
	{
		for (const auto& object : mObjects) {
			pred(object);
		}
	}

public:
	InstancingShader();
	virtual ~InstancingShader();

	std::vector<sptr<GameObject>>& GetObjects() { return mObjects; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual void CreateShader();
	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void Render();

	virtual void Start();
	virtual void Update();

	void SetColor(const Vec3& color);

	void BuildObjects(size_t instanceCount, rsptr<const Mesh> mesh);
};





// [ EffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EffectShader : public InstancingShader {
protected:
	using InstancingShader::BuildObjects;

	// mObjects를 그룹으로 나눈다.
	// 활성화된 그룹
	// <begin, duration>
	std::unordered_map<size_t, float> mActiveGroups{};
	size_t mGroupSize{};
	size_t mCountPerGroup{};

	float mMaxDuration{};

	// 시간 초과 객체 관리
	std::vector<size_t> timeOvers{};

	size_t GetGroupBegin(size_t index);

	template<class Pred>
	void DoActivatedObjects(Pred pred)
	{
		for (auto& [begin, duration] : mActiveGroups) {
			size_t index = begin;
			size_t end = begin + mCountPerGroup;
			for (; index < end; ++index) {
				pred(mObjects[index]);
			}
		}
	}


public:
	EffectShader();
	virtual ~EffectShader();

	virtual void UpdateShaderVariables();

	virtual void Render();

	virtual void Update();

	void SetDuration(float duration) { mMaxDuration = duration; }
	void SetColor(size_t i, const Vec3& color);

	void BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const ModelObjectMesh> mesh);

	// return the activated group
	void SetActive(const Vec3& pos);
};




// [ TexturedEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TexturedEffectShader : public EffectShader {
private:
	sptr<Material> mMaterial;

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void SetMaterial(rsptr<Material> material) { mMaterial = material; }

	virtual void UpdateShaderVariables();
	virtual void Render();
};


// [ StatiShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class StatiShader abstract : public TexturedEffectShader {
private:
	virtual void BuildObjects() abstract;

public:
	virtual void Create();
};

class SmallExpEffectShader : public StatiShader {
private:
	virtual void BuildObjects() override;
};

class BigExpEffectShader : public StatiShader {
private:
	virtual void BuildObjects() override;
};






// [ BulletShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BulletShader : public InstancingShader {
private:
	std::list<sptr<GameObject>> mBuffer{};

public:
	BulletShader();
	virtual ~BulletShader();

	const std::vector<sptr<GameObject>>& GetObjects() { return mObjects; }
	void BuildObjects(size_t bufferSize, rsptr<const MasterModel> model, const Object* owner);
	void SetLifeTime(float bulletLifeTime);
	void SetDamage(float damage);

	void FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);

	virtual void UpdateShaderVariables();

	virtual void Start();
	virtual void Update();
	virtual void Render();

	const std::list<sptr<GameObject>>* GetBullets() const { return &mBuffer; }
};












// [ IlluminatedShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IlluminatedShader : public Shader {
public:
	IlluminatedShader();
	virtual ~IlluminatedShader();

	virtual void CreateShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void Render(int pipelineStateIndex = 0);
};




// [ TexturedShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TexturedShader : public Shader {
public:
	TexturedShader();
	virtual ~TexturedShader();

	virtual void CreateShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

};



// [ ObjectInstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ObjectInstancingShader : public TexturedShader {
public:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};



// [ TransparentShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TransparentShader : public TexturedShader {
public:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();
};



// [ TerrainShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TerrainShader : public Shader {
public:
	TerrainShader();
	virtual ~TerrainShader();

	virtual void CreateShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

};




// [ SkyBoxShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SkyBoxShader : public Shader {
public:
	SkyBoxShader();
	virtual ~SkyBoxShader();

	virtual void CreateShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};



// [ WaterShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WaterShader : public TexturedShader {
public:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};




// [ PostProcessingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Texture;
class PostProcessingShader : public Shader
{
private:
	void CreateTextureResources(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats);
	void CreateSrvs(UINT renderTargetCnt);
	void CreateRtvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

public:
	PostProcessingShader();
	virtual ~PostProcessingShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat);
	virtual void CreateResourcesAndRtvsSrvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	virtual void OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	virtual void OnPostRenderTarget();

	virtual void Render();

protected:
	std::vector<sptr<Texture>> mTextures{};

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mRtvHandles{};

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(UINT index) { return mRtvHandles[index]; }
};


class TextureToFullScreenShader : public PostProcessingShader
{
public:
	TextureToFullScreenShader();
	virtual ~TextureToFullScreenShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};



class BillboardShader : public TexturedShader {
public:
	virtual void CreateShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

class SpriteShader : public BillboardShader {
public:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};



class CanvasShader : public TexturedShader {
public:
	virtual void CreateShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};
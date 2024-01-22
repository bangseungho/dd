#pragma once

#pragma region Include
#include "UploadBuffer.h"
#pragma endregion

#pragma region Struct
struct PassConstants {
    Matrix      MtxView{};
    Matrix      MtxProj{};
    Vec3        EyeW{};
    float       DeltaTime{};
    SceneLight  Lights{};
};

struct ObjectConstants {
    Matrix      MtxWorld{};
    Matrix      MtxSprite{};
    int         MatSBIdx{};
    Vec3        Padding{};
};

struct MaterialData {
    Vec4        Ambient{ Vector4::One() };
    Vec4        Diffuse{};
    Vec4        Specular{};
    Vec4        Emissive{};

    int         DiffuseMapIndex{ -1 };
    int         NormalMapIndex{ -1 };
    int         RoughnessMapIndex{ -1 };
    int         HeightMapIndex{ -1 };
};
#pragma endregion

#pragma region Class
struct FrameResource : private UnCopyable {
public:
    UINT64                              Fence{};
    ComPtr<ID3D12CommandAllocator>      CmdAllocator{};
    
    uptr<UploadBuffer<PassConstants>>   PassCB{};       // 패스 상수 버퍼
    uptr<UploadBuffer<ObjectConstants>> ObjectCB{};     // 오브젝트 상수 버퍼

    uptr<UploadBuffer<MaterialData>>    MaterialSB{}; // 머티리얼 구조적 버퍼

public:
#pragma region C/Dtor
    FrameResource(ID3D12Device* pDevice, int passCount, int objectCount, int materialCount);
    ~FrameResource() = default;
#pragma endregion
};

// 모든 프레임 리소스를 관리하는 클래스
class FrameResourceMgr {
private:
    int mFrameResourceCount;
    int mPassCount;
    int mObjectCount;
    int mMaterialCount;

    ID3D12Fence*                                mFence{};
    std::vector<uptr<FrameResource>>			mFrameResources{};
    FrameResource*                              mCurrFrameResource{};       // 현재 프레임 리소스
    int											mCurrFrameResourceIndex{};	// 현재 프레임 인덱스

    std::unordered_set<int>                     mActiveObjCBIdxes{};        // 사용중인 오브젝트 상수 버퍼 인덱스
    std::queue<int>                             mAvailableObjCBIdxes{};     // 사용 가능한 오브젝트 상수 버퍼 인덱스
    
    std::unordered_set<int>                     mActiveMaterialSBIdxes{};        // 사용중인 머티리얼 버퍼 인덱스
    std::queue<int>                             mAvailableMaterialSBIdxes{};     // 사용 가능한 머티리얼 버퍼 인덱스

public:
#pragma region C/Dtor
    FrameResourceMgr(ID3D12Fence* fence);
    ~FrameResourceMgr() = default;
#pragma endregion

public:
#pragma region Getter
    FrameResource* GetCurrFrameResource() const { return mCurrFrameResource; }
    const D3D12_GPU_VIRTUAL_ADDRESS GetPassCBGpuAddr() const;
    const D3D12_GPU_VIRTUAL_ADDRESS GetMatSBGpuAddr() const;
    const D3D12_GPU_VIRTUAL_ADDRESS GetObjCBGpuAddr(int elementIndex) const;
#pragma endregion

    void CreateFrameResources(ID3D12Device* pDevice);
    void Update();

    // 객체 소멸시 사용하지 않는 상수 버퍼 인덱스를 반환하는 함수
    void ReturnObjCBIdx(int elementIndex);

    // 패스 당 상수 버퍼에 데이터 복사
    void CopyData(const PassConstants& data);

    // 오브젝트 당 상수 버퍼에 데이터 복사
    void CopyData(int& elementIndex, const ObjectConstants& data);

    // 머티리얼 당 상수 버퍼에 데이터 복사
    void CopyData(int& elementIndex, const MaterialData& data);
};

#pragma endregion


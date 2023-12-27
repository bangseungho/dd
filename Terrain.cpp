#include "stdafx.h"
#include "Terrain.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"






CHeightMapTerrain::CHeightMapTerrain(LPCTSTR fileName, int width, int length, int blockWidth, int blockLength)
{
	mWidth = width;
	mLength = length;

	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. blockWidth, blockLength는 격자 메쉬 하나의 가로, 세로 크기이다. quadsPerBlock, quadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int xQuadsPerBlock = blockWidth - 1;
	int zQuadsPerBlock = blockLength - 1;

	mHeightMapImage = std::make_shared<CHeightMapImage>(fileName, width, length);

	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다.
	long xBlocks = (mWidth - 1) / xQuadsPerBlock;
	long zBlocks = (mLength - 1) / zQuadsPerBlock;

	mTerrains.resize(xBlocks * zBlocks);
	mBuffer.resize(xBlocks * zBlocks);

	float meshRadius = std::sqrtf((blockWidth * blockWidth) + (blockLength * blockLength)) * 1.1f;
	MyBoundingSphere bs;
	bs.Radius = meshRadius;

	for (int z = 0, zStart = 0; z < zBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < xBlocks; x++)
		{
			xStart = x * (blockWidth - 1);
			zStart = z * (blockLength - 1);

			// 각자 메쉬를 생성해 저장
			int index = x + (z * xBlocks);
			sptr<CHeightMapGridMesh> mesh = std::make_shared<CHeightMapGridMesh>(xStart, zStart, blockWidth, blockLength, mHeightMapImage);
			mTerrains[index] = std::make_shared<CTerrainBlock>(mesh);
			mTerrains[index]->SetTerrain(this);

			Vec3 center = Vec3(xStart + blockWidth / 2, 0.f, zStart + blockLength / 2);
			mTerrains[index]->SetPosition(center);
			bs.Center = center;
			mTerrains[index]->AddComponent<SphereCollider>()->mBS = bs;
		}
	}

	MATERIALLOADINFO materialInfo;
	materialInfo.mEmissive = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	materialInfo.mGlossiness = 0.1414213f;
	materialInfo.mMetallic = 0.0f;
	materialInfo.mSpecularHighlight = 1.0f;
	materialInfo.mGlossyReflection = 1.0f;

	sptr<CMaterialColors> materialColors = std::make_shared<CMaterialColors>(materialInfo);
	mMaterial = std::make_shared<CMaterial>();
	mMaterial->SetMaterialColors(materialColors);

	mTextureLayer[0] = crntScene->GetTexture("Detail_Texture_7");
	mTextureLayer[1] = crntScene->GetTexture("Detail_Texture_6");
	mTextureLayer[2] = crntScene->GetTexture("Stone"); 
	mTextureLayer[3] = crntScene->GetTexture("GrassUV01");
	mSplatMap = crntScene->GetTexture("Terrain_splatmap");

	mTextureLayer[0]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::TerrainLayer0));
	mTextureLayer[1]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::TerrainLayer1));
	mTextureLayer[2]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::TerrainLayer2));
	mTextureLayer[3]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::TerrainLayer3));
	mSplatMap->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::SplatMap));

	mShader = std::make_shared<CTerrainShader>();
	mShader->CreateShader();
}

CHeightMapTerrain::~CHeightMapTerrain()
{

}

float CHeightMapTerrain::GetHeight(float x, float z)
{
	return mHeightMapImage->GetHeight(x, z);
}

Vec3 CHeightMapTerrain::GetNormal(float x, float z)
{
	return mHeightMapImage->GetHeightMapNormal(static_cast<int>(x), static_cast<int>(z));
}

int CHeightMapTerrain::GetHeightMapWidth()
{
	return mHeightMapImage->GetHeightMapWidth();
}
int CHeightMapTerrain::GetHeightMapLength()
{
	return mHeightMapImage->GetHeightMapLength();
}

void CHeightMapTerrain::PushObject(CTerrainBlock* terrain)
{
	assert(mCrntBufferIndex < mBuffer.size());

	mBuffer[mCrntBufferIndex++] = terrain;
}

void CHeightMapTerrain::Render(bool isMirror)
{
	if (!isMirror) {
		mShader->Render();
	}

	UpdateShaderVariable();
	mMaterial->UpdateShaderVariable();

	mTextureLayer[0]->UpdateShaderVariables();
	mTextureLayer[1]->UpdateShaderVariables();
	mTextureLayer[2]->UpdateShaderVariables();
	mTextureLayer[3]->UpdateShaderVariables();
	mSplatMap->UpdateShaderVariables();

	for (UINT i = 0; i < mCrntBufferIndex; ++i) {
		mBuffer[i]->RenderMesh();
	}

	ResetBuffer();
}

void CHeightMapTerrain::Start()
{
	for (auto& terrain : mTerrains) {
		terrain->Start();
	}
}

void CHeightMapTerrain::Update()
{

}






// [ CHeightMapImage ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float uint16ToFloat(std::uint16_t value)
{
	return static_cast<float>(value) / 65535.0f * 255.0f;
}

CHeightMapImage::CHeightMapImage(LPCTSTR fileName, int width, int length)
{
	mWidth = width;
	mLength = length * 2; // [ERROR] *2 안하면 절반밖에 로딩이 안되는 문제
	std::vector<uint16_t> pHeightMapPixels(mWidth * mLength);

	//파일을 열고 읽는다. 높이 맵 이미지는 파일 헤더가 없는 RAW 이미지이다.
	HANDLE hFile = ::CreateFile(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);

	DWORD dwBytesRead;
	BOOL success = ::ReadFile(hFile, pHeightMapPixels.data(), (mWidth * mLength), &dwBytesRead, nullptr);
	::CloseHandle(hFile);

	mHeightMapPixels.resize(mWidth * mLength);
	for (int y = 0; y < mLength; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			mHeightMapPixels[x + (y * mWidth)] = uint16ToFloat(pHeightMapPixels[x + (y * mWidth)]);
		}
	}
}

CHeightMapImage::~CHeightMapImage()
{

}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER
float CHeightMapImage::GetHeight(float fx, float fz)
{
	/*지형의 좌표 (fx, fz)는 이미지 좌표계이다. 높이 맵의 x-좌표와 z-좌표가 높이 맵의 범위를 벗어나면 지형의 높이는 0이다.*/
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= mWidth - 1) || (fz >= mLength - 1)) {
		return 0.0f;
	}
	//높이 맵의 좌표의 정수 부분과 소수 부분을 계산한다.
	int x = static_cast<int>(fx);
	int z = static_cast<int>(fz);
	float xPercent = fx - x;
	float zPercent = fz - z;

	/*fx = max(256, fx);
	fz = max(256, fz);*/
	float bottomLeft = static_cast<float>(mHeightMapPixels[x + (z * mWidth)]);
	float bottomRight = static_cast<float>(mHeightMapPixels[(x + 1) + (z * mWidth)]);
	float topLeft = static_cast<float>(mHeightMapPixels[x + ((z + 1) * mWidth)]);
	float topRight = static_cast<float>(mHeightMapPixels[(x + 1) + ((z + 1) * mWidth)]);

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	//z-좌표가 1, 3, 5, ...인 경우 인덱스가 오른쪽에서 왼쪽으로 나열된다.
	bool isRightToLeft = ((z % 2) != 0);
	if (isRightToLeft)
	{
		/*지형의 삼각형들이 오른쪽에서 왼쪽 방향으로 나열되는 경우이다.
		다음 그림의 오른쪽은 (zPercent < xPercent) 인 경우이다.
		이 경우 TopLeft의 픽셀 값은 (topLeft = topRight + (bottomLeft - bottomRight))로 근사한다.
		다음 그림의 왼쪽은 (zPercent ≥ xPercent)인 경우이다.
		이 경우 BottomRight의 픽셀 값은 (bottomRight = bottomLeft + (topRight - topLeft))로 근사한다.*/

		if (zPercent >= xPercent)
			bottomRight = bottomLeft + (topRight - topLeft);
		else
			topLeft = topRight + (bottomLeft - bottomRight);
	}
	else
	{
		/*지형의 삼각형들이 왼쪽에서 오른쪽 방향으로 나열되는 경우이다.
		다음 그림의 왼쪽은 (zPercent < (1.0f - xPercent))인 경우이다.
		이 경우 TopRight의 픽셀 값은 (topRight = topLeft + (bottomRight - bottomLeft))로 근사한다.
		다음 그림의 오른쪽은 (zPercent ≥ (1.0f - xPercent))인 경우이다.
		이 경우 BottomLeft의 픽셀 값은 (bottomLeft = topLeft + (bottomRight - topRight))로 근사한다.*/
		if (zPercent < (1.0f - xPercent))
			topRight = topLeft + (bottomRight - bottomLeft);
		else
			bottomLeft = topLeft + (bottomRight - topRight);
	}
#endif

	//사각형의 네 점을 보간하여 높이(픽셀 값)를 계산한다.
	float topHeight = topLeft * (1 - xPercent) + topRight * xPercent;
	float bottomHeight = bottomLeft * (1 - xPercent) + bottomRight * xPercent;
	float height = bottomHeight * (1 - zPercent) + topHeight * zPercent;
	return height;
}
Vec3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	// x - 좌표와 z - 좌표가 높이 맵의 범위를 벗어나면 지형의 법선 벡터는 y - 축 방향 벡터이다.
	if ((x < 0.0f) || (z < 0.0f) || (x >= mWidth) || (z >= mLength)) {
		return Vec3(0.0f, 1.0f, 0.0f);
	}

	/*높이 맵에서 (x, z) 좌표의 픽셀 값과 인접한 두 개의 점 (x+1, z), (z, z+1)에 대한 픽셀 값을 사용하여 법선 벡터를 계산한다.*/
	int heightMapIndex = x + (z * mWidth);
	int xHeightMapAdd = (x < (mWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (mLength - 1)) ? mWidth : -mWidth;

	float y1 = (float)mHeightMapPixels[heightMapIndex];
	float y2 = (float)mHeightMapPixels[heightMapIndex + xHeightMapAdd];
	float y3 = (float)mHeightMapPixels[heightMapIndex + zHeightMapAdd];

	Vec3 edge1 = Vec3(0.0f, y3 - y1, 1.0f);
	Vec3 edge2 = Vec3(1.0f, y2 - y1, 0.0f);
	Vec3 normal = Vector3::CrossProduct(edge1, edge2, true);

	return normal;
}



// [ CHeightMapGridMesh ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 중심점(center)을 기준으로 정점들의 위치를 순회하며 BoundingSphere를 계산
MyBoundingSphere CalculateBoundingSphere(const Vec3& center, const std::vector<Vec3>& positions) {
	float maxDistanceSq = 0.0f;

	// 모든 정점 위치에 대해 평균 중심간 최대 거리 계산
	for (const auto& position : positions) {
		float distanceSq = (position.x - center.x) * (position.x - center.x) +
			(position.y - center.y) * (position.y - center.y) +
			(position.z - center.z) * (position.z - center.z);
		maxDistanceSq = max(maxDistanceSq, distanceSq);
	}

	// BoundingSphere 반지름 설정
	float radius = std::sqrt(maxDistanceSq);

	MyBoundingSphere result;
	result.OriginCenter = center;
	result.Center = center;
	result.Radius = radius;
	return result;
}

// 정점들의 위치를 순회하며 BoundingSphere를 계산
MyBoundingSphere CalculateBoundingSphere(const std::vector<Vec3>& positions) {
	Vec3 center{ 0.0f, 0.0f, 0.0f };
	float maxDistanceSq = 0.0f;

	// 모든 정점 위치에 대한 평균 중심 위치 계산
	for (const auto& position : positions) {
		center.x += position.x;
		center.y += position.y;
		center.z += position.z;
	}
	int vertexCount = positions.size();
	center.x /= vertexCount;
	center.y /= vertexCount;
	center.z /= vertexCount;

	return CalculateBoundingSphere(center, positions);
}

CHeightMapGridMesh::CHeightMapGridMesh(int xStart, int zStart, int width, int length, rsptr<CHeightMapImage> heightMapImage)
{
	static constexpr float corr = 1.0f / TERRAIN_LENGTH;	// for SplatMap
	static constexpr float detailScale = 0.1f;
	mStride = sizeof(Vec3);
	mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	mVertexCount = width * length;
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs0;
	std::vector<Vec2> uvs1;
	positions.resize(width * length);
	normals.resize(width * length);
	uvs0.resize(width * length);
	uvs1.resize(width * length);

	float height = 0.0f;
	float minHeight = +FLT_MAX;
	float maxHeight = -FLT_MAX;

	for (int i = 0, z = zStart; z < (zStart + length); z++) {
		for (int x = xStart; x < (xStart + width); x++, i++) {
			positions[i] = Vec3(x, OnGetHeight(x, z, heightMapImage), z);
			normals[i] = heightMapImage->GetHeightMapNormal(x, z);
			uvs0[i] = Vec2(float(x) * detailScale, float(z) * detailScale);

			uvs1[i] = Vec2(float(x) * corr, float(z) * corr);

			if (height < minHeight) minHeight = height;
			if (height > maxHeight) maxHeight = height;
		}
	}

	::CreateVertexBufferResource<Vec3>(positions, mVertexUploadBuffer, mVertexBuffer);
	::CreateVertexBufferResource<Vec3>(normals, mNormalUploadBuffer, mNormalBuffer);
	::CreateVertexBufferResource<Vec2>(uvs0, mUV0UploadBuffer, mUV0Buffer);
	::CreateVertexBufferResource<Vec2>(uvs1, mUV1UploadBuffer, mUV1Buffer);

	CreateVertexBufferViews();


	mIndexCount = ((width * 2) * (length - 1)) + ((length - 1) - 1);
	std::vector<UINT> indices(mIndexCount);
	for (int j = 0, z = 0; z < length - 1; z++) {
		if ((z % 2) == 0) {
			for (int x = 0; x < width; x++) {
				if ((x == 0) && (z > 0)) indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)((x + (z * width)) + width);
			}
		}
		else {
			for (int x = width - 1; x >= 0; x--) {
				if (x == (width - 1)) indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)((x + (z * width)) + width);
			}
		}
	}

	::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);
	::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);
}
CHeightMapGridMesh::~CHeightMapGridMesh()
{

}


//격자의 좌표가 (x, z)일 때 교점(정점)의 높이를 반환하는 함수이다.
float CHeightMapGridMesh::OnGetHeight(int x, int z, rsptr<CHeightMapImage> heightMapImage)
{
	const std::vector<float>& heightMapPixels = heightMapImage->GetHeightMapPixels();
	int width = heightMapImage->GetHeightMapWidth();

	return heightMapPixels[x + (z * width)];
}

void CHeightMapGridMesh::Render() const
{
	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());

	if (mIndexBuffer) {
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(mVertexCount, 1, mOffset, 0);
	}
}




CTerrainBlock::CTerrainBlock(rsptr<CHeightMapGridMesh> mesh) : CGameObject()
{
	mMesh = mesh;

	SetTag(ObjectTag::Terrain);
}

void CTerrainBlock::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
}

void CTerrainBlock::Render()
{
	Push();
}

void CTerrainBlock::RenderMesh()
{
	mMesh->Render();
	Update();
}
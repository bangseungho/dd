#include "stdafx.h"
#include "Mesh.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Model.h"
#include "Shader.h"
#include "Scene.h"


#include "Script_Sprite.h"






#pragma region Mesh
void Mesh::ReleaseUploadBuffers()
{
	mVertexUploadBuffer    = nullptr;
	mNormalUploadBuffer    = nullptr;
	mUV0UploadBuffer       = nullptr;
	mUV1UploadBuffer       = nullptr;
	mTangentUploadBuffer   = nullptr;
	mBiTangentUploadBuffer = nullptr;
	mIndexUploadBuffer     = nullptr;
};

void Mesh::Render() const
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

void Mesh::RenderInstanced(UINT instanceCount) const
{
	if (instanceCount <= 0) {
		return;
	}

	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	if (mIndexBuffer) {
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCount, instanceCount, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(mVertexCount, instanceCount, mOffset, 0);
	}
}

void Mesh::CreateVertexBufferViews()
{
	VertexBufferViews bufferViews{};
	bufferViews.VertexBuffer    = mVertexBuffer;
	bufferViews.NormalBuffer    = mNormalBuffer;
	bufferViews.UV0Buffer       = mUV0Buffer;
	bufferViews.UV1Buffer       = mUV1Buffer;
	bufferViews.TangentBuffer   = mTangentBuffer;
	bufferViews.BiTangentBuffer = mBiTangentBuffer;

	D3DUtil::CreateVertexBufferViews(mVertexBufferViews, mVertexCount, bufferViews);
}

void Mesh::CreateIndexBuffer(const std::vector<UINT>& indices)
{
	D3DUtil::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);
	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);
}
#pragma endregion





#pragma region ModelObjectMesh
void ModelObjectMesh::CreateMeshFromOBB(const BoundingOrientedBox& box)
{
	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	mVertexCount = 8;
	mIndexCount  = 36;

	vertices.resize(mVertexCount);

	vertices[0] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LDB()));
	vertices[1] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[2] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LUB()));
	vertices[3] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[4] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LDF()));
	vertices[5] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[6] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LUF()));
	vertices[7] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RUF()));

	indices.resize(mIndexCount);
	indices = {
		0, 1, 2, 2, 1, 3, // Front face
		4, 6, 5, 5, 6, 7, // Back face
		0, 2, 4, 4, 2, 6, // Left face
		1, 5, 3, 3, 5, 7, // Right face
		0, 4, 1, 1, 4, 5, // Bottom face
		2, 3, 6, 6, 3, 7  // Top face
	};


	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBuffer(indices);
}

void ModelObjectMesh::CreateCubeMesh(float width, float height, float depth, bool hasTexture, bool isLine)
{
	const float x = width * 0.5f, y = height * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	if (!isLine) {
		mIndexCount = 36;
		indices.resize(mIndexCount);

		if (!hasTexture) {
			mVertexCount = 8;
			vertices.resize(mVertexCount);
			vertices[0] = Vec3(-x, +y, -z);
			vertices[1] = Vec3(+x, +y, -z);
			vertices[2] = Vec3(+x, +y, +z);
			vertices[3] = Vec3(-x, +y, +z);
			vertices[4] = Vec3(-x, -y, -z);
			vertices[5] = Vec3(+x, -y, -z);
			vertices[6] = Vec3(+x, -y, +z);
			vertices[7] = Vec3(-x, -y, +z);

			indices = {
				3, 1, 0, 2, 1, 3,
				0, 5, 4, 1, 5, 0,
				3, 4, 7, 0, 4, 3,
				1, 6, 5, 2, 6, 1,
				2, 7, 6, 3, 7, 2,
				6, 4, 5, 7, 4, 6
			};
		}
		else {
			mVertexCount = 24;
			vertices.resize(mVertexCount);
			vertices[0] = Vec3(+x, -y, +z);
			vertices[1] = Vec3(+x, +y, +z);
			vertices[2] = Vec3(-x, +y, +z);
			vertices[3] = Vec3(-x, -y, +z);
			vertices[4] = Vec3(-x, +y, -z);
			vertices[5] = Vec3(+x, +y, -z);
			vertices[6] = Vec3(+x, -y, -z);
			vertices[7] = Vec3(-x, -y, -z);
			vertices[8] = Vec3(-x, +y, +z);
			vertices[9] = Vec3(+x, +y, +z);
			vertices[10] = Vec3(+x, +y, -z);
			vertices[11] = Vec3(-x, +y, -z);
			vertices[12] = Vec3(+x, -y, -z);
			vertices[13] = Vec3(+x, -y, +z);
			vertices[14] = Vec3(-x, -y, +z);
			vertices[15] = Vec3(-x, -y, -z);
			vertices[16] = Vec3(-x, +y, +z);
			vertices[17] = Vec3(-x, +y, -z);
			vertices[18] = Vec3(-x, -y, -z);
			vertices[19] = Vec3(-x, -y, +z);
			vertices[20] = Vec3(+x, +y, -z);
			vertices[21] = Vec3(+x, +y, +z);
			vertices[22] = Vec3(+x, -y, +z);
			vertices[23] = Vec3(+x, -y, -z);

			indices = {
				0, 1, 2, 0, 2, 3,
				4, 5, 6, 4, 6, 7,
				8, 9, 10, 8, 10, 11,
				12, 13, 14, 12, 14, 15,
				16, 17, 18, 16, 18, 19,
				20, 21, 22, 20, 22, 23
			};

			std::vector<Vec2> UVs(mVertexCount);
			UVs[0] = Vec2(0.f, 1.f);
			UVs[1] = Vec2(0.f, 0.f);
			UVs[2] = Vec2(1.f, 0.f);
			UVs[3] = Vec2(1.f, 1.f);
			UVs[4] = Vec2(0.f, 0.f);
			UVs[5] = Vec2(1.f, 0.f);
			UVs[6] = Vec2(1.f, 1.f);
			UVs[7] = Vec2(0.f, 1.f);
			UVs[8] = Vec2(0.f, 0.f);
			UVs[9] = Vec2(1.f, 0.f);
			UVs[10] = Vec2(0.f, 1.f);
			UVs[11] = Vec2(1.f, 1.f);
			UVs[12] = Vec2(0.f, 0.f);
			UVs[13] = Vec2(0.f, 1.f);
			UVs[14] = Vec2(1.f, 1.f);
			UVs[15] = Vec2(1.f, 0.f);
			UVs[16] = Vec2(0.f, 0.f);
			UVs[17] = Vec2(1.f, 0.f);
			UVs[18] = Vec2(1.f, 1.f);
			UVs[19] = Vec2(0.f, 1.f);
			UVs[20] = Vec2(0.f, 0.f);
			UVs[21] = Vec2(1.f, 0.f);
			UVs[22] = Vec2(1.f, 1.f);
			UVs[23] = Vec2(0.f, 1.f);

			D3DUtil::CreateVertexBufferResource(UVs, mUV0UploadBuffer, mUV0Buffer);
		}
	}
	else {
		mVertexCount = 8;
		mIndexCount = 24;

		vertices.resize(mVertexCount);

		vertices[0] = Vec3(-x, -y, -z);
		vertices[1] = Vec3(x, -y, -z);
		vertices[2] = Vec3(x, -y, z);
		vertices[3] = Vec3(-x, -y, z);
		vertices[4] = Vec3(-x, y, -z);
		vertices[5] = Vec3(x, y, -z);
		vertices[6] = Vec3(x, y, z);
		vertices[7] = Vec3(-x, y, z);

		indices.resize(mIndexCount);
		indices = {
			0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
			4, 5, 5, 6, 6, 7, 7, 4, // Top face
			0, 4, 1, 5, 2, 6, 3, 7, // Connecting lines
		};
	}


	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBuffer(indices);
}

void ModelObjectMesh::CreateSkyBoxMesh(float width, float height, float depth)
{
	const float x = width * 0.5f, y = height * 0.5f, z = depth * 0.5f;

	mVertexCount = 36;

	std::vector<Vec3> vertices;

	vertices.resize(mVertexCount);
	vertices[0] = Vec3(-x, +x, +x);
	vertices[1] = Vec3(+x, +x, +x);
	vertices[2] = Vec3(-x, -x, +x);
	vertices[3] = Vec3(-x, -x, +x);
	vertices[4] = Vec3(+x, +x, +x);
	vertices[5] = Vec3(+x, -x, +x);
	// Back Quad										
	vertices[6] = Vec3(+x, +x, -x);
	vertices[7] = Vec3(-x, +x, -x);
	vertices[8] = Vec3(+x, -x, -x);
	vertices[9] = Vec3(+x, -x, -x);
	vertices[10] = Vec3(-x, +x, -x);
	vertices[11] = Vec3(-x, -x, -x);
	// Left Quad										
	vertices[12] = Vec3(-x, +x, -x);
	vertices[13] = Vec3(-x, +x, +x);
	vertices[14] = Vec3(-x, -x, -x);
	vertices[15] = Vec3(-x, -x, -x);
	vertices[16] = Vec3(-x, +x, +x);
	vertices[17] = Vec3(-x, -x, +x);
	// Right Quad										
	vertices[18] = Vec3(+x, +x, +x);
	vertices[19] = Vec3(+x, +x, -x);
	vertices[20] = Vec3(+x, -x, +x);
	vertices[21] = Vec3(+x, -x, +x);
	vertices[22] = Vec3(+x, +x, -x);
	vertices[23] = Vec3(+x, -x, -x);
	// Top Quad											
	vertices[24] = Vec3(-x, +x, -x);
	vertices[25] = Vec3(+x, +x, -x);
	vertices[26] = Vec3(-x, +x, +x);
	vertices[27] = Vec3(-x, +x, +x);
	vertices[28] = Vec3(+x, +x, -x);
	vertices[29] = Vec3(+x, +x, +x);
	// Bottom Quad										
	vertices[30] = Vec3(-x, -x, +x);
	vertices[31] = Vec3(+x, -x, +x);
	vertices[32] = Vec3(-x, -x, -x);
	vertices[33] = Vec3(-x, -x, -x);
	vertices[34] = Vec3(+x, -x, +x);
	vertices[35] = Vec3(+x, -x, -x);

	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
}

void ModelObjectMesh::CreatePlaneMesh(float width, float depth, bool isLine)
{
	const float x = width * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<Vec2> UV0;

	if (!isLine) {
		mVertexCount = 6;

		vertices.resize(mVertexCount);
		vertices[0] = Vec3(+x, -z, 0.f);
		vertices[1] = Vec3(+x, +z, 0.f);
		vertices[2] = Vec3(-x, +z, 0.f);

		vertices[3] = Vec3(+x, -z, 0.f);
		vertices[4] = Vec3(-x, +z, 0.f);
		vertices[5] = Vec3(-x, -z, 0.f);

		UV0.resize(mVertexCount);
		UV0[0] = Vec2(0.f, 1.f);
		UV0[1] = Vec2(0.f, 0.f);
		UV0[2] = Vec2(1.f, 0.f);

		UV0[3] = Vec2(0.f, 1.f);
		UV0[4] = Vec2(1.f, 0.f);
		UV0[5] = Vec2(1.f, 1.f);

		D3DUtil::CreateVertexBufferResource(UV0, mUV0UploadBuffer, mUV0Buffer);
	}
	else {
		mVertexCount = 8;

		vertices.resize(mVertexCount);
		vertices[0] = Vec3(-x, 0.f, -z);
		vertices[1] = Vec3(-x, 0.f, +z);

		vertices[2] = Vec3(-x, 0.f, +z);
		vertices[3] = Vec3(+x, 0.f, +z);

		vertices[4] = Vec3(+x, 0.f, +z);
		vertices[5] = Vec3(+x, 0.f, -z);

		vertices[6] = Vec3(+x, 0.f, -z);
		vertices[7] = Vec3(-x, 0.f, -z);
	}

	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
}


void ModelObjectMesh::CreateSphereMesh(float radius, int numSegments, bool isLine)
{
	// Calculate the number of vertices and indices needed
	const int numVertices = (numSegments + 1) * (numSegments + 1);

	int numIndices{};
	if (!isLine) {
		numIndices = numSegments * numSegments * 6;
	}
	else {
		numIndices = numSegments * numSegments * 2 * 4;
	}

	mVertexCount = numVertices;
	mIndexCount  = numIndices;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	// Create vertices for the sphere
	vertices.resize(numVertices);
	float phi, theta;
	float phiStep   = Math::kPI / numSegments;
	float thetaStep = 2.0f * Math::kPI / numSegments;
	int vertexIndex = 0;

	for (int i = 0; i <= numSegments; ++i) {
		phi = i * phiStep;
		for (int j = 0; j <= numSegments; ++j) {
			theta                   = j * thetaStep;
			float x                 = radius * sinf(phi) * cosf(theta);
			float y                 = radius * cosf(phi);
			float z                 = radius * sinf(phi) * sinf(theta);
			vertices[vertexIndex++] = Vec3(x, y, z);
		}
	}

	// Create indices for the sphere
	indices.resize(numIndices);
	int index = 0;

	if (!isLine) {
		for (int i = 0; i < numSegments; ++i) {
			for (int j = 0; j < numSegments; ++j) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = vertexIndex0 + 1;
				int vertexIndex2 = (i + 1) * (numSegments + 1) + j;
				int vertexIndex3 = vertexIndex2 + 1;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
				indices[index++] = vertexIndex2;
				indices[index++] = vertexIndex1;
				indices[index++] = vertexIndex3;
				indices[index++] = vertexIndex2;
			}
		}
	}
	else {
		// Create vertical lines
		for (int i = 0; i <= numSegments; ++i) {
			for (int j = 0; j < numSegments; ++j) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = vertexIndex0 + 1;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
			}
		}

		// Create horizontal lines
		for (int j = 0; j <= numSegments; ++j) {
			for (int i = 0; i < numSegments; ++i) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = (i + 1) * (numSegments + 1) + j;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
			}
		}
	}

	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);
	
	CreateVertexBufferViews();
	CreateIndexBuffer(indices);
}
#pragma endregion





#pragma region MergedMesh
MergedMesh::MergedMesh()
{
	mMeshBuffer = std::make_unique<MeshBuffer>();
}

rsptr<Texture> MergedMesh::GetTexture() const
{
	return mFrameMeshInfo.front().Materials.front()->mTexture;
}

// 1. MergeMesh
// 2. MergeMaterial
// 3. ...
// 4. SetObject (transform)
// 5. Close
bool MergedMesh::MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials)
{
	FrameMeshInfo modelMeshInfo{};

	if (!mesh) {
		mFrameMeshInfo.emplace_back(modelMeshInfo);
		return false;
	}
	CopyBack(materials, modelMeshInfo.Materials);

	// copy vertices
	const std::vector<Vec3>& meshVertices = mesh->Buffer.Vertices;
	const std::vector<Vec3>& meshNormals  = mesh->Buffer.Normals;

	CopyBack(meshVertices, mMeshBuffer->Vertices);
	CopyBack(meshNormals,  mMeshBuffer->Normals);

	if (modelMeshInfo.Materials.front()->mTexture) {
		const std::vector<Vec3>& meshTangents   = mesh->Buffer.Tangents;
		const std::vector<Vec3>& meshBiTangents = mesh->Buffer.BiTangents;
		const std::vector<Vec2>& meshUVs0       = mesh->Buffer.UVs0;
		//const std::vector<Vec2>& meshUVs1     = mesh->mUV1;

		CopyBack(meshTangents,	 mMeshBuffer->Tangents);
		CopyBack(meshBiTangents, mMeshBuffer->BiTangents);
		CopyBack(meshUVs0,		 mMeshBuffer->UVs0);
		//CopyBack(meshUVs1, mUVs1);
	}

	// set vertexCount
	modelMeshInfo.VertexCount = mesh->Buffer.Vertices.size();
	mVertexCount += meshVertices.size();

	// merge & copy SubMeshes
	MergeSubMeshes(mesh, modelMeshInfo);

	mFrameMeshInfo.emplace_back(modelMeshInfo);

	// unlink
	mesh = nullptr;
	materials.clear();

	return true;
}

void MergedMesh::StopMerge()
{
	mVertexBuffer = nullptr;
	mNormalBuffer = nullptr;
	mTangentBuffer = nullptr;
	mBiTangentBuffer = nullptr;
	mUV0Buffer = nullptr;
	mUV1Buffer = nullptr;
	mIndexBuffer = nullptr;

	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Vertices, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->UVs0, mUV0UploadBuffer, mUV0Buffer);
	//D3DUtil::CreateVertexBufferResource(mMeshBuffer->UVs1, mUV1UploadBuffer, mUV1Buffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Normals, mNormalUploadBuffer, mNormalBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Tangents, mTangentUploadBuffer, mTangentBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->BiTangents, mBiTangentUploadBuffer, mBiTangentBuffer);

	CreateVertexBufferViews();
	CreateIndexBuffer(mMeshBuffer->Indices);

	mMeshBuffer = nullptr;
}

void UpdateShaderVars(const Vec4x4& transform)
{
	scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrix::Transpose(transform), 0);
}


void MergedMesh::Render(const GameObject* object) const
{
	Render(object->GetMergedTransform());
}

void MergedMesh::Render(const ObjectInstBuffer* instBuffer) const
{
	if (!instBuffer) {
		return;
	}

	UINT instanceCount = instBuffer->GetInstanceCount();
	if (instanceCount <= 0) {
		return;
	}

	instBuffer->UpdateShaderVars();

	Render(instBuffer->GetMergedTransform(), instanceCount);
}


void MergedMesh::RenderSprite(const GameObject* object) const
{
	constexpr int rootIndex{ 0 };
	if (!HasMesh(rootIndex)) {
		return;
	}

	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	cmdList->IASetIndexBuffer(&mIndexBufferView);

	constexpr UINT transformIndex{ 0 };
	const FrameMeshInfo& modelMeshInfo = mFrameMeshInfo[transformIndex];

	modelMeshInfo.Materials.front()->UpdateShaderVars();
	object->GetComponent<Script_Sprite>()->UpdateSpriteVariable();

	constexpr UINT indexCount{ 6 };
	cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void MergedMesh::MergeSubMeshes(rsptr<MeshLoadInfo> mesh, FrameMeshInfo& modelMeshInfo)
{
	const UINT subMeshCount = mesh->SubMeshCount;
	modelMeshInfo.IndexCounts.resize(subMeshCount);

	for (int i = 0; i < subMeshCount; ++i) {
		std::vector<UINT>& indices = mesh->SubSetIndices[i];
		modelMeshInfo.IndexCounts[i] = indices.size();
		CopyBack(indices, mMeshBuffer->Indices);
	}

	for (UINT indexCount : modelMeshInfo.IndexCounts) {
		mIndexCount += indexCount;
	}
}

void MergedMesh::Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCount) const
{
	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	cmdList->IASetIndexBuffer(&mIndexBufferView);

	UINT indexLocation{ 0 };
	UINT vertexLocation{ 0 };
	const UINT transformCount = mergedTransform.size();

	for (int transformIndex = 0; transformIndex < transformCount; ++transformIndex) {
		if (!HasMesh(transformIndex)) {
			continue;
		}

		const FrameMeshInfo& modelMeshInfo = mFrameMeshInfo[transformIndex];
		const Transform* transform = mergedTransform[transformIndex];

		transform->UpdateShaderVars();

		UINT vertexCount = modelMeshInfo.VertexCount;
		UINT mat{ 0 };
		for (UINT indexCount : modelMeshInfo.IndexCounts) {
			modelMeshInfo.Materials[mat++]->UpdateShaderVars();

			cmdList->DrawIndexedInstanced(indexCount, instanceCount, indexLocation, vertexLocation, 0);
			indexLocation += indexCount;
		}

		vertexLocation += vertexCount;
	}
}
#pragma endregion

#pragma endregion
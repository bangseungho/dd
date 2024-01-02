#include "stdafx.h"
#include "MeshRenderer.h"

#include "Mesh.h"



uptr<ModelObjectMesh> MeshRenderer::mBoxMesh;
uptr<ModelObjectMesh> MeshRenderer::mSphereMesh;
uptr<ModelObjectMesh> MeshRenderer::mPlaneMesh;

void MeshRenderer::Render(const BoundingBox& box)
{
	const Vec3 center = box.Center;
	const Vec3 extents = box.Extents;

	const Matrix scaleMatrix = XMMatrixScaling(extents.x * 2, extents.y * 2, extents.z * 2);
	const Matrix translateMatrix = XMMatrixTranslation(center.x, center.y, center.z);

	const Matrix matrix = XMMatrixMultiply(scaleMatrix, translateMatrix);

	Transform::UpdateShaderVars(matrix);
	mBoxMesh->Render();
}

void MeshRenderer::Render(const BoundingOrientedBox& box)
{
	const Vec3 center = box.Center;
	const Vec3 extents = box.Extents;
	const Vec4 orientation = box.Orientation;
	const Vector quaternion = XMLoadFloat4(&orientation);

	const Matrix scaleMatrix = XMMatrixScaling(extents.x * 2, extents.y * 2, extents.z * 2);
	const Matrix rotationMatrix = XMMatrixRotationQuaternion(quaternion);
	const Matrix translateMatrix = XMMatrixTranslation(center.x, center.y, center.z);

	const Matrix matrix = XMMatrixMultiply(XMMatrixMultiply(scaleMatrix, rotationMatrix), translateMatrix);

	Transform::UpdateShaderVars(matrix);
	mBoxMesh->Render();
}

void MeshRenderer::Render(const BoundingSphere& bs)
{
	const Vec3 center = bs.Center;
	const float radius = bs.Radius;

	const Matrix scaleMatrix = XMMatrixScaling(radius / 2, radius / 2, radius / 2);
	const Matrix translateMatrix = XMMatrixTranslation(center.x, center.y, center.z);

	const Matrix matrix = XMMatrixMultiply(scaleMatrix, translateMatrix);

	Transform::UpdateShaderVars(matrix);
	mSphereMesh->Render();
}

void MeshRenderer::RenderPlane(const Vec3& pos, float width, float length)
{
	const Matrix scaleMatrix = XMMatrixScaling(width, 1, length);
	const Matrix translateMatrix = XMMatrixTranslation(pos.x, pos.y, pos.z);

	const Matrix matrix = XMMatrixMultiply(scaleMatrix, translateMatrix);

	Transform::UpdateShaderVars(matrix);
	mPlaneMesh->Render();
}


void MeshRenderer::BuildMeshes()
{
	mBoxMesh = std::make_unique<ModelObjectMesh>();
	mSphereMesh = std::make_unique<ModelObjectMesh>();
	mPlaneMesh = std::make_unique< ModelObjectMesh>();

	mBoxMesh->CreateCubeMesh(1.f, 1.f, 1.f, false, true);
	mSphereMesh->CreateSphereMesh(1.f, 12, true);
	mPlaneMesh->CreatePlaneMesh(1.f, 1.f, true);
}

void MeshRenderer::ReleaseUploadBuffers()
{
	mBoxMesh->ReleaseUploadBuffers();
	mSphereMesh->ReleaseUploadBuffers();
	mPlaneMesh->ReleaseUploadBuffers();
}

void MeshRenderer::Release()
{
	mBoxMesh = nullptr;
	mSphereMesh = nullptr;
	mPlaneMesh = nullptr;
}
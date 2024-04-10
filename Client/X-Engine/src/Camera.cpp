#include "EnginePch.h"
#include "Component/Camera.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Component/Collider.h"
#include "Scene.h"





#pragma region Camera
void Camera::Awake()
{
	float width = dxgi->GetWindowWidth();
	float height = dxgi->GetWindowHeight();

	mAspectRatio = width / height;

	mViewport    = { 0.f, 0.f, width, height, 0.f, 1.f };
	mScissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height)};
}

void Camera::UpdateViewMtx()
{
	const Vec3 kPos   = mObject->GetPosition();
	const Vec3 kRight = mObject->GetRight();
	const Vec3 kUp    = mObject->GetUp();
	const Vec3 kLook  = mObject->GetLook();

	mViewTransform._11 = kRight.x; mViewTransform._12 = kUp.x; mViewTransform._13 = kLook.x;
	mViewTransform._21 = kRight.y; mViewTransform._22 = kUp.y; mViewTransform._23 = kLook.y;
	mViewTransform._31 = kRight.z; mViewTransform._32 = kUp.z; mViewTransform._33 = kLook.z;
	mViewTransform._41 = -kPos.Dot(kRight);
	mViewTransform._42 = -kPos.Dot(kUp);
	mViewTransform._43 = -kPos.Dot(kLook);

	CalculateFrustumPlanes();
}
void Camera::SetProjMtx(float nearPlaneDistance, float farPlaneDistance, float fovAngle)
{
	const Matrix kProjMtx = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngle), mAspectRatio, nearPlaneDistance, farPlaneDistance);
	XMStoreFloat4x4(&mProjTransform, kProjMtx);

	BoundingFrustum::CreateFromMatrix(mFrustumView, kProjMtx);
}


void Camera::SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ, float maxZ)
{
	mViewport.TopLeftX = float(xTopLeft);
	mViewport.TopLeftY = float(yTopLeft);
	mViewport.Width    = float(width);
	mViewport.Height   = float(height);
	mViewport.MinDepth = minZ;
	mViewport.MaxDepth = maxZ;
}
void Camera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	mScissorRect.left   = xLeft;
	mScissorRect.top    = yTop;
	mScissorRect.right  = xRight;
	mScissorRect.bottom = yBottom;
}
void Camera::SetViewportsAndScissorRects()
{
	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);
}


void Camera::LookAt(const Vec3& lookAt, const Vec3& upVector)
{
	mObject->SetAxis(Matrix4x4::LookAtLH(mObject->GetPosition(), lookAt, upVector, true));
}

Vec2 Camera::WorldToScreenPoint(const Vec3& pos)
{
	// position을 클립 좌표 공간으로 변환
	XMVECTOR screenPoint = XMVector3TransformCoord(XMVector3TransformCoord(_VECTOR(pos), _MATRIX(mViewTransform)), _MATRIX(mProjTransform));

	// 클립 좌표를 NDC로 변환
	screenPoint /= XMVectorGetW(screenPoint);

	// NDC를 screen 좌표로 변환
	screenPoint = XMVectorMultiplyAdd(screenPoint, XMVectorSet(mViewport.Width, mViewport.Height, 0.0f, 0.0f), XMVectorSet(mViewport.Width * 0.5f, mViewport.Height * 0.5f, 0.0f, 0.0f));
	screenPoint = XMVectorSubtract(screenPoint, XMVectorSet(mViewport.Width * 0.5f, mViewport.Height * 0.5f, 0.f, 0.f));

	Vec3 result;
	XMStoreFloat3(&result, screenPoint);
	return Vec2(result.x, result.y);
}

void Camera::CalculateFrustumPlanes()
{
	mFrustumView.Transform(mFrustumWorld, XMMatrixInverse(nullptr, _MATRIX(mViewTransform)));
}
#pragma endregion





#pragma region CameraObject
CameraObject::CameraObject() : Object()
{
	mCamera = AddComponent<Camera>();
}


void CameraObject::Rotate(float pitch, float yaw, float roll)
{
	base::Rotate(pitch, yaw, roll);

	SetRightY(0.f);	// Roll 회전 x
}


void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}
#pragma endregion





#pragma region MainCameraObject
void MainCameraObject::Awake()
{
	base::Awake();

	SetPosition(1, 2, 3);	// must be non-zero
}
#pragma endregion
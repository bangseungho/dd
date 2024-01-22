#include "stdafx.h"
#include "SkyBox.h"

#include "Camera.h"
#include "Scene.h"

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


SkyBox::SkyBox() : Transform(this)
{
	mMesh = std::make_unique<ModelObjectMesh>();
	mMesh->CreateSkyBoxMesh(20, 20, 20);

	mTexture = std::make_unique<Texture>(D3DResource::TextureCube);
	mTexture->LoadTexture("Models/Skybox/", "SkyBox_0");

	mShader = std::make_unique<SkyBoxShader>();
	mShader->Create();

	//scene->CreateShaderResourceView(mTexture.get(), 0);
}

void SkyBox::Render()
{
	Vec3 pos = mainCameraObject->GetPosition();
	SetPosition(pos.x, pos.y, pos.z);
	
	//UpdateShaderVars();
	//mTexture->UpdateShaderVars();
	//mShader->Set();
	//mMesh->Render();
}
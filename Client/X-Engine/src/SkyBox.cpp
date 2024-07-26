#include "EnginePch.h"
#include "SkyBox.h"
#include "DXGIMgr.h"

#include "Component/Camera.h"

#include "ResourceMgr.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


SkyBox::SkyBox() : Transform(this)
{
	mMesh = std::make_shared<ModelObjectMesh>();
	mMesh->CreateSphereMesh(0.5f);
	mTexture = RESOURCE<Texture>("SkyBox_1");
}

void SkyBox::Render()
{
	UpdateShaderVars();
	mMesh->Render();
}
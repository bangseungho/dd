#include "Common.hlsl"

TextureCube gSkyBoxTexture : register(t2);

Texture2D gAlbedoTexture : register(t2);
Texture2D gTerrainTextureLayer0 : register(t2);
Texture2D gTerrainTextureLayer1 : register(t3);
Texture2D gTerrainTextureLayer2 : register(t4);
Texture2D gTerrainSplatMap : register(t5);

Texture2D gRT1_Texture : register(t6);
Texture2D gRT2_UI : register(t7);
Texture2D gRT3_Normal : register(t8);
Texture2D gRT4_Distance : register(t9);
Texture2D gRT5_Depth : register(t10);

SamplerState gSamplerState : register(s0);
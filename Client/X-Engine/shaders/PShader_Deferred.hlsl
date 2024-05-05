#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

struct PSOutput_MRT {
    float4 Position           : SV_TARGET0;
    float4 Normal             : SV_TARGET1;
    float4 Diffuse            : SV_TARGET2;
    float4 Emissive           : SV_TARGET3;
    float2 MetallicSmoothness : SV_TARGET4;
    float1 Occlusion          : SV_TARGET5;
};

PSOutput_MRT PSDeferred(VSOutput_Standard pin)
{
    // material info
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse        = matInfo.Diffuse;
    float metallic        = matInfo.Metallic;
    float roughness       = matInfo.Roughness;
    float occlusion       = 1.f;
    int diffuseMapIndex   = matInfo.DiffuseMap0Index;
    int normalMapIndex    = matInfo.NormalMapIndex;
    int metallicMapIndex  = matInfo.MetallicMapIndex;
    int emissiveMapIndex  = matInfo.EmissiveMapIndex;
    int occlusionMapIndex = matInfo.OcclusionMapIndex;
    
    // sampling diffuseMap
    if (diffuseMapIndex != -1)
    {
        diffuse *= GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
    }

    // normalize normal
    pin.NormalW = normalize(pin.NormalW);
    float3 bumpedNormalW = pin.NormalW;
    
    // sampling normalMap, to world space
    float4 normalMapSample = (float4)0;
    if (normalMapIndex != -1)
    {
        normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }
    
    // sampling emissiveMap
    float4 emissiveMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        emissiveMapSample = gTextureMaps[emissiveMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }
    
    // sampling metallicMap
    float4 metallicMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        metallicMapSample = GammaDecoding(gTextureMaps[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        metallic = metallicMapSample.r;
        roughness = 1 - metallicMapSample.a;
    }
    
    if (occlusionMapIndex != -1)
    {
        occlusion = (float)GammaDecoding(gTextureMaps[occlusionMapIndex].Sample(gsamAnisotropicWrap, pin.UV).x);
    }
    
    float4 rimLight = ComputeRimLight(float4(1.f, 0.f, 0.f, 0.f), 0.6f, gObjectCB.RimFactor, pin.PosW, bumpedNormalW);
    
    if (matInfo.UseSphereMask == 1)
    {
        float3 pinPosV = mul(float4(pin.PosW, 1.f), gPassCB.MtxView);
        float3 camPosV = mul(float4(gPassCB.CameraPos, 1.f), gPassCB.MtxView);
        
        float dissolve = gTextureMaps[44].Sample(gsamAnisotropicWrap, pin.UV).x;
        
        float sphereMaskRadius = 2.f;
        float dist = distance(pinPosV.xy, camPosV.xy);
        float yDist = abs(gPassCB.CameraPos.y - pin.PosW.y);
        
        float sphereMask = clamp(dist / sphereMaskRadius, 0.f, 1.f);

        
        float color1 = 1.f;
        float color2 = 0.f;
        float litSphereMask = lerp(color1, color2, sphereMask);
        
        if (yDist < 9.f)
            clip(dissolve + sphereMask - 0.99f);
    }
    
    PSOutput_MRT pout;
    pout.Position = float4(pin.PosW, 0.f);
    pout.Normal = float4(bumpedNormalW, 0.f);
    pout.Diffuse = diffuse;
    pout.Emissive = emissiveMapSample + rimLight;
    pout.MetallicSmoothness = float2(metallic, roughness);
    pout.Occlusion = occlusion;
    
    return pout;
}

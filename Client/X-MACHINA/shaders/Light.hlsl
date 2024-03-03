#include "Common.hlsl"

// PBR 관련 함수들로 내부 구현은 알 필요 없다.
namespace PBR
{
    float3 Fresnel(in float3 specAlbedo, in float3 h, in float3 l)
    {
        float3 fresnel = specAlbedo + (1.0f - specAlbedo) * pow((1.0f - saturate(dot(l, h))), 5.0f);

        // Fade out spec entirely when lower than 0.1% albedo
        fresnel *= saturate(dot(specAlbedo, 333.0f));

        return fresnel;
    }

//-------------------------------------------------------------------------------------------------
// Calculates the Fresnel factor using Schlick's approximation
//-------------------------------------------------------------------------------------------------
    float3 Fresnel(in float3 specAlbedo, in float3 fresnelAlbedo, in float3 h, in float3 l)
    {
        float3 fresnel = specAlbedo + (fresnelAlbedo - specAlbedo) * pow((1.0f - saturate(dot(l, h))), 5.0f);

    // Fade out spec entirely when lower than 0.1% albedo
        fresnel *= saturate(dot(specAlbedo, 333.0f));

        return fresnel;
    }

//-------------------------------------------------------------------------------------------------
// Helper for computing the Beckmann geometry term
//-------------------------------------------------------------------------------------------------
    float BeckmannG1(float m, float nDotX)
    {
        float nDotX2 = nDotX * nDotX;
        float tanTheta = sqrt((1 - nDotX2) / nDotX2);
        float a = 1.0f / (m * tanTheta);
        float a2 = a * a;

        float g = 1.0f;
        if (a < 1.6f)
            g *= (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2);

        return g;
    }

//-------------------------------------------------------------------------------------------------
// Computes the specular term using a Beckmann microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
//-------------------------------------------------------------------------------------------------
    float BeckmannSpecular(in float m, in float3 n, in float3 h, in float3 v, in float3 l)
    {
        float nDotH = max(dot(n, h), 0.0001f);
        float nDotL = saturate(dot(n, l));
        float nDotV = max(dot(n, v), 0.0001f);

        float nDotH2 = nDotH * nDotH;
        float nDotH4 = nDotH2 * nDotH2;
        float m2 = m * m;

    // Calculate the distribution term
        float tanTheta2 = (1 - nDotH2) / nDotH2;
        float expTerm = exp(-tanTheta2 / m2);
        float d = expTerm / (3.141592 * m2 * nDotH4);

    // Calculate the matching geometric term
        float g1i = BeckmannG1(m, nDotL);
        float g1o = BeckmannG1(m, nDotV);
        float g = g1i * g1o;

        return d * g * (1.0f / (4.0f * nDotL * nDotV));
    }


//-------------------------------------------------------------------------------------------------
// Helper for computing the GGX visibility term
//-------------------------------------------------------------------------------------------------
    float GGXV1(in float m2, in float nDotX)
    {
        return 1.0f / (nDotX + sqrt(m2 + (1 - m2) * nDotX * nDotX));
    }

//-------------------------------------------------------------------------------------------------
// Computes the GGX visibility term
//-------------------------------------------------------------------------------------------------
    float GGXVisibility(in float m2, in float nDotL, in float nDotV)
    {
        return GGXV1(m2, nDotL) * GGXV1(m2, nDotV);
    }

    float SmithGGXMasking(float3 n, float3 l, float3 v, float a2)
    {
        float dotNL = saturate(dot(n, l));
        float dotNV = saturate(dot(n, v));
        float denomC = sqrt(a2 + (1.0f - a2) * dotNV * dotNV) + dotNV;

        return 2.0f * dotNV / denomC;
    }

    float SmithGGXMaskingShadowing(float3 n, float3 l, float3 v, float a2)
    {
        float dotNL = saturate(dot(n, l));
        float dotNV = saturate(dot(n, v));

        float denomA = dotNV * sqrt(a2 + (1.0f - a2) * dotNL * dotNL);
        float denomB = dotNL * sqrt(a2 + (1.0f - a2) * dotNV * dotNV);

        return 2.0f * dotNL * dotNV / (denomA + denomB);
    }

//-------------------------------------------------------------------------------------------------
// Computes the specular term using a GGX microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
//-------------------------------------------------------------------------------------------------
    float GGXSpecular(in float m, in float3 n, in float3 h, in float3 v, in float3 l)
    {
        float nDotH = saturate(dot(n, h));
        float nDotL = saturate(dot(n, l));
        float nDotV = saturate(dot(n, v));

        float nDotH2 = nDotH * nDotH;
        float m2 = m * m;

    // Calculate the distribution term
        float x = nDotH * nDotH * (m2 - 1) + 1;
        float d = m2 / (3.141592 * pow(x, 2));

    // Calculate the matching visibility term
        float vis = GGXVisibility(m2, nDotL, nDotV);

        return d * vis;
    }

    float GGXSpecularAnisotropic(float m, float3 n, float3 h, float3 v, float3 l, float3 tx, float3 ty, float anisotropy)
    {
        float nDotH = saturate(dot(n, h));
        float nDotL = saturate(dot(n, l));
        float nDotV = saturate(dot(n, v));
        float nDotH2 = nDotH * nDotH;

        float ax = m;
        float ay = lerp(ax, 1.0f, anisotropy);
        float ax2 = ax * ax;
        float ay2 = ay * ay;

        float xDotH = dot(tx, h);
        float yDotH = dot(ty, h);
        float xDotH2 = xDotH * xDotH;
        float yDotH2 = yDotH * yDotH;

    // Calculate the distribution term
        float denom = (xDotH2 / ax2) + (yDotH2 / ay2) + nDotH2;
        denom *= denom;
        float d = (1.0f / (3.141592 * ax * ay)) * 1.0f / denom;

    // Calculate the matching visibility term
        float vis = GGXVisibility(m * m, nDotL, nDotV);

        return d * vis;
    }

// Distribution term for the velvet BRDF
    float VelvetDistribution(in float m, in float nDotH2, in float offset)
    {
        m = 0.25f + 0.75f * m;
        float cot2 = nDotH2 / (1.000001f - nDotH2);
        float sin2 = 1.0f - nDotH2;
        float sin4 = sin2 * sin2;
        float amp = 4.0f;
        float m2 = m * m + 0.000001f;
        float cnorm = 1.0f / (3.141592 * (offset + amp * m2));

        return cnorm * (offset + (amp * exp(-cot2 / (m2 + 0.000001f)) / sin4));
    }

// Specular term for the velvet BRDF
    float VelvetSpecular(in float m, in float3 n, in float3 h, in float3 v, in float3 l, in float offset)
    {
        float nDotH = saturate(dot(n, h));
        float nDotH2 = nDotH * nDotH;
        float nDotV = saturate(dot(n, v));
        float nDotL = saturate(dot(n, l));

        float D = VelvetDistribution(m, nDotH2, offset);
        float G = 1.0f;
        float denom = 1.0f / (4.0f * (nDotL + nDotV - nDotL * nDotV));
        return D * G * denom;
    }

//-------------------------------------------------------------------------------------------------
// Returns scale and bias values for environment specular reflections that represents the
// integral of the geometry/visibility + fresnel terms for a GGX BRDF given a particular
// viewing angle and roughness value. The final value is computed using polynomials that were
// fitted to tabulated data generated via monte carlo integration.
//-------------------------------------------------------------------------------------------------
    float2 GGXEnvironmentBRDFScaleBias(in float nDotV, in float sqrtRoughness)
    {
        const float nDotV2 = nDotV * nDotV;
        const float sqrtRoughness2 = sqrtRoughness * sqrtRoughness;
        const float sqrtRoughness3 = sqrtRoughness2 * sqrtRoughness;

        const float delta = 0.991086418474895f + (0.412367709802119f * sqrtRoughness * nDotV2) -
                        (0.363848256078895f * sqrtRoughness2) -
                        (0.758634385642633f * nDotV * sqrtRoughness2);
        const float bias = saturate((0.0306613448029984f * sqrtRoughness) + 0.0238299731830387f /
                                (0.0272458171384516f + sqrtRoughness3 + nDotV2) -
                                0.0454747751719356f);

        const float scale = saturate(delta - bias);
        return float2(scale, bias);
    }

//-------------------------------------------------------------------------------------------------
// Returns an adjusted scale factor for environment specular reflections that represents the
// integral of the geometry/visibility + fresnel terms for a GGX BRDF given a particular
// viewing angle and roughness value. The final value is computed using polynomials that were
// fitted to tabulated data generated via monte carlo integration.
//-------------------------------------------------------------------------------------------------
    float3 GGXEnvironmentBRDF(in float3 specAlbedo, in float nDotV, in float sqrtRoughness)
    {
        float2 scaleBias = GGXEnvironmentBRDFScaleBias(nDotV, sqrtRoughness);
        return specAlbedo * scaleBias.x + scaleBias.y;
    }
}


// 선형 감쇠 함수
float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

LightColor BRDF(float3 normal, float3 lightDir, float3 lightStrength, Material mat, float3 toCameraW)
{
    float3 msEnergyCompensation = 1.0.xxx;
    float2 DFG = PBR::GGXEnvironmentBRDFScaleBias(saturate(dot(normal, toCameraW)), mat.Roughness * mat.Roughness);
    float Ess = DFG.x;
    msEnergyCompensation = 1.0.xxx + mat.SpecularAlbedo * (1.0f / Ess - 1.0f);
    
    float3 h = normalize(toCameraW + lightDir);
    
    // 슐릭 근사 방정식을 사용하여 프레넬 값(얼마나 반사하는지)을 구한다.
    float3 fresnel = PBR::Fresnel(mat.SpecularAlbedo, h, lightDir);
    // GGXSpecular 모델을 사용하여 정반사 값을 구한다.
    float specular = PBR::GGXSpecular(clamp(pow(mat.Roughness, 2), 0.1f, 1.f), normal, h, toCameraW, lightDir);
    
    LightColor result;
    result.Diffuse = mat.DiffuseAlbedo * lightStrength;
    result.Specular = specular * fresnel * lightStrength * msEnergyCompensation;
    
    return result;
}

LightColor ComputeDirectionalLight(LightInfo L, Material mat, float3 pos, float3 normal, float3 toCameraW, float shadowFactor)
{
    // 빛이 나아가는 방향의 반대 방향
    float3 lightVec = -L.Direction;
    
    // Lambert를 half Lambert로 변경
    float ndotl = saturate(pow(dot(lightVec, normal) * 0.5f + 0.5f, 4));
    float3 lightStrength = L.Strength * ndotl;
    
    LightColor result = BRDF(normal, lightVec, lightStrength, mat, toCameraW);
    result.Diffuse  *= shadowFactor;
    result.Specular *= shadowFactor;
    
    return result;
}

LightColor ComputePointLight(LightInfo L, Material mat, float3 pos, float3 normal, float3 toCameraW)
{
    // 표면에서 광원으로의 벡터
    float3 lightVec = L.Position - pos;

    // 광원과 표면 사이의 거리
    float distance = length(lightVec);
    
    // 빛 벡터 정규화
    lightVec /= distance;
    
    // Lambert를 half Lambert로 변경
    float ndotl = saturate(dot(lightVec, normal) * 0.5f + 0.5f);
    float3 lightStrength = L.Strength * ndotl;

    // 거리에 따라 빛을 감쇠한다.
    float att = CalcAttenuation(distance, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;
    
    return BRDF(normal, lightVec, lightStrength, mat, toCameraW);
}

LightColor ComputeSpotLight(LightInfo L, Material mat, float3 pos, float3 normal, float3 toCameraW)
{
    // 표면에서 광원으로의 벡터
    float3 lightVec = L.Position - pos;

    // 광원과 표면 사이의 거리
    float distance = length(lightVec);

    // 빛 벡터 정규화
    lightVec /= distance;
    
    // 람베르트 코사인 법칙에 따라 빛의 세기를 줄인다.
    float ndotl = saturate(dot(lightVec, normal) * 0.5f + 0.5f);
    float3 lightStrength = L.Strength * ndotl;

    // 거리에 따라 빛을 감쇠한다.
    float att = CalcAttenuation(distance, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;
    
    // Scale by spotlight
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
    lightStrength *= spotFactor;
    
    return BRDF(normal, lightVec, lightStrength, mat, toCameraW);
}

LightColor ComputeLighting(Material mat, float3 pos, float3 normal, float3 toCameraW, float3 shadowFactor)
{
    LightColor result = (LightColor)0.f;
    
    for (uint i = 0; i < gPassCB.LightCount; ++i)
    {
        if (gPassCB.Lights[i].LightType == 0)
        {
            LightColor color = ComputeSpotLight(gPassCB.Lights[i], mat, pos, normal, toCameraW);
            result.Diffuse += color.Diffuse;
            result.Specular += color.Specular;
        }
        else if (gPassCB.Lights[i].LightType == 1)
        {
            LightColor color = ComputeDirectionalLight(gPassCB.Lights[i], mat, pos, normal, toCameraW, shadowFactor[i]);
            result.Diffuse += color.Diffuse;
            result.Specular += color.Specular;
        }
        else if (gPassCB.Lights[i].LightType == 2)
        {
            LightColor color = ComputePointLight(gPassCB.Lights[i], mat, pos, normal, toCameraW);
            result.Diffuse += color.Diffuse;
            result.Specular += saturate(color.Specular);
        }
    }

    return result;
}





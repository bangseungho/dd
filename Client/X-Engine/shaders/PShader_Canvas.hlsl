#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    float4 color;
    // 머티리얼을 사용하지 않는 경우 MaterialIndex에 바로 텍스처 인덱스를 Set할 것
    color = gTextureMaps[gObjectCB.MatIndex].Sample(gsamLinearWrap, input.UV);

    if (gObjectCB.UseOutline)
    {
        color.rgb = color.a * float4(gObjectCB.HitRimColor, 1.f);
    }
    color.a *= gObjectCB.AlphaIntensity;
    
    if (input.UV.x > gObjectCB.SliderValueX)
    {
        discard;
    }
    
    if ((1 - input.UV.y) > gObjectCB.SliderValueY)
    {
        discard;
    }

    return color;
}

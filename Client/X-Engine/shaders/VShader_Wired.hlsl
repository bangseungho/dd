#include "Common.hlsl"

struct VSInput {
    float3 PosL : POSITION;
};

struct VSOutput {
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};


VSOutput VSWired(VSInput input)
{
    VSOutput output;

    output.PosH = mul(mul(mul(float4(input.PosL, 1.f), gColliderCB.MtxView), gPassCB.MtxView), gPassCB.MtxProj);
    output.Color = float4(1.f, 0.f, 0.f, 1.f);

    return output;
}
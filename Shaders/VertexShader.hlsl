#include "RootSignature.hlsl"

[RootSignature(ROOTSIG)]
float4 main( float2 pos : POSITION ) : SV_POSITION
{
    return float4(pos.xy, 0.0f, 1.0f);
}
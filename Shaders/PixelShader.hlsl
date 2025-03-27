#include "RootSignature.hlsl"

float3 Color : register(b0);

[RootSignature(ROOTSIG)]
float4 main() : SV_TARGET
{
	return float4(Color, 1.0f);
}
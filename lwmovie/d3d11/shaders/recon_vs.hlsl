#include "common_h.hlsl"

ReconPSInput mainVS( float4 pos : POSITION, float2 texCoord : TEXCOORD0 )
{
	ReconPSInput psInput;
	psInput.pos = pos;
	psInput.texCoord = texCoord;
	return psInput;
}

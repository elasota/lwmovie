struct ReconPSInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct ReconPSOutputChroma
{
	float4 outputU : SV_TARGET0;
	float4 outputV : SV_TARGET1;
};

struct ReconConstants
{
	float2 fDimensionsL;
	float2 fDimensionsC;
	uint2 mbDimensions;
};
struct DCTBlockV
{
	int4 packedCoeffs[8];
};
struct DCTBlock
{
	int packedCoeffs[32];
};
struct ReconMBlockInfo
{
	int4 motionVectors;
	uint4 flags;
};

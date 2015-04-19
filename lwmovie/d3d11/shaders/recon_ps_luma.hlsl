#include "common_h.hlsl"
#include "commondefs.h"

StructuredBuffer<DCTBlock> myDCTBlocks : register(t0);
StructuredBuffer<ReconMBlockInfo> myMBlockInfos : register(t1);
Texture2D myForwPredLuma : register(t2);
Texture2D myBackPredLuma : register(t3);

SamplerState mySampler : register(s0);
ReconConstants myConstants : register(b0);

float4 mainPS( ReconPSInput psInput ) : SV_Target
{
	uint2 xyCoords = uint2(round(psInput.texCoord.xy * myConstants.fDimensionsL - float2(0.5, 0.5)));
	uint2 mbCoords = xyCoords / 16;
	uint2 subMBCoords = xyCoords % 16;
	uint2 subMBBlockCoords = subMBCoords / 8;
	uint2 subBlockCoords = subMBCoords % 8;
	uint mbAddr = mbCoords.y * myConstants.mbDimensions.x + mbCoords.x;
	uint subBlockAddr = (subMBBlockCoords.y * 2) + subMBBlockCoords.x;
	uint dctBlockAddr = mbAddr * 6 + subBlockAddr;
	uint dctCoeffAddr = subBlockCoords.y * 8 + subBlockCoords.x;
	uint dctPackedCoeffAddr = dctCoeffAddr / 2;
	int packedCoeff;
	if((dctCoeffAddr & 1) == 0)
	{
		int packedCoeffInter = (myDCTBlocks[dctBlockAddr].packedCoeffs[dctPackedCoeffAddr] << 16) & 0xffff0000;
		packedCoeff = packedCoeffInter >> 16;
	}
	else
		packedCoeff = myDCTBlocks[dctBlockAddr].packedCoeffs[dctPackedCoeffAddr] >> 16;

	float residual = float(packedCoeff) / 255.0;
	float forwPred = 0.0;
	float backPred = 0.0;
	uint mblockFlags = myMBlockInfos[mbAddr].flags.x;
	int4 motionVectors = myMBlockInfos[mbAddr].motionVectors;

	float2 forwCoords = psInput.texCoord.xy;
	float2 backCoords;
	if(mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_FORW_MOTION)
	{
		forwCoords = float2(motionVectors.xy) * 0.5 / myConstants.fDimensionsL + psInput.texCoord.xy;
		forwPred = myForwPredLuma.Sample(mySampler, forwCoords).r;
	}
	if(mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_BACK_MOTION)
	{
		backCoords = float2(motionVectors.zw) * 0.5 / myConstants.fDimensionsL + psInput.texCoord.xy;
		backPred = myBackPredLuma.Sample(mySampler, backCoords).r;
	}
	if((mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_BOTH_MOTION) == LWMOVIE_D3D11_BLOCK_FLAG_BOTH_MOTION)
	{
		forwPred *= 0.5;
		backPred *= 0.5;
	}

	float finalValue = forwPred + backPred + residual;

	return float4(finalValue, finalValue, finalValue, 1.0);
}

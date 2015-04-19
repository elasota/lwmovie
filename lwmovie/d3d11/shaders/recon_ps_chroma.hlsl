#include "common_h.hlsl"
#include "commondefs.h"

StructuredBuffer<DCTBlock> myDCTBlocks : register(t0);
StructuredBuffer<ReconMBlockInfo> myMBlockInfos : register(t1);
Texture2D myForwPredU : register(t2);
Texture2D myForwPredV : register(t3);
Texture2D myBackPredU : register(t4);
Texture2D myBackPredV : register(t5);

SamplerState mySampler : register(s0);
ReconConstants myConstants : register(b0);

struct ReconPSChromaOut
{
	float4 u : SV_TARGET0;
	float4 v : SV_TARGET1;
};

ReconPSChromaOut mainPS( ReconPSInput psInput )
{
	uint2 xyCoords = uint2(round(psInput.texCoord.xy * myConstants.fDimensionsC - float2(0.5, 0.5)));
	uint2 mbCoords = xyCoords / 8;
	uint mbAddr = mbCoords.y * myConstants.mbDimensions.x + mbCoords.x;

	uint2 subBlockCoords = xyCoords % 8;
	uint dctBlockAddr = mbAddr * 6;
	uint dctCoeffAddr = subBlockCoords.y * 8 + subBlockCoords.x;

	uint dctPackedCoeffAddr = dctCoeffAddr / 2;
	int2 packedCoeffs;
	packedCoeffs.x = myDCTBlocks[dctBlockAddr+4].packedCoeffs[dctPackedCoeffAddr];
	packedCoeffs.y = myDCTBlocks[dctBlockAddr+5].packedCoeffs[dctPackedCoeffAddr];
	if((dctCoeffAddr & 1) == 0)
		packedCoeffs = ((packedCoeffs + 0x8000) & 0xffff) - 0x8000;
	else
		packedCoeffs = packedCoeffs >> 16;

	float2 residual = float2(packedCoeffs) / 255.0;
	float2 forwPred = float2(0.0, 0.0);
	float2 backPred = float2(0.0, 0.0);
	uint mblockFlags = myMBlockInfos[mbAddr].flags.x;
	int4 motionVectors = myMBlockInfos[mbAddr].motionVectors;

	// Round motion vectors towards zero, to a multiple of 2
	int4 motionVectorsRounder = motionVectors >> 31;
	motionVectors -= motionVectorsRounder;

	if(mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_FORW_MOTION)
	{
		float2 forwCoords = float2(motionVectors.xy) * 0.25 / myConstants.fDimensionsC + psInput.texCoord.xy;
		forwPred.x = myForwPredU.Sample(mySampler, forwCoords).r;
		forwPred.y = myForwPredV.Sample(mySampler, forwCoords).r;
	}
	if(mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_BACK_MOTION)
	{
		float2 backCoords = float2(motionVectors.zw) * 0.25 / myConstants.fDimensionsC + psInput.texCoord.xy;
		backPred.x = myBackPredU.Sample(mySampler, backCoords).r;
		backPred.y = myBackPredV.Sample(mySampler, backCoords).r;
	}
	if((mblockFlags & LWMOVIE_D3D11_BLOCK_FLAG_BOTH_MOTION) == LWMOVIE_D3D11_BLOCK_FLAG_BOTH_MOTION)
	{
		forwPred *= 0.5;
		backPred *= 0.5;
	}

	float2 finalValue = forwPred + backPred + residual;

	ReconPSChromaOut psOut;
	psOut.u = float4(finalValue.xxx, 1.0);
	psOut.v = float4(finalValue.yyy, 1.0);

	return psOut;
}

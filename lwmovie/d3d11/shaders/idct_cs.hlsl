#include "common_h.hlsl"
#include "commondefs.h"

#define FIX_0_298631336  2446
#define FIX_0_390180644  3196
#define FIX_0_541196100  4433
#define FIX_0_765366865  6270
#define FIX_0_899976223  7373
#define FIX_1_175875602  9633
#define FIX_1_501321110  12299
#define FIX_1_847759065  15137
#define FIX_1_961570560  16069
#define FIX_2_053119869  16819
#define FIX_2_562915447  20995
#define FIX_3_072711026  25172
#define CONST_BITS 13
#define DESCALE(x, n)  (((x)+(1 << ((n)-1))) >> (n))

StructuredBuffer<DCTBlock> myInBlocks : register(t0);
StructuredBuffer<ReconMBlockInfo> myMBlockInfos : register(t1);
RWStructuredBuffer<DCTBlock> myOutBlocks : register(u0);

void idct8(inout int c0, inout int c1, inout int c2, inout int c3, inout int c4, inout int c5, inout int c6, inout int c7)
{
	int z2 = c2;
	int z3 = c6;
	int z1 = (z2 + z3) * FIX_0_541196100;
	int tmp2 = z1 + z3 * (-FIX_1_847759065);
	int tmp3 = z1 + z2 * FIX_0_765366865;
	int tmp0 = (c0 + c4) << CONST_BITS;
	int tmp1 = (c0 - c4) << CONST_BITS;
	int tmp10 = tmp0 + tmp3;
	int tmp13 = tmp0 - tmp3;
	int tmp11 = tmp1 + tmp2;
	int tmp12 = tmp1 - tmp2;
	tmp0 = c7;
	tmp1 = c5;
	tmp2 = c3;
	tmp3 = c1;
	z1 = tmp0 + tmp3;
	z2 = tmp1 + tmp2;
	z3 = tmp0 + tmp2;
	int z4 = tmp1 + tmp3;
	int z5 = (z3 + z4) * FIX_1_175875602;
	tmp0 = tmp0 * FIX_0_298631336;
	tmp1 = tmp1 * FIX_2_053119869;
	tmp2 = tmp2 * FIX_3_072711026;
	tmp3 = tmp3 * FIX_1_501321110;
	z1 = z1 * (-FIX_0_899976223);
	z2 = z2 * (-FIX_2_562915447);
	z3 = z3 * (-FIX_1_961570560);
	z4 = z4 * (-FIX_0_390180644);
	z3 += z5;
	z4 += z5;
	tmp0 += z1 + z3;
	tmp1 += z2 + z4;
	tmp2 += z2 + z3;
	tmp3 += z1 + z4;
	c0 = tmp10 + tmp3;
	c7 = tmp10 - tmp3;
	c1 = tmp11 + tmp2;
	c6 = tmp11 - tmp2;
	c2 = tmp12 + tmp1;
	c5 = tmp12 - tmp1;
	c3 = tmp13 + tmp0;
	c4 = tmp13 - tmp0;
}
[numthreads(LWMOVIE_D3D11_IDCT_NUMTHREADS,1,1)]
void mainCS( uint3 dtID : SV_DispatchThreadID )
{
	uint mbAddr = dtID.x / 6;
	uint sbAddr = dtID.x % 6;
	if(myMBlockInfos[mbAddr].flags.x & (1 << sbAddr))
	{
		// Zero block
		for (int outCel=0;outCel<32;outCel++)
			myOutBlocks[dtID.x].packedCoeffs[outCel] = 0;
	}
	else
	{
		// DCT block
		int gridCoeffs[64];
		for (int i=0;i<8;i++)
		{
			int rowCoeffs[8];
			for (int col=0;col<4;col++)
			{
				int packedCoeff = myInBlocks[dtID.x].packedCoeffs[i*4+col];
				rowCoeffs[col*2+0] = ((packedCoeff + 0x8000) & 0xffff) - 0x8000;
				rowCoeffs[col*2+1] = packedCoeff >> 16;
			}
			idct8(rowCoeffs[0], rowCoeffs[1], rowCoeffs[2], rowCoeffs[3], rowCoeffs[4], rowCoeffs[5], rowCoeffs[6], rowCoeffs[7]);
			for (int c=0;c<8;c++)
				gridCoeffs[i*8+c] = DESCALE(rowCoeffs[c], 12);
		}
		for (int idctCol=0;idctCol<8;idctCol++)
			idct8(gridCoeffs[idctCol+0], gridCoeffs[idctCol+8], gridCoeffs[idctCol+16], gridCoeffs[idctCol+24], gridCoeffs[idctCol+32], gridCoeffs[idctCol+40], gridCoeffs[idctCol+48], gridCoeffs[idctCol+56]);
		for (int outCel=0;outCel<32;outCel++)
		{
			int coeff0 = DESCALE(gridCoeffs[outCel*2+0], 17);
			int coeff1 = DESCALE(gridCoeffs[outCel*2+1], 17);
			int packedCoeff = (coeff0 & 0xffff) | (coeff1 << 16);
			myOutBlocks[dtID.x].packedCoeffs[outCel] = packedCoeff;
		}
	}
}

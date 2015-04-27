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

StructuredBuffer<DCTBlockV> myInBlocks : register(t0);
StructuredBuffer<ReconMBlockInfo> myMBlockInfos : register(t1);
RWStructuredBuffer<DCTBlockV> myOutBlocks : register(u0);

void idct8_row(inout int4 c0123, inout int4 c4567)
{
	int4 tmp, tmp1;
	tmp = c0123.xxzz * int4((1<<CONST_BITS), (1<<CONST_BITS), FIX_0_541196100, (FIX_0_541196100+FIX_0_765366865)) +
		c4567.xxzz * int4((1<<CONST_BITS), -(1<<CONST_BITS), (FIX_0_541196100-FIX_1_847759065), FIX_0_541196100);
	
	tmp1.xy = tmp.xy + tmp.wz;
	tmp1.zw = tmp.yx - tmp.zw;
	tmp.xy = c4567.wy;
	tmp.zw = c0123.wy;
	
	int4 z = tmp.xyxy + tmp.wzzw;
	int z5 = (z.z + z.w) * FIX_1_175875602;

	tmp *= int4(FIX_0_298631336, FIX_2_053119869, FIX_3_072711026, FIX_1_501321110);
	z *= int4(-FIX_0_899976223, -FIX_2_562915447, -FIX_1_961570560, -FIX_0_390180644);
	z.zw += int2(z5, z5);
	tmp += z.xyyx + z.zwzw;
	c0123 = tmp1.xyzw + tmp.wzyx;
	c4567 = tmp1.wzyx - tmp.xyzw;
}

void idct8_col(inout int4 c0, inout int4 c1, inout int4 c2, inout int4 c3, inout int4 c4, inout int4 c5, inout int4 c6, inout int4 c7)
{
	int4 z2 = c2;
	int4 z3 = c6;
	int4 z1 = (z2 + z3) * FIX_0_541196100;
	int4 tmp2 = z1 + z3 * (-FIX_1_847759065);
	int4 tmp3 = z1 + z2 * FIX_0_765366865;
	int4 tmp0 = (c0 + c4) << CONST_BITS;
	int4 tmp1 = (c0 - c4) << CONST_BITS;
	int4 tmp10 = tmp0 + tmp3;
	int4 tmp13 = tmp0 - tmp3;
	int4 tmp11 = tmp1 + tmp2;
	int4 tmp12 = tmp1 - tmp2;
	tmp0 = c7;
	tmp1 = c5;
	tmp2 = c3;
	tmp3 = c1;
	z1 = tmp0 + tmp3;
	z2 = tmp1 + tmp2;
	z3 = tmp0 + tmp2;
	int4 z4 = tmp1 + tmp3;
	int4 z5 = (z3 + z4) * FIX_1_175875602;
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

void UnpackCoeffs(int4 packedCoeff, out int4 outCoeffsLeft, out int4 outCoeffsRight)
{
	int4 unpacked0 = ((packedCoeff + 0x8000) & 0xffff) - 0x8000;
	int4 unpacked1 = packedCoeff >> 16;
	outCoeffsLeft.xz = unpacked0.xy;
	outCoeffsLeft.yw = unpacked1.xy;
	outCoeffsRight.xz = unpacked0.zw;
	outCoeffsRight.yw = unpacked1.zw;
}

[numthreads(LWMOVIE_D3D11_IDCT_NUMTHREADS,1,1)]
void mainCS( uint3 dtID : SV_DispatchThreadID )
{
	uint mbAddr = dtID.x / 6;
	uint sbAddr = dtID.x % 6;
	if(myMBlockInfos[mbAddr].flags.x & (1 << sbAddr))
	{
		// Zero block
		for (int outCel=0;outCel<8;outCel++)
			myOutBlocks[dtID.x].packedCoeffs[outCel] = int4(0,0,0,0);
	}
	else
	{
		// DCT block
		int4 gridCoeffsLeft[8];
		int4 gridCoeffsRight[8];
		for (int i=0;i<8;i++)
		{
			int4 rowCoeffsLeft, rowCoeffsRight;
			UnpackCoeffs(myInBlocks[dtID.x].packedCoeffs[i], rowCoeffsLeft, rowCoeffsRight);
			
			idct8_row(rowCoeffsLeft, rowCoeffsRight);
			
			gridCoeffsLeft[i] = DESCALE(rowCoeffsLeft, 12);
			gridCoeffsRight[i] = DESCALE(rowCoeffsRight, 12);
		}

		idct8_col(gridCoeffsLeft[0], gridCoeffsLeft[1], gridCoeffsLeft[2], gridCoeffsLeft[3], gridCoeffsLeft[4], gridCoeffsLeft[5], gridCoeffsLeft[6], gridCoeffsLeft[7]);
		idct8_col(gridCoeffsRight[0], gridCoeffsRight[1], gridCoeffsRight[2], gridCoeffsRight[3], gridCoeffsRight[4], gridCoeffsRight[5], gridCoeffsRight[6], gridCoeffsRight[7]);

		for (int outCel=0;outCel<8;outCel++)
		{
			int4 coeff0 = int4(gridCoeffsLeft[outCel].xz, gridCoeffsRight[outCel].xz);
			int4 coeff1 = int4(gridCoeffsLeft[outCel].yw, gridCoeffsRight[outCel].yw);
			coeff0 = DESCALE(coeff0, 17);
			coeff1 = DESCALE(coeff1, 17);

			int4 packedCoeff = (coeff0 & 0xffff) | (coeff1 << 16);
			myOutBlocks[dtID.x].packedCoeffs[outCel] = packedCoeff;
		}
	}
}

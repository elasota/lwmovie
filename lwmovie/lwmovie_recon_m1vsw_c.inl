/*
 * Copyright (c) 2015 Eric Lasota
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "../common/lwmovie_config.h"

#ifndef LWMOVIE_NOSIMD
#error "Don't include this file unless using LWMOVIE_NOSIMD"
#endif

#include <string.h>

#include "lwmovie_recon_m1vsw.hpp"
#include "lwmovie_profile.hpp"
#include "lwmovie_idct.hpp"

namespace lwmovie
{
	namespace m1v
	{
		namespace reconutil
		{
			inline void Average16Bytes(lwmUInt8 *output, const lwmUInt8 *input1, const lwmUInt8 *input2)
			{
				for (int i = 0; i < 16; i++)
					output[i] = static_cast<lwmUInt8>((static_cast<lwmFastUInt16>(input1[i]) + static_cast<lwmFastUInt16>(input2[i])) / 2);
			}

			inline void Average8Bytes(lwmUInt8 *output, const lwmUInt8 *input1, const lwmUInt8 *input2)
			{
				for (int i = 0; i < 8; i++)
					output[i] = static_cast<lwmUInt8>((static_cast<lwmFastUInt16>(input1[i]) + static_cast<lwmFastUInt16>(input2[i])) / 2);
			}

			inline void Saturate8x16To8(lwmUInt8 *output, const lwmSInt16 *input)
			{
				for (int i = 0; i < 8; i++)
				{
					lwmSInt16 inCoeff = input[i];
					if (inCoeff < 0)
						output[i] = 0;
					else if (inCoeff > 255)
						output[i] = 255;
					else
						output[i] = static_cast<lwmUInt8>(inCoeff);
				}
			}

			inline void AddAndSaturate8(lwmUInt8 *output, const lwmSInt16 *inputS, const lwmUInt8 *inputB)
			{
				for (int i = 0; i < 8; i++)
				{
					lwmSInt16 inCoeff = inputS[i] + static_cast<lwmSInt16>(inputB[i]);
					if (inCoeff < 0)
						output[i] = 0;
					else if (inCoeff > 255)
						output[i] = 255;
					else
						output[i] = static_cast<lwmUInt8>(inCoeff);
				}
			}

			inline void ExtractMotionLuma(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 16;
				while (rows--)
				{
					memcpy(outBlock, inBase, 16);
					outBlock += 16;
					inBase += stride;
				}
			}

			inline void ExtractMotionLumaMerged(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 16;
				while (rows--)
				{
					Average16Bytes(outBlock, inBase, inBase + 1);
					outBlock += 16;
					inBase += stride;
				}
			}

			inline void ExtractMotionLumaAvg(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 16;
				lwmUInt8 prevBlock[16];
				memcpy(prevBlock, inBase, 16);
				while (rows--)
				{
					inBase += stride;
					lwmUInt8 block[16];
					memcpy(block, inBase, 16);
					for (int i = 0; i < 16; i++)
						outBlock[i] = static_cast<lwmUInt8>((static_cast<lwmFastUInt16>(prevBlock[i]) + static_cast<lwmFastUInt16>(block[i])) / 2);
					memcpy(prevBlock, block, 16);
					outBlock += 16;
				}
			}

			inline void ExtractMotionLumaMergedAvg(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 16;

				lwmUInt8 prevAvg[16];
				Average16Bytes(prevAvg, inBase, inBase + 1);

				while (rows--)
				{
					inBase += stride;
					lwmUInt8 avg[16];
					Average16Bytes(avg, inBase, inBase + 1);
					Average16Bytes(outBlock, avg, prevAvg);
					memcpy(prevAvg, avg, 16);
					outBlock += 16;
				}
			}

			inline void ExtractMotionLuma(lwmUInt8 *outBlock, const lwmUInt8 *inBlock, lwmSInt32 right, lwmSInt32 down, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
			{
#ifdef LWMOVIE_DEEP_PROFILE
				lwmCAutoProfile _(profileTags, lwmEPROFILETAG_Motion);
#endif

				const lwmUInt8 *rowScanBase = inBlock + (right >> 1) + (down >> 1) * static_cast<lwmLargeSInt>(stride);

				if ((down & 1) == 0)
				{
					// No vertical hpel

					if ((right & 1) == 0)
					{
						// No horizontal hpel
						ExtractMotionLuma(outBlock, rowScanBase, stride);
					}
					else
					{
						// Horizontal hpel
						ExtractMotionLumaMerged(outBlock, rowScanBase, stride);
					}
				}
				else
				{
					// Vertical hpel
					if ((right & 1) == 0)
					{
						// Unaligned, no horizontal hpel
						ExtractMotionLumaAvg(outBlock, rowScanBase, stride);
					}
					else
					{
						// Horizontal hpel
						ExtractMotionLumaMergedAvg(outBlock, rowScanBase, stride);
					}
				}
			}

			inline void MergeLumaMotion(lwmUInt8 *mergeBlock, const lwmUInt8 *cblock)
			{
				lwmLargeUInt rows = 16;

				while (rows--)
				{
					Average16Bytes(mergeBlock, mergeBlock, cblock);
					mergeBlock += 16;
					cblock += 16;
				}
			}

			inline void ZeroLumaBlockPair(lwmUInt8 *c, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				while (rows--)
				{
					memset(c, 0, 16);
					c += stride;
				}
			}

			inline void SetLumaDCTPaired(lwmUInt8 *c, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					Saturate8x16To8(c, blockData);
					Saturate8x16To8(c + 8, blockData + 64);
					c += stride;
					blockData += 8;
				}
			}

			inline void SetLumaDCTLow(lwmUInt8 *c, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					Saturate8x16To8(c, blockData);
					memset(c + 8, 0, 8);
					c += stride;
					blockData += 8;
				}
			}

			inline void SetLumaDCTHigh(lwmUInt8 *c, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					memset(c, 0, 8);
					Saturate8x16To8(c + 8, blockData + 64);
					c += stride;
					blockData += 8;
				}
			}

			inline void CopyLumaBlockPair(lwmUInt8 *c, const lwmUInt8 *motion, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				while (rows--)
				{
					memcpy(c, motion, 16);
					c += stride;
					motion += 16;
				}
			}

			inline void ApplyLumaDCTPaired(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					AddAndSaturate8(c, blockData, motion);
					AddAndSaturate8(c + 8, blockData + 64, motion + 8);
					c += stride;
					motion += 16;
					blockData += 8;
				}
			}


			inline void ApplyLumaDCTLow(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					AddAndSaturate8(c, blockData, motion);
					memcpy(c + 8, motion + 8, 8);
					c += stride;
					motion += 16;
					blockData += 8;
				}
			}

			inline void ApplyLumaDCTHigh(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;
				while (rows--)
				{
					memcpy(c, motion, 8);
					AddAndSaturate8(c + 8, blockData + 64, motion + 8);
					c += stride;
					motion += 16;
					blockData += 8;
				}
			}


			// ****************************************************************************
			// Chroma reconstruction
			inline void ZeroChromaBlock(lwmUInt8 *c, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				while (rows--)
				{
					memset(c, 0, 8);
					c += stride;
				}
			}

			inline void SetChromaDCT(lwmUInt8 *c, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				const lwmSInt16 *dctData = dctBlock->data;

				lwmLargeUInt rowPairs = 8;
				while (rowPairs--)
				{
					Saturate8x16To8(c, dctData);
					c += stride;
					dctData += 8;
				}
			}

			template<int rightSub>
			inline inline void ReadChromaRowTpl(lwmUInt8 *rowOutput, const lwmUInt8 *base)
			{
				if (rightSub == 0)
				{
					memcpy(rowOutput, base, 8);
					return;
				}

				Average8Bytes(rowOutput, base, base + 1);
			}

			template<int rightSub, int downSub>
			inline inline void ExtractMotionChromaTpl(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
			{
#ifdef LWMOVIE_DEEP_PROFILE
				lwmCAutoProfile _(profileTags, lwmEPROFILETAG_Motion);
#endif

				if (downSub == 0)
				{
					lwmLargeUInt rows = 8;
					while (rows--)
					{
						ReadChromaRowTpl<rightSub>(outBlock, inBase);
						inBase += stride;
						outBlock += 8;
					}
				}
				else
				{
					lwmUInt8 prevRow[8];
					ReadChromaRowTpl<rightSub>(prevRow, inBase);
					for (int row = 0; row < 8; row++)
					{
						inBase += stride;
						lwmUInt8 thisRow[8];
						ReadChromaRowTpl<rightSub>(thisRow, inBase);
						Average8Bytes(outBlock, prevRow, thisRow);
						memcpy(prevRow, thisRow, 8);
						outBlock += 8;
					}
				}
			}

			inline void ExtractMotionChroma(lwmUInt8 *outBlock, const lwmUInt8 *inBlock, lwmSInt32 right, lwmSInt32 down, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
			{
				const lwmUInt8 *rowScanBase = inBlock + (right >> 1) + (down >> 1) * static_cast<lwmLargeSInt>(stride);

				switch (((down & 0x1) << 1) | (right & 0x1))
				{
				case 0:
					ExtractMotionChromaTpl<0, 0>(outBlock, rowScanBase, stride, profileTags);
					break;
				case 1:
					ExtractMotionChromaTpl<1, 0>(outBlock, rowScanBase, stride, profileTags);
					break;
				case 2:
					ExtractMotionChromaTpl<0, 1>(outBlock, rowScanBase, stride, profileTags);
					break;
				case 3:
					ExtractMotionChromaTpl<1, 1>(outBlock, rowScanBase, stride, profileTags);
					break;
				};
			}


			inline void MergeChromaMotion(lwmUInt8 *mergeBlock, const lwmUInt8 *cblock)
			{
				lwmLargeUInt rows = 8;

				while (rows--)
				{
					Average8Bytes(mergeBlock, mergeBlock, cblock);
					mergeBlock += 8;
					cblock += 8;
				}
			}

			inline void CopyChromaBlock(lwmUInt8 *output, const lwmUInt8 *mblock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;

				while (rows--)
				{
					memcpy(output, mblock, 8);
					output += stride;
					mblock += 8;
				}
			}

			inline void ApplyChromaDCT(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::idct::DCTBLOCK *dctBlock, lwmLargeUInt stride)
			{
				lwmLargeUInt rows = 8;
				const lwmSInt16 *blockData = dctBlock->data;

				while (rows--)
				{
					AddAndSaturate8(c, blockData, motion);
					c += stride;
					motion += 8;
					blockData += 8;
				}
			}
		}
	}
}
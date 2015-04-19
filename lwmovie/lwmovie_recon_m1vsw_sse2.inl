/*
 * Copyright (c) 2014 Eric Lasota
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

#ifndef LWMOVIE_SSE2
#error "Don't include this file unless SSE2 is enabled"
#endif

#include <emmintrin.h>

#include "lwmovie_recon_m1vsw.hpp"
#include "lwmovie_profile.hpp"
#include "lwmovie_idct.hpp"


static void ExtractMotionLumaAligned(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	while(rows--)
	{
		__m128i block = _mm_load_si128(reinterpret_cast<const __m128i *>(inBase));
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), block);
		outBlock += 16;
		inBase += stride;
	}
}

static void ExtractMotionLumaUnaligned(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	while(rows--)
	{
		__m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), block);
		outBlock += 16;
		inBase += stride;
	}
}

static void ExtractMotionLumaMerged(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	while(rows--)
	{
		__m128i blockA = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
		__m128i blockB = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase + 1));
		__m128i avg = _mm_avg_epu8(blockA, blockB);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), avg);
		outBlock += 16;
		inBase += stride;
	}
}


static void ExtractMotionLumaAlignedAvg(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	__m128i prevBlock = _mm_load_si128(reinterpret_cast<const __m128i *>(inBase));
	while(rows--)
	{
		inBase += stride;
		__m128i block = _mm_load_si128(reinterpret_cast<const __m128i *>(inBase));
		__m128i avg = _mm_avg_epu8(block, prevBlock);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), avg);
		prevBlock = block;
		outBlock += 16;
	}
}

static void ExtractMotionLumaUnalignedAvg(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	__m128i prevBlock = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
	while(rows--)
	{
		inBase += stride;
		__m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
		__m128i avg = _mm_avg_epu8(block, prevBlock);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), avg);
		prevBlock = block;
		outBlock += 16;
	}
}

static void ExtractMotionLumaMergedAvg(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 16;
	
	__m128i blockA = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
	__m128i blockB = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase + 1));
	__m128i prevAvg = _mm_avg_epu8(blockA, blockB);

	while(rows--)
	{
		inBase += stride;
		blockA = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase));
		blockB = _mm_loadu_si128(reinterpret_cast<const __m128i *>(inBase + 1));
		__m128i avg = _mm_avg_epu8(blockA, blockB);
		__m128i finalAvg = _mm_avg_epu8(avg, prevAvg);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), finalAvg);
		prevAvg = avg;
		outBlock += 16;
	}
}

static void ExtractMotionLuma(lwmUInt8 *outBlock, const lwmUInt8 *inBlock, lwmSInt32 right, lwmSInt32 down, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_Motion);
#endif

	const lwmUInt8 *rowScanBase = inBlock + (right >> 1) + (down >> 1) * static_cast<lwmLargeSInt>(stride);

	if((down & 1) == 0)
	{
		// No vertical hpel

		if((right & 0x1f) == 0)
		{
			// Aligned, no horizontal hpel
			ExtractMotionLumaAligned(outBlock, rowScanBase, stride);
		}
		else if((right & 1) == 0)
		{
			// Unaligned, no horizontal hpel
			ExtractMotionLumaUnaligned(outBlock, rowScanBase, stride);
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
		if((right & 0x1f) == 0)
		{
			// Aligned, no horizontal hpel
			ExtractMotionLumaAlignedAvg(outBlock, rowScanBase, stride);
		}
		else if((right & 1) == 0)
		{
			// Unaligned, no horizontal hpel
			ExtractMotionLumaUnalignedAvg(outBlock, rowScanBase, stride);
		}
		else
		{
			// Horizontal hpel
			ExtractMotionLumaMergedAvg(outBlock, rowScanBase, stride);
		}
	}
}

static void MergeLumaMotion(lwmUInt8 *mergeBlock, const lwmUInt8 *cblock)
{
	lwmLargeUInt rows = 16;

	while(rows--)
	{
		__m128i inA = _mm_load_si128(reinterpret_cast<const __m128i *>(mergeBlock));
		__m128i inB = _mm_load_si128(reinterpret_cast<const __m128i *>(cblock));
		_mm_store_si128(reinterpret_cast<__m128i *>(mergeBlock), _mm_avg_epu8(inA, inB));
		mergeBlock += 16;
		cblock += 16;
	}
}

static void ZeroLumaBlockPair(lwmUInt8 *c, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	__m128i zero = _mm_setzero_si128();
	while(rows--)
	{
		_mm_store_si128(reinterpret_cast<__m128i*>(c), zero);
		c += stride;
	}
}

static void SetLumaDCTPaired(lwmUInt8 *c, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	__m128i zero = _mm_setzero_si128();
	const lwmSInt16 *blockData = dctBlock->data;
	while(rows--)
	{
		__m128i block1 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData));
		block1 = _mm_max_epi16(block1, zero);
		__m128i block2 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData + 64));
		block2 = _mm_max_epi16(block2, zero);
		__m128i merged = _mm_packus_epi16(block1, block2);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), merged);
		c += stride;
		blockData += 8;
	}
}

static void SetLumaDCTLow(lwmUInt8 *c, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	__m128i zero = _mm_setzero_si128();
	const lwmSInt16 *blockData = dctBlock->data;
	while(rows--)
	{
		__m128i block1 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData));
		block1 = _mm_max_epi16(block1, zero);
		__m128i merged = _mm_packus_epi16(block1, zero);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), merged);
		c += stride;
		blockData += 8;
	}
}

static void SetLumaDCTHigh(lwmUInt8 *c, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	__m128i zero = _mm_setzero_si128();
	const lwmSInt16 *blockData = dctBlock->data;
	while(rows--)
	{
		__m128i block2 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData + 64));
		block2 = _mm_max_epi16(block2, zero);
		__m128i merged = _mm_packus_epi16(zero, block2);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), merged);
		c += stride;
		blockData += 8;
	}
}

static void CopyLumaBlockPair(lwmUInt8 *c, const lwmUInt8 *motion, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	while(rows--)
	{
		__m128i block = _mm_load_si128(reinterpret_cast<const __m128i *>(motion));
		_mm_store_si128(reinterpret_cast<__m128i*>(c), block);
		c += stride;
		motion += 16;
	}
}

static void ApplyLumaDCTPaired(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	const lwmSInt16 *blockData = dctBlock->data;
	__m128i zero = _mm_setzero_si128();
	while(rows--)
	{
		__m128i mblock = _mm_load_si128(reinterpret_cast<const __m128i *>(motion));
		__m128i dctblock1 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData));
		__m128i dctblock2 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData + 64));
		__m128i mblock1 = _mm_unpacklo_epi8(mblock, zero);
		__m128i mblock2 = _mm_unpackhi_epi8(mblock, zero);
		__m128i merged1 = _mm_adds_epi16(mblock1, dctblock1);
		__m128i merged2 = _mm_adds_epi16(mblock2, dctblock2);
		merged1 = _mm_max_epi16(merged1, zero);
		merged2 = _mm_max_epi16(merged2, zero);
		__m128i combinedBlock = _mm_packus_epi16(merged1, merged2);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), combinedBlock);
		c += stride;
		motion += 16;
		blockData += 8;
	}
}

static void ApplyLumaDCTLow(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	const lwmSInt16 *blockData = dctBlock->data;
	__m128i zero = _mm_setzero_si128();
	while(rows--)
	{
		__m128i mblock = _mm_load_si128(reinterpret_cast<const __m128i *>(motion));
		__m128i dctblock1 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData));
		__m128i mblock1 = _mm_unpacklo_epi8(mblock, zero);
		__m128i mblock2 = _mm_unpackhi_epi8(mblock, zero);
		__m128i merged1 = _mm_adds_epi16(mblock1, dctblock1);
		merged1 = _mm_max_epi16(merged1, zero);
		__m128i combinedBlock = _mm_packus_epi16(merged1, mblock2);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), combinedBlock);
		c += stride;
		motion += 16;
		blockData += 8;
	}
}

static void ApplyLumaDCTHigh(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rows = 8;
	const lwmSInt16 *blockData = dctBlock->data;
	__m128i zero = _mm_setzero_si128();
	while(rows--)
	{
		__m128i mblock = _mm_load_si128(reinterpret_cast<const __m128i *>(motion));
		__m128i dctblock2 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData + 64));
		__m128i mblock1 = _mm_unpacklo_epi8(mblock, zero);
		__m128i mblock2 = _mm_unpackhi_epi8(mblock, zero);
		__m128i merged2 = _mm_adds_epi16(mblock2, dctblock2);
		merged2 = _mm_max_epi16(merged2, zero);
		__m128i combinedBlock = _mm_packus_epi16(mblock1, merged2);
		_mm_store_si128(reinterpret_cast<__m128i*>(c), combinedBlock);
		c += stride;
		motion += 16;
		blockData += 8;
	}
}


// ****************************************************************************
// Chroma reconstruction
static void ZeroChromaBlock(lwmUInt8 *c, lwmLargeUInt stride)
{
	__m128i zero = _mm_setzero_si128();

	lwmLargeUInt rows = 8;
	while(rows--)
	{
		_mm_storel_epi64(reinterpret_cast<__m128i*>(c), zero);
		c += stride;
	}
}

static void SetChromaDCT(lwmUInt8 *c, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	const lwmSInt16 *dctData = dctBlock->data;
	__m128i zero = _mm_setzero_si128();

	lwmLargeUInt rowPairs = 4;
	while(rowPairs--)
	{
		__m128i dctRow1 = _mm_load_si128(reinterpret_cast<const __m128i *>(dctData));
		dctRow1 = _mm_max_epi16(dctRow1, zero);
		__m128i dctRow2 = _mm_load_si128(reinterpret_cast<const __m128i *>(dctData + 8));
		dctRow2 = _mm_max_epi16(dctRow2, zero);
		__m128i mergedRows = _mm_packus_epi16(dctRow1, dctRow2);

		_mm_storel_epi64(reinterpret_cast<__m128i*>(c), mergedRows);
		c += stride;
		mergedRows = _mm_srli_si128(mergedRows, 8);
		_mm_storel_epi64(reinterpret_cast<__m128i*>(c), mergedRows);
		c += stride;
		dctData += 16;
	}
}

template<int rightSub>
static inline __m128i ReadChromaRowPairTpl(const lwmUInt8 *base, lwmLargeUInt stride)
{
	if(rightSub == 0)
	{
		__m128i row1 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base));
		__m128i row2 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base + stride));
		return _mm_unpacklo_epi64(row1, row2);
	}

	__m128i left1 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base));
	__m128i left2 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base + stride));
	__m128i left = _mm_unpacklo_epi64(left1, left2);
	__m128i right1 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base + 1));
	__m128i right2 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base + 1 + stride));
	__m128i right = _mm_unpacklo_epi64(right1, right2);

	return _mm_avg_epu8(left, right);
}

template<int rightSub>
static inline __m128i ReadChromaRowTpl(const lwmUInt8 *base)
{
	if(rightSub == 0)
		return _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base));

	__m128i left = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base));
	__m128i right = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(base + 1));

	return _mm_avg_epu8(left, right);
}

template<int rightSub, int downSub>
static inline void ExtractMotionChromaTpl(lwmUInt8 *outBlock, const lwmUInt8 *inBase, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_Motion);
#endif

	if(downSub == 0)
	{
		lwmLargeUInt rowsPairs = 4;
		while(rowsPairs--)
		{
			_mm_store_si128(reinterpret_cast<__m128i *>(outBlock), ReadChromaRowPairTpl<rightSub>(inBase, stride));

			// TODO: Remove
			//__m128i row1 = ReadChromaRowTpl<rightSub>(inBase);
			inBase += stride * 2;
			outBlock += 16;
		}
	}
	else
	{
		__m128i rows01 = ReadChromaRowPairTpl<rightSub>(inBase, stride);
		__m128i rows23 = ReadChromaRowPairTpl<rightSub>(inBase + stride * 2, stride);
		__m128i rows22 = _mm_unpacklo_epi64(rows23, rows23);
		__m128i rows12 = _mm_unpackhi_epi64(rows01, rows22);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock + 0), _mm_avg_epu8(rows01, rows12));
		__m128i rows45 = ReadChromaRowPairTpl<rightSub>(inBase + stride * 4, stride);
		__m128i rows44 = _mm_unpacklo_epi64(rows45, rows45);
		__m128i rows34 = _mm_unpackhi_epi64(rows23, rows44);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock + 16), _mm_avg_epu8(rows23, rows34));
		__m128i rows67 = ReadChromaRowPairTpl<rightSub>(inBase + stride * 6, stride);
		__m128i rows66 = _mm_unpacklo_epi64(rows67, rows67);
		__m128i rows56 = _mm_unpackhi_epi64(rows45, rows66);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock + 32), _mm_avg_epu8(rows45, rows56));
		__m128i rows77 = _mm_unpackhi_epi64(rows67, rows67);
		__m128i lastRow = ReadChromaRowTpl<rightSub>(inBase + stride * 8);
		__m128i rows78 = _mm_unpacklo_epi64(rows77, lastRow);
		_mm_store_si128(reinterpret_cast<__m128i *>(outBlock + 48), _mm_avg_epu8(rows67, rows78));
	}
}

static void ExtractMotionChroma(lwmUInt8 *outBlock, const lwmUInt8 *inBlock, lwmSInt32 right, lwmSInt32 down, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
{
	const lwmUInt8 *rowScanBase = inBlock + (right >> 1) + (down >> 1) * static_cast<lwmLargeSInt>(stride);

	switch( ((down & 0x1) << 1) | (right & 0x1) )
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


static void MergeChromaMotion(lwmUInt8 *mergeBlock, const lwmUInt8 *cblock)
{
	lwmLargeUInt rowsPairs = 4;

	while(rowsPairs--)
	{
		__m128i inA = _mm_load_si128(reinterpret_cast<const __m128i *>(mergeBlock));
		__m128i inB = _mm_load_si128(reinterpret_cast<const __m128i *>(cblock));
		_mm_store_si128(reinterpret_cast<__m128i *>(mergeBlock), _mm_avg_epu8(inA, inB));
		mergeBlock += 16;
		cblock += 16;
	}
}


static void CopyChromaBlock(lwmUInt8 *output, const lwmUInt8 *mblock, lwmLargeUInt stride)
{
	lwmLargeUInt rowsPairs = 4;

	while(rowsPairs--)
	{
		__m128i rowPair = _mm_load_si128(reinterpret_cast<const __m128i *>(mblock));
		_mm_storel_epi64(reinterpret_cast<__m128i *>(output), rowPair);
		output += stride;

		rowPair = _mm_srli_si128(rowPair, 8);
		_mm_storel_epi64(reinterpret_cast<__m128i *>(output), rowPair);
		output += stride;
		mblock += 16;
	}
}

static void ApplyChromaDCT(lwmUInt8 *c, const lwmUInt8 *motion, const lwmovie::lwmDCTBLOCK *dctBlock, lwmLargeUInt stride)
{
	lwmLargeUInt rowsPairs = 4;
	const lwmSInt16 *blockData = dctBlock->data;

	__m128i zero = _mm_setzero_si128();
	while(rowsPairs--)
	{
		__m128i mblockRowPair = _mm_load_si128(reinterpret_cast<const __m128i *>(motion));
		__m128i mblockRow1 = _mm_unpacklo_epi8(mblockRowPair, zero);
		__m128i dctRow1 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData));
		__m128i merged1 = _mm_adds_epi16(mblockRow1, dctRow1);
		merged1 = _mm_max_epi16(merged1, zero);
		
		__m128i mblockRow2 = _mm_unpackhi_epi8(mblockRowPair, zero);
		__m128i dctRow2 = _mm_load_si128(reinterpret_cast<const __m128i *>(blockData + 8));
		__m128i merged2 = _mm_adds_epi16(mblockRow2, dctRow2);
		merged2 = _mm_max_epi16(merged2, zero);

		__m128i combinedBlock = _mm_packus_epi16(merged1, merged2);

		_mm_storel_epi64(reinterpret_cast<__m128i*>(c), combinedBlock);
		c += stride;

		combinedBlock = _mm_srli_si128(combinedBlock, 8);
		_mm_storel_epi64(reinterpret_cast<__m128i*>(c), combinedBlock);

		c += stride;
		motion += 16;
		blockData += 16;
	}
}

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

#ifdef LWMOVIE_SSE2
#include <emmintrin.h>
#endif

#include <string.h>
#include "../common/lwmovie_coretypes.h"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_idct.hpp"


namespace lwmovie
{
	namespace idct
	{
		class SparseIDCTContainer
		{
		public:
			void Init()
			{
				lwmUInt8 *sbData = m_sparseBlockData + 15;
				lwmLargeSInt diff = sbData - static_cast<const lwmUInt8 *>(NULL);
				sbData -= (diff & 0xf);
				m_sparseBlocks = reinterpret_cast<lwmDCTBLOCK*>(sbData);

				for(int i=0;i<64;i++)
				{
					m_sparseBlocks[i].FastZeroFill();
					m_sparseBlocks[i].data[i] = 256;
					IDCT(m_sparseBlocks[i].data);
				}
			}

			inline lwmDCTBLOCK *GetSparseBlock(lwmFastUInt8 index)
			{
				return m_sparseBlocks + index;
			}

			static SparseIDCTContainer staticInstance;

		private:
			lwmUInt8 m_sparseBlockData[sizeof(lwmDCTBLOCK) * 64 + 15];
			lwmDCTBLOCK *m_sparseBlocks;
		};
	}
}

lwmovie::idct::SparseIDCTContainer lwmovie::idct::SparseIDCTContainer::staticInstance;

void lwmovie::idct::Initialize()
{
	SparseIDCTContainer::staticInstance.Init();
}

void lwmovie::idct::IDCT_SparseDC( lwmSInt16 data[64], lwmSInt16 value )
{
#ifdef LWMOVIE_SSE2
	__m128i fill = _mm_setzero_si128();
	fill = _mm_insert_epi16(fill, static_cast<int>(value >> 3), 0);
	fill = _mm_unpacklo_epi16(fill, fill);
	fill = _mm_unpacklo_epi32(fill, fill);
	fill = _mm_unpacklo_epi64(fill, fill);
	int rows = 8;
	lwmSInt16 *dataPtr = data;
	while(rows--)
	{
		_mm_store_si128(reinterpret_cast<__m128i*>(dataPtr), fill);
		dataPtr += 8;
	}
#else
	value >>= 3;
	int rows = 8;
	lwmSInt16 *dataPtr = data;
	while(rows--)
	{
		for(int i=0;i<8;i++)
			dataPtr[i] = value;
		dataPtr += 8;
	}
#endif
}

void lwmovie::idct::IDCT_SparseAC( lwmSInt16 data[64], lwmFastUInt8 coeffPos, lwmSInt16 value )
{
#ifdef LWMOVIE_SSE2
	__m128i fill = _mm_setzero_si128();
	fill = _mm_insert_epi16(fill, static_cast<int>(value), 0);
	fill = _mm_unpacklo_epi16(fill, fill);
	fill = _mm_unpacklo_epi32(fill, fill);
	fill = _mm_unpacklo_epi64(fill, fill);

	int rows = 8;
	lwmSInt16 *dataPtr = data;
	const lwmSInt16 *sparseMat = lwmovie::idct::SparseIDCTContainer::staticInstance.GetSparseBlock(coeffPos)->data;
	while(rows--)
	{
		__m128i sparseInputRow = _mm_load_si128(reinterpret_cast<const __m128i*>(sparseMat));
		__m128i mullo = _mm_mullo_epi16(sparseInputRow, fill);
		__m128i mulhi = _mm_mulhi_epi16(sparseInputRow, fill);
		__m128i mullos = _mm_srli_epi16(mullo, 8);
		__m128i mulhis = _mm_slli_epi16(mulhi, 8);
		__m128i merged = _mm_or_si128(mullos, mulhis);
		_mm_store_si128(reinterpret_cast<__m128i*>(dataPtr), merged);
		dataPtr += 8;
		sparseMat += 8;
	}
#else
	int rows = 8;
	lwmSInt16 *dataPtr = data;
	const lwmSInt16 *sparseMat = lwmovie::idct::SparseIDCTContainer::staticInstance.GetSparseBlock(coeffPos)->data;
	while(rows--)
	{
		const lwmSInt16 *sparseInputRow = sparseMat;
		for(int i=0;i<8;i++)
			dataPtr[i] = static_cast<lwmSInt16>((static_cast<lwmSInt32>(sparseInputRow[i]) * value) >> 8);
		dataPtr += 8;
		sparseMat += 8;
	}
#endif
}

extern "C" void j_rev_dct_sse2( lwmSInt16 data[64] );
extern "C" void j_rev_dct ( lwmSInt16 data[64] );

void lwmovie::idct::IDCT( lwmSInt16 data[64] )
{
#ifdef LWMOVIE_SSE2
	j_rev_dct_sse2(data);
#endif
#ifdef LWMOVIE_NOSIMD
	j_rev_dct(data);
#endif
}


void lwmovie::lwmBlockInfo::IDCT(lwmDCTBLOCK *block) const
{
	if(needs_idct)
	{
		if(sparse_idct)
		{
			if(sparse_idct_index == 0)
				lwmovie::idct::IDCT_SparseDC(block->data, sparse_idct_coef);
			else
				lwmovie::idct::IDCT_SparseAC(block->data, sparse_idct_index, sparse_idct_coef);
		}
		else
			lwmovie::idct::IDCT(block->data);
	}
	else
	{
		block->FastZeroFill();
	}
}

#ifdef LWMOVIE_SSE2
void lwmovie::lwmDCTBLOCK::FastZeroFill()
{
	int rows = 8;
	__m128i zero = _mm_setzero_si128();
	lwmSInt16 *coeffs = data;
	while (rows--)
	{
		_mm_store_si128(reinterpret_cast<__m128i*>(coeffs), zero);
		coeffs += 8;
	}
}
#endif

#ifdef LWMOVIE_NOSIMD
inline void lwmovie::lwmDCTBLOCK::FastZeroFill()
{
	memset(this->data, 0, sizeof(lwmSInt16) * 64);
}
#endif

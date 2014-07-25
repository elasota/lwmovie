#include <emmintrin.h>
#include <string.h>
#include "lwmovie_types.hpp"
#include "lwmovie_videotypes.hpp"

void j_rev_dct_sse2( lwmSInt16 data[64] );

namespace lwmovie
{
	class lwmSparseIDCTContainer
	{
	public:
		lwmSparseIDCTContainer()
		{
			lwmUInt8 *sbData = m_sparseBlockData + 15;
			lwmLargeSInt diff = sbData - static_cast<const lwmUInt8 *>(NULL);
			sbData -= (diff & 0xf);
			m_sparseBlocks = reinterpret_cast<lwmDCTBLOCK*>(sbData);

			for(int i=0;i<64;i++)
			{
				m_sparseBlocks[i].FastZeroFill();
				m_sparseBlocks[i].data[i] = 256;
				j_rev_dct_sse2(m_sparseBlocks[i].data);
			}
		}

		inline lwmDCTBLOCK *GetSparseBlock(lwmFastUInt8 index)
		{
			return m_sparseBlocks + index;
		}

		static lwmSparseIDCTContainer staticInstance;

	private:
		lwmUInt8 m_sparseBlockData[sizeof(lwmDCTBLOCK) * 64 + 15];
		lwmDCTBLOCK *m_sparseBlocks;
	};
}


lwmovie::lwmSparseIDCTContainer lwmovie::lwmSparseIDCTContainer::staticInstance;


void j_rev_dct_sse2_sparseDC( lwmSInt16 data[64], lwmSInt16 value )
{
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
}

void j_rev_dct_sse2_sparseAC( lwmSInt16 data[64], lwmFastUInt8 coeffPos, lwmSInt16 value )
{
	__m128i fill = _mm_setzero_si128();
	fill = _mm_insert_epi16(fill, static_cast<int>(value), 0);
	fill = _mm_unpacklo_epi16(fill, fill);
	fill = _mm_unpacklo_epi32(fill, fill);
	fill = _mm_unpacklo_epi64(fill, fill);

	int rows = 8;
	lwmSInt16 *dataPtr = data;
	const lwmSInt16 *sparseMat = lwmovie::lwmSparseIDCTContainer::staticInstance.GetSparseBlock(coeffPos)->data;
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
}


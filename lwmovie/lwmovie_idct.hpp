#ifndef __LWMOVIE_IDCT_HPP__
#define __LWMOVIE_IDCT_HPP__

namespace lwmovie
{
	namespace idct
	{
		struct DCTBLOCK
		{
			lwmSInt16 data[64];

			void FastZeroFill();
		};

		void Initialize();
		void IDCT( lwmSInt16 data[64] );
		void IDCT_SparseDC( lwmSInt16 data[64], lwmSInt16 value );
		void IDCT_SparseAC( lwmSInt16 data[64], lwmFastUInt8 coeffPos, lwmSInt16 value );
	}
}

#endif

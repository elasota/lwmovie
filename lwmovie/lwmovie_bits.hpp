#ifndef __LWMOVIE_BITS_HPP__
#define __LWMOVIE_BITS_HPP__

#include "lwmovie_types.hpp"

namespace lwmovie
{
	namespace bits
	{
		inline lwmUInt32 nBitMask(lwmUInt8 idx)
		{
			if(idx == 0)
				return 0;
			return 0xffffffff << (32 - idx);
		}

		inline lwmUInt32 bitMask(lwmUInt8 idx)
		{
			if (idx == 0)
                return 0xffffffff;
            return ~(0xffffffff << (32 - idx));
        }

        inline lwmUInt32 rBitMask(lwmUInt8 idx)
        {
            lwmUInt32 mask = 0xffffffff << idx;
            return mask;
        }

        inline lwmUInt32 bitTest(lwmUInt8 idx)
        {
            return static_cast<lwmUInt32>(1) << (31 - idx);
        }

		inline lwmUInt8 saturate8(lwmSInt16 coeff)
		{
			if(coeff < 0)
				return 0;
			if(coeff > 255)
				return 255;
			return static_cast<lwmUInt8>(coeff);
		}
	}
}

#endif

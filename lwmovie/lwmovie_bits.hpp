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
#ifndef __LWMOVIE_BITS_HPP__
#define __LWMOVIE_BITS_HPP__

#include "../common/lwmovie_coretypes.h"

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

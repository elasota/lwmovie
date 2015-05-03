/*
* Mersenne Twister Random Algorithm
* Copyright (c) 2006 Ryan Martell.
* Based on A C-program for MT19937, with initialization improved 2002/1/26. Coded by
* Takuji Nishimura and Makoto Matsumoto.
*
* This file is part of FFmpeg.
*
* FFmpeg is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* FFmpeg is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef __SB4_RANDOM_HPP__
#define __SB4_RANDOM_HPP__

#define SB4_AV_RANDOM_N 624

#include "../common/lwmovie_coretypes.h"

class SB4RandomState
{
public:
	void Init(lwmUInt32 seed);
	lwmUInt32 Random();

private:
	void GenerateUntemperedNumbers();
	lwmUInt32 m_mt[SB4_AV_RANDOM_N]; ///< the array for the state vector
	lwmUInt32 m_index; ///< current untempered value we use as the base.
};


inline lwmUInt32 SB4RandomState::Random()
{
	// regenerate the untempered numbers if we should...
	if (m_index >= SB4_AV_RANDOM_N)
		GenerateUntemperedNumbers();

	// grab one...
	lwmUInt32 y = m_mt[m_index++];

	/* Now temper (Mersenne Twister coefficients) The coefficients for MT19937 are.. */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= (y >> 18);

	return y;
}

#endif // __SB4_RANDOM_HPP__


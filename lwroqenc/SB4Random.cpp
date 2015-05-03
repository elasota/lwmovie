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


/**
see http://en.wikipedia.org/wiki/Mersenne_twister for an explanation of this algorithm.
*/
#include <stdio.h>
#include "SB4Random.hpp"


/* Period parameters */
#define M 397
#define A 0x9908b0df /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/** initializes mt[AV_RANDOM_N] with a seed */
void SB4RandomState::Init(lwmUInt32 seed)
{
	/*
	This differs from the wikipedia article.  Source is from the Makoto
	Makoto Matsumoto and Takuji Nishimura code, with the following comment:
	*/
	/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
	/* In the previous versions, MSBs of the seed affect   */
	/* only MSBs of the array mt[].                        */
	m_mt[0] = seed & 0xffffffff;
	for (lwmUInt32 index = 1; index < SB4_AV_RANDOM_N; index++) {
		unsigned int prev = m_mt[index - 1];
		m_mt[index] = (1812433253UL * (prev ^ (prev >> 30)) + index) & 0xffffffff;
	}
	m_index = SB4_AV_RANDOM_N; // will cause it to generate untempered numbers the first iteration
}

/** generate AV_RANDOM_N words at one time (which will then be tempered later) (av_random calls this; you shouldn't) */
void SB4RandomState::GenerateUntemperedNumbers()
{
	lwmUInt32 kk;

	for (kk = 0; kk < SB4_AV_RANDOM_N - M; kk++) {
		lwmUInt32 y = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
		m_mt[kk] = m_mt[kk + M] ^ (y >> 1) ^ ((y & 1)*A);
	}
	for (; kk < SB4_AV_RANDOM_N - 1; kk++) {
		lwmUInt32 y = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
		m_mt[kk] = m_mt[kk + (M - SB4_AV_RANDOM_N)] ^ (y >> 1) ^ ((y & 1)*A);
	}
	lwmUInt32 y = (m_mt[SB4_AV_RANDOM_N - 1] & UPPER_MASK) | (m_mt[0] & LOWER_MASK);
	m_mt[SB4_AV_RANDOM_N - 1] = m_mt[M - 1] ^ (y >> 1) ^ ((y & 1)*A);
	m_index = 0;
}

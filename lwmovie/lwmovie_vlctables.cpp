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

/*
* Copyright (c) 1995 The Regents of the University of California.
* All rights reserved.    
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
* CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS
* ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/*
* Portions of this software Copyright (c) 1995 Brown University.
* All rights reserved.
* 
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement
* is hereby granted, provided that the above copyright notice and the
* following two paragraphs appear in all copies of this software.
* 
* IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
* UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
* BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
* SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "lwmovie_vlc.hpp"


lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::coded_block_pattern[512] =
{
	{ lwmovie::m1v::vlc::UERROR8, 0 }, { lwmovie::m1v::vlc::UERROR8, 0 },
	{39, 9}, {27, 9}, {59, 9}, {55, 9}, {47, 9}, {31, 9},
	{58, 8}, {58, 8}, {54, 8}, {54, 8}, {46, 8}, {46, 8}, {30, 8}, {30, 8},
	{57, 8}, {57, 8}, {53, 8}, {53, 8}, {45, 8}, {45, 8}, {29, 8}, {29, 8},
	{38, 8}, {38, 8}, {26, 8}, {26, 8}, {37, 8}, {37, 8}, {25, 8}, {25, 8},
	{43, 8}, {43, 8}, {23, 8}, {23, 8}, {51, 8}, {51, 8}, {15, 8}, {15, 8},
	{42, 8}, {42, 8}, {22, 8}, {22, 8}, {50, 8}, {50, 8}, {14, 8}, {14, 8},
	{41, 8}, {41, 8}, {21, 8}, {21, 8}, {49, 8}, {49, 8}, {13, 8}, {13, 8},
	{35, 8}, {35, 8}, {19, 8}, {19, 8}, {11, 8}, {11, 8}, {7, 8}, {7, 8},
	{34, 7}, {34, 7}, {34, 7}, {34, 7}, {18, 7}, {18, 7}, {18, 7}, {18, 7},
	{10, 7}, {10, 7}, {10, 7}, {10, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7}, 
	{33, 7}, {33, 7}, {33, 7}, {33, 7}, {17, 7}, {17, 7}, {17, 7}, {17, 7}, 
	{9, 7}, {9, 7}, {9, 7}, {9, 7}, {5, 7}, {5, 7}, {5, 7}, {5, 7}, 
	{63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, 
	{3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, 
	{36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, 
	{24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, 
	{62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
	{62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
	{2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, 
	{2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, 
	{61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, 
	{61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, 
	{1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, 
	{1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, 
	{56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, 
	{56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, 
	{52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, 
	{52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, 
	{44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, 
	{44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, 
	{28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, 
	{28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, 
	{40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, 
	{40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, 
	{20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, 
	{20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, 
	{48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, 
	{48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, 
	{12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, 
	{12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, 
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, 
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}
};

/* Decoding tables for dct_dc_size_luminance */
lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::dct_dc_size_luminance[32] =
{   {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, 
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, 
	{0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, 
	{4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5},
	{ lwmovie::m1v::vlc::UERROR8, 0 }
};

lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::dct_dc_size_luminance1[16] =
{   {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
	{8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10, 9}, {11, 9}
};

/* Decoding table for dct_dc_size_chrominance */
lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::dct_dc_size_chrominance[32] =
{   {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, 
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, 
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, 
	{3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5},
	{ lwmovie::m1v::vlc::UERROR8, 0 }
};

lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::dct_dc_size_chrominance1[32] =
{   {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, 
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, 
	{7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, 
	{8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10, 10}, {11, 10}
};

lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::mb_addr_inc[2048];

/* Macro for filling up the decoding table for mb_addr_inc */
static lwmUInt8 ASSIGN1(int start, int end, int step, lwmUInt8 val, lwmUInt8 num)
{
	for (int i = start; i < end; i+= step)
	{
		for (int j = 0; j < step; j++)
		{
			lwmovie::m1v::vlc::mb_addr_inc[i + j].value = val;
			lwmovie::m1v::vlc::mb_addr_inc[i + j].num_bits = num;
		}
		val--;
	}
	return val;
}

static void init_mb_addr_inc()
{
	for (int i = 0; i < 8; i++)
	{
		lwmovie::m1v::vlc::mb_addr_inc[i].value = lwmovie::m1v::vlc::UERROR8;
		lwmovie::m1v::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmovie::m1v::vlc::mb_addr_inc[8].value = lwmovie::m1v::vlc::MACRO_BLOCK_ESCAPE;
	lwmovie::m1v::vlc::mb_addr_inc[8].num_bits = 11;

	for (int i = 9; i < 15; i++)
	{
		lwmovie::m1v::vlc::mb_addr_inc[i].value = lwmovie::m1v::vlc::UERROR8;
		lwmovie::m1v::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmovie::m1v::vlc::mb_addr_inc[15].value = lwmovie::m1v::vlc::MACRO_BLOCK_STUFFING;
	lwmovie::m1v::vlc::mb_addr_inc[15].num_bits = 11;

	for (int i = 16; i < 24; i++)
	{
		lwmovie::m1v::vlc::mb_addr_inc[i].value = lwmovie::m1v::vlc::UERROR8;
		lwmovie::m1v::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmUInt8 val = 33;

	val = ASSIGN1(24, 36, 1, val, 11);
	val = ASSIGN1(36, 48, 2, val, 10);
	val = ASSIGN1(48, 96, 8, val, 8);
	val = ASSIGN1(96, 128, 16, val, 7);
	val = ASSIGN1(128, 256, 64, val, 5);
	val = ASSIGN1(256, 512, 128, val, 4);
	val = ASSIGN1(512, 1024, 256, val, 3);
	val = ASSIGN1(1024, 2048, 1024, val, 1);
}


lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::mb_type_P[64];
lwmovie::m1v::vlc::lwmVlcValue8 lwmovie::m1v::vlc::mb_type_B[64];

static void ASSIGN2(int start, int end, bool quant, bool motion_forward, bool motion_backward, bool pattern, bool intra, lwmUInt8 num, lwmovie::m1v::vlc::lwmVlcValue8 *mb_type)
{
	for (int i = start; i < end; i ++)
	{
		lwmUInt8 flags = 0;
		if (quant) flags |= lwmovie::m1v::vlc::MB_FLAG_QUANT;
		if (motion_forward) flags |= lwmovie::m1v::vlc::MB_FLAG_MOTION_FORWARD;
		if (motion_backward) flags |= lwmovie::m1v::vlc::MB_FLAG_MOTION_BACKWARD;
		if (pattern) flags |= lwmovie::m1v::vlc::MB_FLAG_PATTERN;
		if (intra) flags |= lwmovie::m1v::vlc::MB_FLAG_INTRA;

		mb_type[i].value = flags;
		mb_type[i].num_bits = num;
	}
}

static void init_mb_type_P()
{
	lwmovie::m1v::vlc::mb_type_P[0].value = 0;
	lwmovie::m1v::vlc::mb_type_P[0].num_bits = 0;

	ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(2, 4, 1, 0, 0, 1, 0, 5, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(4, 6, 1, 1, 0, 1, 0, 5, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(8, 16, 0, 1, 0, 0, 0, 3, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(16, 32, 0, 0, 0, 1, 0, 2, lwmovie::m1v::vlc::mb_type_P);
	ASSIGN2(32, 64, 0, 1, 0, 1, 0, 1, lwmovie::m1v::vlc::mb_type_P);
}

static void init_mb_type_B()
{
	lwmovie::m1v::vlc::mb_type_B[0].value = 0;
	lwmovie::m1v::vlc::mb_type_B[0].num_bits = 0;

	ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(2, 3, 1, 0, 1, 1, 0, 6, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(3, 4, 1, 1, 0, 1, 0, 6, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(4, 6, 1, 1, 1, 1, 0, 5, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(8, 12, 0, 1, 0, 0, 0, 4, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(12, 16, 0, 1, 0, 1, 0, 4, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(16, 24, 0, 0, 1, 0, 0, 3, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(24, 32, 0, 0, 1, 1, 0, 3, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(32, 48, 0, 1, 1, 0, 0, 2, lwmovie::m1v::vlc::mb_type_B);
	ASSIGN2(48, 64, 0, 1, 1, 1, 0, 2, lwmovie::m1v::vlc::mb_type_B);
}

lwmovie::m1v::vlc::lwmVlcValueS8 lwmovie::m1v::vlc::motion_vectors[2048];

static lwmSInt8 ASSIGN3(int start, int end, int step, lwmSInt8 val, int num)
{
	for (int i = start; i < end; i+= step)
	{
		for (int j = 0; j < step / 2; j++)
		{
			lwmovie::m1v::vlc::motion_vectors[i + j].value = val;
			lwmovie::m1v::vlc::motion_vectors[i + j].num_bits = num;
		}
		for (int j = step / 2; j < step; j++)
		{
			lwmovie::m1v::vlc::motion_vectors[i + j].value = -val;
			lwmovie::m1v::vlc::motion_vectors[i + j].num_bits = num;
		}
		val--;
	}
	return val;
}

static void init_motion_vectors()
{
	lwmSInt8 val = 16;

	for (int i = 0; i < 24; i++)
	{
		lwmovie::m1v::vlc::motion_vectors[i].value = lwmovie::m1v::vlc::ERROR8;
		lwmovie::m1v::vlc::motion_vectors[i].num_bits = 0;
	}

	val = ASSIGN3(24, 36, 2, val, 11);	// 6
	val = ASSIGN3(36, 48, 4, val, 10);	// 3
	val = ASSIGN3(48, 96, 16, val, 8);	// 3
	val = ASSIGN3(96, 128, 32, val, 7);	// 1
	val = ASSIGN3(128, 256, 128, val, 5);	// 1
	val = ASSIGN3(256, 512, 256, val, 4);	// 1
	val = ASSIGN3(512, 1024, 512, val, 3);	// 1
	val = ASSIGN3(1024, 2048, 1024, val, 1); // 1
}

void lwmovie::m1v::vlc::InitTables()
{
	init_mb_addr_inc();
	init_mb_type_P();
	init_mb_type_B();
	init_motion_vectors();
}

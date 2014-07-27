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
#include "lwmovie_constants.hpp"

lwmUInt8 lwmovie::constants::DEFAULT_INTRA_MATRIX[64] =
{
	8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

lwmUInt8 lwmovie::constants::DEFAULT_NON_INTRA_MATRIX[64] =
{
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
};

lwmUInt8 lwmovie::constants::ZIGZAG_DIRECT[64] =
{
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
	19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
	42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};


lwmUInt8 lwmovie::constants::ZIGZAG[] =
{
	0 + 0*8, 1 + 0*8, 0 + 1*8, 0 + 2*8, 1 + 1*8,
	2 + 0*8, 3 + 0*8, 2 + 1*8, 1 + 2*8, 0 + 3*8,
	0 + 4*8, 1 + 3*8, 2 + 2*8, 3 + 1*8, 4 + 0*8,
	5 + 0*8, 4 + 1*8, 3 + 2*8, 2 + 3*8, 1 + 4*8,
	0 + 5*8, 0 + 6*8, 1 + 5*8, 2 + 4*8, 3 + 3*8,
	4 + 2*8, 5 + 1*8, 6 + 0*8, 7 + 0*8, 6 + 1*8,
	5 + 2*8, 4 + 3*8, 3 + 4*8, 2 + 5*8, 1 + 6*8,
	0 + 7*8, 1 + 7*8, 2 + 6*8, 3 + 5*8, 4 + 4*8,
	5 + 3*8, 6 + 2*8, 7 + 1*8, 7 + 2*8, 6 + 3*8,
	5 + 4*8, 4 + 5*8, 3 + 6*8, 2 + 7*8, 3 + 7*8,
	4 + 6*8, 5 + 5*8, 6 + 4*8, 7 + 3*8, 7 + 4*8,
	6 + 5*8, 5 + 6*8, 4 + 7*8, 5 + 7*8, 6 + 6*8,
	7 + 5*8, 7 + 6*8, 6 + 7*8, 7 + 7*8
};

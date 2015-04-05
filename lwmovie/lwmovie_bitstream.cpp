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
#include <stdlib.h>

#include "lwmovie_videotypes.hpp"

bool lwmovie::lwmCBitstream::Initialize(const void *bytes, lwmUInt32 numBytes)
{
	this->bit_offset = 0;
	this->new_bits = bytes;
	this->new_undigested_bytes = numBytes;
	this->new_emitted_zero = false;
	this->cur_bits = this->get_more_bits();
	this->next_bits = this->get_more_bits();

	return true;
}

lwmUInt32 lwmovie::lwmCBitstream::show_bits32()
{
	if (bit_offset != 0)
		return cur_bits | (next_bits >> (32 - bit_offset));
	return cur_bits;
}

lwmUInt32 lwmovie::lwmCBitstream::show_bitsX(lwmFastUInt8 num, lwmUInt32 mask, lwmFastUInt8 shift)
{
	lwmFastUInt8 bO = bit_offset + num;
	if(bO > 32)
	{
		bO -= 32;
		return ((cur_bits & mask) >> shift) |
			(next_bits >> (shift + (num - bO)));
	}

	return ((cur_bits & mask) >> shift);
}

lwmUInt32 lwmovie::lwmCBitstream::show_bits1()  { return show_bitsX(1,  0x80000000, 31); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits2()  { return show_bitsX(2,  0xc0000000, 30); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits3()  { return show_bitsX(3,  0xe0000000, 29); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits4()  { return show_bitsX(4,  0xf0000000, 28); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits5()  { return show_bitsX(5,  0xf8000000, 27); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits6()  { return show_bitsX(6,  0xfc000000, 26); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits7()  { return show_bitsX(7,  0xfe000000, 25); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits8()  { return show_bitsX(8,  0xff000000, 24); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits9()  { return show_bitsX(9,  0xff800000, 23); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits10() { return show_bitsX(10, 0xffc00000, 22); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits11() { return show_bitsX(11, 0xffe00000, 21); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits12() { return show_bitsX(12, 0xfff00000, 20); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits13() { return show_bitsX(13, 0xfff80000, 19); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits14() { return show_bitsX(14, 0xfffc0000, 18); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits15() { return show_bitsX(15, 0xfffe0000, 17); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits16() { return show_bitsX(16, 0xffff0000, 16); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits17() { return show_bitsX(17, 0xffff8000, 15); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits18() { return show_bitsX(18, 0xffffc000, 14); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits19() { return show_bitsX(19, 0xffffe000, 13); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits20() { return show_bitsX(20, 0xfffff000, 12); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits21() { return show_bitsX(21, 0xfffff800, 11); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits22() { return show_bitsX(22, 0xfffffc00, 10); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits23() { return show_bitsX(23, 0xfffffe00,  9); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits24() { return show_bitsX(24, 0xffffff00,  8); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits25() { return show_bitsX(25, 0xffffff80,  7); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits26() { return show_bitsX(26, 0xffffffc0,  6); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits27() { return show_bitsX(27, 0xffffffe0,  5); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits28() { return show_bitsX(28, 0xfffffff0,  4); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits29() { return show_bitsX(29, 0xfffffff8,  3); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits30() { return show_bitsX(30, 0xfffffffc,  2); }
lwmUInt32 lwmovie::lwmCBitstream::show_bits31() { return show_bitsX(31, 0xfffffffe,  1); }

lwmUInt32 lwmovie::lwmCBitstream::get_bitsX(lwmFastUInt8 num, lwmUInt32 mask, lwmFastUInt8 shift)
{
	/* Check for underflow. */
	bit_offset += num;

	lwmUInt32 result;
	if ((bit_offset & 0x20) != 0)
	{
		bit_offset -= 32;
		if (bit_offset != 0)
			cur_bits |= (next_bits >> (num - bit_offset));
		result = ((cur_bits & mask) >> shift);
		cur_bits= next_bits << bit_offset;
		next_bits = get_more_bits();
	}
	else
	{
		result = (cur_bits & mask) >> shift;
		cur_bits <<= num;
	}
	return result;
}



lwmUInt32 lwmovie::lwmCBitstream::get_bits1()  { return get_bitsX(1,  0x80000000, 31); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits2()  { return get_bitsX(2,  0xc0000000, 30); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits3()  { return get_bitsX(3,  0xe0000000, 29); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits4()  { return get_bitsX(4,  0xf0000000, 28); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits5()  { return get_bitsX(5,  0xf8000000, 27); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits6()  { return get_bitsX(6,  0xfc000000, 26); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits7()  { return get_bitsX(7,  0xfe000000, 25); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits8()  { return get_bitsX(8,  0xff000000, 24); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits9()  { return get_bitsX(9,  0xff800000, 23); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits10() { return get_bitsX(10, 0xffc00000, 22); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits11() { return get_bitsX(11, 0xffe00000, 21); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits12() { return get_bitsX(12, 0xfff00000, 20); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits13() { return get_bitsX(13, 0xfff80000, 19); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits14() { return get_bitsX(14, 0xfffc0000, 18); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits15() { return get_bitsX(15, 0xfffe0000, 17); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits16() { return get_bitsX(16, 0xffff0000, 16); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits17() { return get_bitsX(17, 0xffff8000, 15); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits18() { return get_bitsX(18, 0xffffc000, 14); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits19() { return get_bitsX(19, 0xffffe000, 13); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits20() { return get_bitsX(20, 0xfffff000, 12); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits21() { return get_bitsX(21, 0xfffff800, 11); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits22() { return get_bitsX(22, 0xfffffc00, 10); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits23() { return get_bitsX(23, 0xfffffe00,  9); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits24() { return get_bitsX(24, 0xffffff00,  8); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits25() { return get_bitsX(25, 0xffffff80,  7); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits26() { return get_bitsX(26, 0xffffffc0,  6); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits27() { return get_bitsX(27, 0xffffffe0,  5); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits28() { return get_bitsX(28, 0xfffffff0,  4); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits29() { return get_bitsX(29, 0xfffffff8,  3); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits30() { return get_bitsX(30, 0xfffffffc,  2); }
lwmUInt32 lwmovie::lwmCBitstream::get_bits31() { return get_bitsX(31, 0xfffffffe,  1); }

lwmUInt32 lwmovie::lwmCBitstream::get_bitsn(lwmFastUInt8 num)
{
	return get_bitsX((num), (0xffffffff << (32 - (num))), (32 - (num)));
}

lwmUInt32 lwmovie::lwmCBitstream::show_bitsn(lwmFastUInt8 num)
{
	return show_bitsX((num), (0xffffffff << (32 - (num))), (32 - (num)));
}

void lwmovie::lwmCBitstream::flush_bits32()
{
	cur_bits = next_bits << bit_offset;
	next_bits = get_more_bits();
}

void lwmovie::lwmCBitstream::flush_bits(lwmFastUInt8 num)
{
	bit_offset += num;
	if((bit_offset & 0x20) != 0)
	{
		bit_offset -= 32;
		cur_bits = next_bits << bit_offset;
		next_bits = get_more_bits();
	}
	else
		cur_bits <<= num;
}

bool lwmovie::lwmCBitstream::check_next_bits(lwmUInt8 num, lwmUInt32 mask)
{
	return mask == show_bitsn(num);
}

inline lwmUInt32 lwmovie::lwmCBitstream::get_more_bits()
{
	const lwmUInt8 *bytes = static_cast<const lwmUInt8*>(new_bits);
	if(new_undigested_bytes >= 4)
	{
		lwmUInt32 outBits = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
		new_bits = bytes + 4;
		new_undigested_bytes -= 4;
		return outBits;
	}

	if(new_undigested_bytes == 0)
	{
		if(!new_emitted_zero)
		{
			new_emitted_zero = true;
			return 0;
		}
		return constants::MPEG_SEQ_END_CODE;
	}
	
	lwmUInt32 outBits = 0;
	outBits = bytes[0] << 8;
	if(new_undigested_bytes >= 2)
		outBits |= bytes[1];
	outBits <<= 8;
	if(new_undigested_bytes >= 3)
		outBits |= bytes[2];
	outBits <<= 8;
	new_bits = bytes + new_undigested_bytes;
	new_undigested_bytes = 0;
	return outBits;
}

lwmovie::lwmCBitstream::lwmCBitstream()
{
	cur_bits = 0;
	next_bits = 0;
	bit_offset = 0;

	buf_start = NULL;

	m_owner = NULL;

	m_stream = NULL;
}

lwmovie::lwmCBitstream::~lwmCBitstream()
{
	buf_start = NULL;
}

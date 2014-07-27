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
#ifndef __LWMOVIE_BITSTREAM_HPP__
#define __LWMOVIE_BITSTREAM_HPP__

#include "lwmovie_types.hpp"

namespace lwmovie
{
	class lwmVidStream;

	class lwmCBitstream
	{
	private:
		lwmUInt32 cur_bits;
		lwmUInt32 next_bits;
		lwmFastUInt8 bit_offset;

		const void *new_bits;
		lwmUInt32 new_undigested_bytes;
		bool new_emitted_zero;

		lwmUInt32 *buf_start;

		lwmVidStream *m_owner;
		bool m_eof;

		lwmIOFuncs m_iofuncs;
		void *m_stream;

		lwmSAllocator *m_alloc;

		lwmUInt32 get_more_bits();
		lwmUInt32 show_bitsX(lwmFastUInt8 num, lwmUInt32 mask, lwmFastUInt8 shift);
		lwmUInt32 get_bitsX(lwmFastUInt8 num, lwmUInt32 mask, lwmFastUInt8 shift);

	public:
		lwmCBitstream();
		~lwmCBitstream();

		//bool Initialize(lwmVidStream *ownerStream, const lwmSAllocator *alloc, const lwmIOFuncs *ioFuncs, void *ioStream, lwmLargeUInt preloadSize);
		bool Initialize(const void *bytes, lwmUInt32 numBytes);

		void flush_bits32();
		void flush_bits(lwmFastUInt8 bits);

		lwmUInt32 show_bits1();
		lwmUInt32 show_bits2();
		lwmUInt32 show_bits3();
		lwmUInt32 show_bits4();
		lwmUInt32 show_bits5();
		lwmUInt32 show_bits6();
		lwmUInt32 show_bits7();
		lwmUInt32 show_bits8();
		lwmUInt32 show_bits9();
		lwmUInt32 show_bits10();
		lwmUInt32 show_bits11();
		lwmUInt32 show_bits12();
		lwmUInt32 show_bits13();
		lwmUInt32 show_bits14();
		lwmUInt32 show_bits15();
		lwmUInt32 show_bits16();
		lwmUInt32 show_bits17();
		lwmUInt32 show_bits18();
		lwmUInt32 show_bits19();
		lwmUInt32 show_bits20();
		lwmUInt32 show_bits21();
		lwmUInt32 show_bits22();
		lwmUInt32 show_bits23();
		lwmUInt32 show_bits24();
		lwmUInt32 show_bits25();
		lwmUInt32 show_bits26();
		lwmUInt32 show_bits27();
		lwmUInt32 show_bits28();
		lwmUInt32 show_bits29();
		lwmUInt32 show_bits30();
		lwmUInt32 show_bits31();
		lwmUInt32 show_bits32();
		lwmUInt32 show_bitsn(lwmFastUInt8 num);

		lwmUInt32 get_bits1();
		lwmUInt32 get_bits2();
		lwmUInt32 get_bits3();
		lwmUInt32 get_bits4();
		lwmUInt32 get_bits5();
		lwmUInt32 get_bits6();
		lwmUInt32 get_bits7();
		lwmUInt32 get_bits8();
		lwmUInt32 get_bits9();
		lwmUInt32 get_bits10();
		lwmUInt32 get_bits11();
		lwmUInt32 get_bits12();
		lwmUInt32 get_bits13();
		lwmUInt32 get_bits14();
		lwmUInt32 get_bits15();
		lwmUInt32 get_bits16();
		lwmUInt32 get_bits17();
		lwmUInt32 get_bits18();
		lwmUInt32 get_bits19();
		lwmUInt32 get_bits20();
		lwmUInt32 get_bits21();
		lwmUInt32 get_bits22();
		lwmUInt32 get_bits23();
		lwmUInt32 get_bits24();
		lwmUInt32 get_bits25();
		lwmUInt32 get_bits26();
		lwmUInt32 get_bits27();
		lwmUInt32 get_bits28();
		lwmUInt32 get_bits29();
		lwmUInt32 get_bits30();
		lwmUInt32 get_bits31();
		lwmUInt32 get_bits32();
		lwmUInt32 get_bitsn(lwmFastUInt8 num);

		bool check_next_bits(lwmUInt8 num, lwmUInt32 mask);
	};
}

#endif

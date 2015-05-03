/*
* Copyright (c) 2015 Eric Lasota
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
#ifndef __LWPLAY_INTERFACES_HPP__
#define __LWPLAY_INTERFACES_HPP__

#include <stdio.h>

#include "../lwmovie/lwmovie.h"
#include "../lwmovie/lwmovie_cpp_shims.hpp"
#include "../lwmovie/lwmovie_cake_cppshims.hpp"

namespace lwplay
{
	class CAllocator : public lwmIAllocator
	{
	public:
		virtual void *Alloc(lwmLargeUInt sz);
		virtual void Free(void *ptr);
	};

	class CFileReader : public lwmICakeFileReader
	{
	public:
		virtual bool IsEOF();
		virtual lwmLargeUInt ReadBytes(void *dest, lwmLargeUInt numBytes);
		explicit CFileReader(FILE *f);

	private:
		FILE *m_f;
	};

	class CTimer : public lwmICakeTimeReader
	{
	public:
		virtual lwmUInt64 GetTimeMilliseconds();
		virtual lwmUInt32 GetResolutionMilliseconds();
	};
}

#endif

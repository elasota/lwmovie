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

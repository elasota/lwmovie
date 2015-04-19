#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdlib.h>
#include "lwplay_interfaces.hpp"

void *lwplay::CAllocator::Alloc(lwmLargeUInt sz)
{
	return _aligned_malloc(sz, 16);
}

void lwplay::CAllocator::Free(void *ptr)
{
	_aligned_free(ptr);
}

bool lwplay::CFileReader::IsEOF()
{
	return (feof(m_f) != 0);
}

lwmLargeUInt lwplay::CFileReader::ReadBytes(void *dest, lwmLargeUInt numBytes)
{
	return fread(dest, 1, numBytes, m_f);
}

lwplay::CFileReader::CFileReader(FILE *f)
	: m_f(f)
{
}

lwmUInt64 lwplay::CTimer::GetTimeMilliseconds()
{
	return GetTickCount64();
}

lwmUInt32 lwplay::CTimer::GetResolutionMilliseconds()
{
	return 10;
}

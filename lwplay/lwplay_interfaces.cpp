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
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <unordered_set>

#include <stdlib.h>
#include "lwplay_interfaces.hpp"

int numAllocations = 0;
int numActiveAllocations = 0;

std::unordered_set<void*> activeAllocs;

void *lwplay::CAllocator::Realloc(void *ptr, lwmLargeUInt sz)
{
	if (ptr == NULL && sz == 0)
		return NULL;

	void *outPtr = _aligned_realloc(ptr, sz, 16);

	if (ptr == NULL)
	{
		if (sz != 0)
			activeAllocs.insert(outPtr);
	}
	else
	{
		if (sz == 0)
		{
			std::unordered_set<void*>::iterator element = activeAllocs.find(ptr);
			if (element == activeAllocs.end())
				throw new std::exception();
			activeAllocs.erase(element);
		}
		else
		{
			activeAllocs.erase(ptr);
			activeAllocs.insert(outPtr);
		}
	}
	return outPtr;
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

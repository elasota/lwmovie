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
#ifndef __LWMUX_PLANIO_HPP__
#define __LWMUX_PLANIO_HPP__

#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"

#include <string.h>

template<class T>
inline void lwmWritePlanToFile(const T &input, lwmOSFile *osFile)
{
	lwmUInt8 buffer[lwmPlanHandler<T>::SIZE];
	lwmPlanHandler<T>::Write(input, buffer);
	osFile->WriteBytes(buffer, lwmPlanHandler<T>::SIZE);
}

template<class T>
inline bool lwmReadPlanFromFile(T &input, lwmOSFile *osFile)
{
	lwmUInt8 buffer[lwmPlanHandler<T>::SIZE];
	if(osFile->ReadBytes(buffer, lwmPlanHandler<T>::SIZE) != lwmPlanHandler<T>::SIZE)
		return false;
	return lwmPlanHandler<T>::Read(input, buffer);
}

inline void EncodeMetaID(const char *metaID, lwmUInt32 metaIDChunks[2])
{
	size_t len = strlen(metaID);
	if(len > 8)
		len = 8;
	metaIDChunks[0] = metaIDChunks[1] = 0;
	for (size_t i = 0; i < len; i++)
		metaIDChunks[i / 4] |= static_cast<lwmUInt8>(metaID[i]) << ((i % 4) * 8);
}

#endif

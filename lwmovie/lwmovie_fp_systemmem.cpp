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
#include <new>
#include "lwmovie.h"
#include "lwmovie_cpp_shims.hpp"
#include "lwmovie_simd_defs.hpp"

namespace lwmovie
{
	class CSystemMemFrameProvider : public lwmIVideoFrameProvider
	{
	public:
		explicit CSystemMemFrameProvider(lwmSAllocator *alloc);
		~CSystemMemFrameProvider();

		virtual int CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);
		virtual void LockWorkFrame(lwmUInt32 workFrameIndex, lwmEVideoLockType lockType);
		virtual void UnlockWorkFrame(lwmUInt32 workFrameIndex);
		virtual void *GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch);
		virtual lwmUInt32 GetWorkFramePlaneWidth(lwmUInt32 planeIndex) const;
		virtual lwmUInt32 GetWorkFramePlaneHeight(lwmUInt32 planeIndex) const;
		virtual void Destroy();

	private:
		static const lwmLargeUInt MAX_CHANNELS = 3;

		lwmUInt8 *m_frameBytes;
		lwmLargeUInt m_frameSize;
		lwmLargeUInt m_channelOffsets[MAX_CHANNELS];
		lwmLargeUInt m_channelStrides[MAX_CHANNELS];
		lwmSAllocator *m_alloc;

		lwmUInt32 m_channelWidths[MAX_CHANNELS];
		lwmUInt32 m_channelHeights[MAX_CHANNELS];
	};
}

lwmovie::CSystemMemFrameProvider::CSystemMemFrameProvider(lwmSAllocator *alloc)
	: lwmIVideoFrameProvider()
	, m_frameBytes(NULL)
{
	m_alloc = alloc;
}

lwmovie::CSystemMemFrameProvider::~CSystemMemFrameProvider()
{
	if(m_frameBytes)
		m_alloc->Free(m_frameBytes);
}

int lwmovie::CSystemMemFrameProvider::CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat)
{
	lwmLargeUInt channelStrides[MAX_CHANNELS];
	lwmLargeUInt channelOffsets[MAX_CHANNELS + 1];
	lwmUInt32 channelWidths[MAX_CHANNELS];
	lwmUInt32 channelHeights[MAX_CHANNELS];
	lwmLargeUInt numChannels;

	switch(frameFormat)
	{
	case lwmFRAMEFORMAT_8Bit_420P_Planar:
		channelStrides[0] = workFrameWidth;
		channelStrides[1] = channelStrides[2] = (workFrameWidth + 1) / 2;
		channelWidths[0] = workFrameWidth;
		channelHeights[0] = workFrameHeight;
		channelWidths[1] = channelWidths[2] = workFrameWidth / 2;
		channelHeights[1] = channelHeights[2] = workFrameHeight / 2;
		numChannels = 3;
		break;
	case lwmFRAMEFORMAT_8Bit_3Channel_Planar:
		for (int i = 0; i < 3; i++)
		{
			channelStrides[i] = workFrameWidth;
			channelWidths[i] = workFrameWidth;
			channelHeights[i] = workFrameHeight;
		}
		numChannels = 3;
		break;
	case lwmFRAMEFORMAT_8Bit_4Channel_Interleaved:
		channelStrides[0] = workFrameWidth * 4;
		channelWidths[0] = workFrameWidth;
		channelHeights[0] = workFrameHeight;
		numChannels = 1;
		break;
	default:
		return 0;
	};

	channelOffsets[0] = 0;
	for(lwmLargeUInt i=0;i<numChannels;i++)
	{
		if (UINT32_MAX - (lwmovie::SIMD_ALIGN - 1) < channelStrides[i])
			return 0;

		channelStrides[i] += (lwmovie::SIMD_ALIGN - 1);
		channelStrides[i] -= channelStrides[i] % lwmovie::SIMD_ALIGN;

		if (UINT32_MAX / channelHeights[i] < channelStrides[i])
			return 0;

		if (UINT32_MAX - channelStrides[i] * channelHeights[i] < channelOffsets[i])
			return 0;

		channelOffsets[i + 1] = channelOffsets[i] + channelStrides[i] * channelHeights[i];
	}
	
	for(lwmLargeUInt i=0;i<numChannels;i++)
	{
		m_channelOffsets[i] = channelOffsets[i];
		m_channelStrides[i] = channelStrides[i];
		m_channelWidths[i] = channelWidths[i];
		m_channelHeights[i] = channelHeights[i];
	}

	m_frameSize = channelOffsets[numChannels];

	if (UINT32_MAX - numRWFrames < numWriteOnlyFrames)
		return 0;

	if (m_frameSize == 0)
		return 0;

	if (UINT32_MAX / m_frameSize < (numRWFrames * numWriteOnlyFrames))
		return 0;

	m_frameBytes = m_alloc->NAlloc<lwmUInt8>(m_frameSize * (numRWFrames + numWriteOnlyFrames));
	if(!m_frameBytes)
		return 0;

	return 1;
}

void lwmovie::CSystemMemFrameProvider::LockWorkFrame(lwmUInt32 workFrameIndex, lwmEVideoLockType lockType)
{
}

void lwmovie::CSystemMemFrameProvider::UnlockWorkFrame(lwmUInt32 workFrameIndex)
{
}

void *lwmovie::CSystemMemFrameProvider::GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch)
{
	if(outPitch)
		*outPitch = m_channelStrides[planeIndex];
	return m_frameBytes + workFrameIndex * m_frameSize + m_channelOffsets[planeIndex];
}

lwmUInt32 lwmovie::CSystemMemFrameProvider::GetWorkFramePlaneWidth(lwmUInt32 planeIndex) const
{
	return this->m_channelWidths[planeIndex];
}

lwmUInt32 lwmovie::CSystemMemFrameProvider::GetWorkFramePlaneHeight(lwmUInt32 planeIndex) const
{
	return this->m_channelHeights[planeIndex];
}

void lwmovie::CSystemMemFrameProvider::Destroy()
{
	CSystemMemFrameProvider *self = this;
	lwmSAllocator *alloc = self->m_alloc;
	self->~CSystemMemFrameProvider();
	alloc->Free(self);
}

///////////////////////////////////////////////////////////////////////////////
// C API
LWMOVIE_API_LINK lwmSVideoFrameProvider *lwmCreateSystemMemoryFrameProvider(lwmSAllocator *alloc, const lwmMovieState *movieState)
{
	lwmovie::CSystemMemFrameProvider *fp = alloc->NAlloc<lwmovie::CSystemMemFrameProvider>(1);
	if(!fp)
		return NULL;
	new (fp)lwmovie::CSystemMemFrameProvider(alloc);
	return fp;
}

LWMOVIE_API_LINK void lwmSVideoFrameProvider_Destroy(lwmSVideoFrameProvider *vfp)
{
	vfp->destroyFunc(vfp);
}

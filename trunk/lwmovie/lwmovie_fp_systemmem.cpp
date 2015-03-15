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
#include "lwmovie_fp.hpp"
#include "lwmovie_simd_defs.hpp"

namespace lwmovie
{
	class lwmCSystemMemFrameProvider : public lwmIVideoFrameProvider
	{
	public:
		explicit lwmCSystemMemFrameProvider(lwmSAllocator *alloc);
		~lwmCSystemMemFrameProvider();

		virtual int CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);
		virtual void LockWorkFrame(lwmUInt32 workFrameIndex, lwmUInt32 lockType);
		virtual void UnlockWorkFrame(lwmUInt32 workFrameIndex);
		virtual void *GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);
		virtual lwmUInt32 GetWorkFramePlaneStride(lwmUInt32 planeIndex);
		virtual void Destroy();

	private:
		static const lwmLargeUInt MAX_CHANNELS = 3;

		lwmUInt8 *m_frameBytes;
		lwmLargeUInt m_frameSize;
		lwmLargeUInt m_channelOffsets[MAX_CHANNELS];
		lwmLargeUInt m_channelStrides[MAX_CHANNELS];
		lwmSAllocator *m_alloc;
	};
}

lwmovie::lwmCSystemMemFrameProvider::lwmCSystemMemFrameProvider(lwmSAllocator *alloc)
	: lwmIVideoFrameProvider()
	, m_frameBytes(NULL)
{
	m_alloc = alloc;
}

lwmovie::lwmCSystemMemFrameProvider::~lwmCSystemMemFrameProvider()
{
	if(m_frameBytes)
		m_alloc->Free(m_frameBytes);
}

int lwmovie::lwmCSystemMemFrameProvider::CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat)
{
	lwmLargeUInt channelStrides[MAX_CHANNELS];
	lwmLargeUInt channelOffsets[MAX_CHANNELS + 1];
	lwmLargeUInt numChannels;

	switch(frameFormat)
	{
	case lwmFRAMEFORMAT_YUV420P_Planar:
		channelStrides[0] = workFrameWidth;
		channelStrides[1] = channelStrides[2] = (workFrameWidth + 1) / 2;
		numChannels = 3;
		break;
	default:
		return 0;
	};

	channelOffsets[0] = 0;
	for(lwmLargeUInt i=0;i<numChannels;i++)
	{
		channelStrides[i] += (lwmovie::SIMD_ALIGN - 1);
		channelStrides[i] -= channelStrides[i] % lwmovie::SIMD_ALIGN;
		channelOffsets[i + 1] = channelOffsets[i] + channelStrides[i] * workFrameHeight;
	}
	
	for(lwmLargeUInt i=0;i<numChannels;i++)
	{
		m_channelOffsets[i] = channelOffsets[i];
		m_channelStrides[i] = channelStrides[i];
	}

	m_frameSize = channelOffsets[numChannels];

	// TODO: Overflow check
	m_frameBytes = m_alloc->NAlloc<lwmUInt8>(m_frameSize * (numRWFrames + numWriteOnlyFrames));
	if(!m_frameBytes)
		return 0;

	return 1;
}

void lwmovie::lwmCSystemMemFrameProvider::LockWorkFrame(lwmUInt32 workFrameIndex, lwmUInt32 lockType)
{
}

void lwmovie::lwmCSystemMemFrameProvider::UnlockWorkFrame(lwmUInt32 workFrameIndex)
{
}

void *lwmovie::lwmCSystemMemFrameProvider::GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex)
{
	return m_frameBytes + workFrameIndex * m_frameSize + m_channelOffsets[planeIndex];
}

lwmUInt32 lwmovie::lwmCSystemMemFrameProvider::GetWorkFramePlaneStride(lwmUInt32 planeIndex)
{
	return this->m_channelStrides[planeIndex];
}

void lwmovie::lwmCSystemMemFrameProvider::Destroy()
{
	lwmCSystemMemFrameProvider *self = this;
	lwmSAllocator *alloc = self->m_alloc;
	self->~lwmCSystemMemFrameProvider();
	alloc->Free(self);
}


LWMOVIE_API_LINK lwmSVideoFrameProvider *lwmCreateSystemMemoryFrameProvider(lwmSAllocator *alloc, const lwmMovieState *movieState)
{
	lwmovie::lwmCSystemMemFrameProvider *fp = alloc->NAlloc<lwmovie::lwmCSystemMemFrameProvider>(1);
	if(!fp)
		return NULL;
	new (fp) lwmovie::lwmCSystemMemFrameProvider(alloc);
	return fp;
}

LWMOVIE_API_LINK void lwmSVideoFrameProvider_Destroy(lwmSVideoFrameProvider *vfp)
{
	vfp->destroyFunc(vfp);
}

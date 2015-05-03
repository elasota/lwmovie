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
#ifndef __LWMOVIE_AUDIOBUFFER_HPP__
#define __LWMOVIE_AUDIOBUFFER_HPP__

#include "../common/lwmovie_coretypes.h"

struct lwmSAllocator;

namespace lwmovie
{
	class CAudioBuffer
	{
	public:
		explicit CAudioBuffer(lwmSAllocator *alloc);
		~CAudioBuffer();
		bool Init(lwmSAllocator *alloc, lwmUInt32 numSamples, lwmUInt8 numChannels);
		lwmUInt8 GetNumChannels() const;
		lwmUInt32 GetNumCommittedSamples() const;
		lwmUInt32 GetNumUncommittedSamples() const;
		void CommitSamples();
		void ClearAll();
		void ClearCommittedSamples();
		lwmUInt32 ReadCommittedSamples(void *output, lwmUInt32 numSamples);

		// Reserves a contiguous block of samples at the end of the ring.  Never fails if the buffer is large enough, may delete samples.
		void *ReserveNewContiguous(lwmUInt32 numSamples, lwmUInt32 &outNumDroppedSamples);
		void SkipSamples(lwmUInt32 numSamples);

	private:
		// Synchronization info
		lwmUInt32 m_startPeriod;
		lwmUInt32 m_capacity;

		lwmUInt32 m_topOffset;
		lwmUInt32 m_topSize;
		lwmUInt32 m_bottomOffset;
		lwmUInt32 m_bottomSize;

		lwmUInt32 m_numCommittedSamples;

		lwmSAllocator *m_alloc;
		void *m_samples;
		lwmUInt8 m_numChannels;
		lwmUInt8 m_sampleSizeBytes;

		void *SampleMemAtPos(lwmUInt32 pos);
	};
}

inline lwmUInt8 lwmovie::CAudioBuffer::GetNumChannels() const
{
	return m_numChannels;
}

inline lwmUInt32 lwmovie::CAudioBuffer::GetNumCommittedSamples() const
{
	return m_numCommittedSamples;
}

inline lwmUInt32 lwmovie::CAudioBuffer::GetNumUncommittedSamples() const
{
	return m_topSize + m_bottomSize - m_numCommittedSamples;
}

inline void lwmovie::CAudioBuffer::CommitSamples()
{
	m_numCommittedSamples = m_topSize + m_bottomSize;
}

inline void lwmovie::CAudioBuffer::ClearAll()
{
	m_numCommittedSamples = m_topSize = m_bottomSize = 0;
}

inline void lwmovie::CAudioBuffer::ClearCommittedSamples()
{
	SkipSamples(m_numCommittedSamples);
}

inline void *lwmovie::CAudioBuffer::SampleMemAtPos(lwmUInt32 pos)
{
	return static_cast<lwmUInt8*>(m_samples) + pos * m_sampleSizeBytes;
}

#endif

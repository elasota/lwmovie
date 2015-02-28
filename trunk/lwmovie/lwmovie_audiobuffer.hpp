#ifndef __LWMOVIE_AUDIOBUFFER_HPP__
#define __LWMOVIE_AUDIOBUFFER_HPP__

#include "../common/lwmovie_coretypes.h"

struct lwmSAllocator;

class lwmCAudioBuffer
{
public:
	explicit lwmCAudioBuffer(lwmSAllocator *alloc);
	~lwmCAudioBuffer();
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

inline lwmUInt8 lwmCAudioBuffer::GetNumChannels() const
{
	return m_numChannels;
}

inline lwmUInt32 lwmCAudioBuffer::GetNumCommittedSamples() const
{
	return m_numCommittedSamples;
}

inline lwmUInt32 lwmCAudioBuffer::GetNumUncommittedSamples() const
{
	return m_topSize + m_bottomSize - m_numCommittedSamples;
}

inline void lwmCAudioBuffer::CommitSamples()
{
	m_numCommittedSamples = m_topSize + m_bottomSize;
}

inline void lwmCAudioBuffer::ClearAll()
{
	m_numCommittedSamples = m_topSize = m_bottomSize = 0;
}

inline void lwmCAudioBuffer::ClearCommittedSamples()
{
	SkipSamples(m_numCommittedSamples);
}

inline void *lwmCAudioBuffer::SampleMemAtPos(lwmUInt32 pos)
{
	return static_cast<lwmUInt8*>(m_samples) + pos * m_sampleSizeBytes;
}

#endif

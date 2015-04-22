#include <stdlib.h>
#include <string.h>

#include "lwmovie_audiobuffer.hpp"
#include "lwmovie_external_types.h"

lwmovie::CAudioBuffer::CAudioBuffer(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_startPeriod(0)
	, m_topOffset(0)
	, m_topSize(0)
	, m_bottomOffset(0)
	, m_bottomSize(0)
	, m_numChannels(0)
	, m_sampleSizeBytes(0)
	, m_samples(NULL)
	, m_capacity(0)
	, m_numCommittedSamples(0)
{
}

lwmovie::CAudioBuffer::~CAudioBuffer()
{
	if(m_samples)
		m_alloc->Free(m_samples);
}


bool lwmovie::CAudioBuffer::Init(lwmSAllocator *alloc, lwmUInt32 numSamples, lwmUInt8 numChannels)
{
	lwmUInt32 intMax = ~static_cast<lwmUInt32>(0);
	m_sampleSizeBytes = 2 * numChannels;
	if(intMax / m_sampleSizeBytes < numSamples)
		return false;	// Overflow
	m_alloc = alloc;
	m_samples = alloc->NAlloc<lwmUInt8>(m_sampleSizeBytes * numSamples);
	m_numChannels = numChannels;
	m_capacity = numSamples;
	if(m_samples == NULL)
		return false;
	return true;
}

lwmUInt32 lwmovie::CAudioBuffer::ReadCommittedSamples(void *output, lwmUInt32 numSamples)
{
	if(numSamples > m_numCommittedSamples)
		numSamples = m_numCommittedSamples;
	m_numCommittedSamples -= numSamples;

	lwmUInt32 numTopDecoded = 0;
	if(m_topSize)
	{
		const void *topMem = SampleMemAtPos(m_topOffset);
		if(m_topSize >= numSamples)
		{
			m_topSize -= numSamples;
			m_topOffset += numSamples;

			memcpy(output, topMem, m_sampleSizeBytes * numSamples);
			return numSamples;
		}
		else
		{
			memcpy(output, topMem, m_sampleSizeBytes * m_topSize);
			numTopDecoded = m_topSize;
			numSamples -= m_topSize;
			output = static_cast<lwmUInt8*>(output) + m_topSize * m_sampleSizeBytes;
			m_topSize = 0;
		}
	}

	if(m_bottomSize)
	{
		const void *bottomMem = SampleMemAtPos(m_bottomOffset);
		if(m_bottomSize >= numSamples)
		{
			m_bottomSize -= numSamples;
			m_bottomOffset += numSamples;
			memcpy(output, bottomMem, m_sampleSizeBytes * numSamples);
			return numTopDecoded + numSamples;
		}
		else
		{
			memcpy(output, bottomMem, m_sampleSizeBytes * m_bottomSize);
			lwmUInt32 numBottomDecoded = m_bottomSize;
			m_bottomSize = 0;
			return numBottomDecoded + numTopDecoded;
		}
	}
	else
	{
		return numTopDecoded;
	}
}

void *lwmovie::CAudioBuffer::ReserveNewContiguous(lwmUInt32 numSamples, lwmUInt32 &outNumDroppedSamples)
{
	if(numSamples > m_capacity)
		return NULL;

	lwmUInt32 available = m_capacity - m_topSize - m_bottomSize;
	if(available < numSamples)
	{
		lwmUInt32 numDropped = numSamples - available;
		if(numDropped >= m_numCommittedSamples)
			m_numCommittedSamples = 0;
		else
			m_numCommittedSamples -= numDropped;
		SkipSamples(numDropped);
		outNumDroppedSamples = numDropped;
	}
	else
	{
		outNumDroppedSamples = 0;
	}

	if(m_bottomSize == 0)
		m_bottomOffset = 0;

	if(m_topSize == 0)
	{
		// Single chunk in the ring buffer
		// See if there is space at the end of bottom
		if(m_capacity - m_bottomOffset - m_bottomSize >= numSamples)
		{
			// Reserve at the end of bottom
			void *loc = SampleMemAtPos(m_bottomOffset + m_bottomSize);
			m_bottomSize += numSamples;
			return loc;
		}
		// See if there is space below bottom, which would cause bottom to become top
		if(m_bottomOffset >= numSamples)
		{
			m_topOffset = m_bottomOffset;
			m_topSize = m_bottomSize;
			m_bottomOffset = 0;
			m_bottomSize = numSamples;

			return m_samples;
		}

		// Move the bottom to zero
		memmove(m_samples, SampleMemAtPos(m_bottomOffset), m_bottomSize * m_sampleSizeBytes);
		m_bottomOffset = 0;
		void *loc = SampleMemAtPos(0 + m_bottomSize);
		m_bottomSize += numSamples;
		return loc;
	}
	else
	{
		// 2 chunks in the ring buffer
		// See if there is space between the chunks
		if(m_topOffset - m_bottomOffset - m_bottomSize)
		{
			void *loc = SampleMemAtPos(m_bottomOffset + m_bottomSize);
			m_bottomSize += numSamples;
			return loc;
		}
		// There isn't, need to relocate both chunks to the edges
		// TODO: Might be able to merge by moving top below bottom if the reserve fits above
		memmove(m_samples, SampleMemAtPos(m_bottomOffset), m_bottomSize * m_sampleSizeBytes);
		m_bottomOffset = 0;
		memmove(SampleMemAtPos(m_capacity - m_topSize), SampleMemAtPos(m_topOffset), m_topSize * m_sampleSizeBytes);
		m_topOffset = m_capacity - m_topSize;
		void *loc = SampleMemAtPos(0 + m_bottomSize);
		m_bottomSize += numSamples;
		return loc;
	}
}

void lwmovie::CAudioBuffer::SkipSamples(lwmUInt32 numSamples)
{
	if(numSamples >= m_numCommittedSamples)
		m_numCommittedSamples = 0;
	else
		m_numCommittedSamples -= numSamples;

	if(m_topSize >= numSamples)
	{
		m_topOffset += numSamples;
		m_topSize -= numSamples;
		return;
	}
	numSamples -= m_topSize;
	m_topSize = 0;

	if(m_bottomSize >= numSamples)
	{
		m_bottomOffset += numSamples;
		m_bottomSize -= numSamples;
		return;
	}
	else
	{
		m_bottomOffset = 0;
		m_bottomSize = 0;
	}
}

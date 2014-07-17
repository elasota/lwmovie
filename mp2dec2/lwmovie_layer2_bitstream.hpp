#ifndef __LWMOVIE_LAYER2_BITSTREAM_HPP__
#define __LWMOVIE_LAYER2_BITSTREAM_HPP__

#include "lwmovie_layer2.hpp"

namespace lwmovie
{
	namespace layerii
	{
		class lwmCMemBitstream
		{
			const lwmUInt8	*m_bytes;
			lwmFastUInt8	m_bitOffs;
			lwmLargeUInt	m_totalBits;

		public:
			lwmCMemBitstream(const void *bytes);

			lwmUInt32 Read(lwmFastUInt8 bits);
			void SkipBytes(lwmLargeUInt numBytes);
			static lwmUInt32 BitMask32(lwmFastUInt8 offset, lwmFastUInt8 length);
			inline lwmLargeUInt GetTotalBits() const { return m_totalBits; }
		};
	}
}

inline lwmovie::layerii::lwmCMemBitstream::lwmCMemBitstream(const void *bytes)
{
	m_bytes = static_cast<const lwmUInt8 *>(bytes);
	m_bitOffs = 0;
	m_totalBits = 0;
}

inline void lwmovie::layerii::lwmCMemBitstream::SkipBytes(lwmLargeUInt numBytes)
{
	m_bytes += numBytes;
}

inline lwmUInt32 lwmovie::layerii::lwmCMemBitstream::BitMask32(lwmFastUInt8 offset, lwmFastUInt8 length)
{
	lwmUInt32 maskBits = ~(0xFFFFFFFF << length);
	return maskBits << offset;
}

inline lwmUInt32 lwmovie::layerii::lwmCMemBitstream::Read(lwmFastUInt8 bits)
{
	lwmUInt32 result = 0;
	lwmFastUInt8 resultBitOffs = bits;
	lwmFastUInt8 streamBitOffs = m_bitOffs;
	const lwmUInt8 *bytes = static_cast<const lwmUInt8 *>(m_bytes);
	lwmFastUInt8 originalBits = bits;	// Compiler optimization assist
	m_totalBits += bits;

	if(bits == 1)
	{
		result = ((bytes[0] & (1 << (7 - (streamBitOffs)))) >> (7 - (streamBitOffs)));

		if(streamBitOffs == 7)
		{
			m_bytes++;
			m_bitOffs = 0;
		}
		else
			m_bitOffs++;
		return result;
	}

	if(streamBitOffs != 0)
	{
		lwmUInt8 availBits = 8 - streamBitOffs;
		if(bits >= availBits)
		{
			// Longer than the first byte
			resultBitOffs -= availBits;
			lwmUInt32 bitMask = BitMask32(0, availBits);
			result = (bytes[0] & BitMask32(0, availBits)) << resultBitOffs;
			bits -= availBits;
			streamBitOffs = 0;
			bytes++;
		}
		else
		{
			result = (bytes[0] & BitMask32((availBits - bits), bits)) >> (availBits - bits);

			m_bitOffs = streamBitOffs + bits;
			return result;
		}
	}

	if(originalBits >= 8)	// Compiler optimization assist
	{
		while(bits >= 8)
		{
			resultBitOffs -= 8;
			result |= (bytes[0] << resultBitOffs);
			bits -= 8;
			bytes++;
		}
	}

	if(bits != 0)
	{
		result |= (bytes[0] & BitMask32(8 - bits, bits)) >> (8 - bits);
		streamBitOffs = bits;
	}

	m_bytes = bytes;
	m_bitOffs = streamBitOffs;

	return result;
}

#endif

#ifndef __LWMOVIE_LAYER2_DECODESTATE_HPP__
#define __LWMOVIE_LAYER2_DECODESTATE_HPP__

#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_constants.hpp"
#include "lwmovie_layer2_dspops.hpp"

namespace lwmovie
{
	namespace layerii
	{
		class lwmCMP2DecodeState
		{
			lwmFixedReal22	m_accumulators[MAX_CHANNELS][FILTER_SIZE][NUM_SUBBANDS];
			int				m_sbsRotator;

			lwmUInt8		m_tableSet;	// 0 = 0/1, 1 = 2/3
			bool			m_hasPadding;
			bool			m_hasChecksum;
			bool			m_isJointStereo;
			//lwmUInt8		m_jsLimit;
			lwmUInt16		m_sampleRate;
			lwmUInt16		m_bitrateKbps;
			lwmUInt8		m_bitrateIndex;
			lwmUInt8		m_samplerateIndex;
			lwmUInt8		m_numChannels;
			lwmUInt8		m_modeExt;
			EMPEGVersion	m_mpegVersion;

		public:
			lwmCMP2DecodeState();
			bool ParseHeader(const void *bytes);
			
			template<lwmFastUInt8 numChannels>
			bool DecodeFrameCH(const void *bytes, lwmSInt16 *output);

			bool DecodeFrame(const void *bytes, lwmSInt16 *output);

			lwmUInt16 GetFrameSizeBytes() const;
		};
	}
}

#endif

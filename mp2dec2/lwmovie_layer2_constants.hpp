#ifndef __LWMOVIE_LAYER2_CONSTANTS_HPP__
#define __LWMOVIE_LAYER2_CONSTANTS_HPP__

#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_fixedreal.hpp"
#include "lwmovie_layer2_csf.hpp"

namespace lwmovie
{
	namespace layerii
	{
		static const int NUM_SUBBANDS			= 32;
		static const int FILTER_SIZE			= 16;
		static const int MAX_CHANNELS			= 2;
		static const int MAX_BITALLOC			= 4;
		static const int SCALE_BLOCK_SIZE		= 12;

		static const int OUT_SCALE_SHIFT		= 15;
		static const int OUT_SCALE_MAX			= ((1 << OUT_SCALE_SHIFT)-1);
		static const int OUT_SCALE_MIN			= (-(1 << OUT_SCALE_SHIFT));
		static const int NUM_BITRATE_INDEXES	= 15;
		static const int NUM_SAMPLERATE_INDEXES	= 3;
		static const int NUM_TABLESETS			= 2;
		static const int NUM_QINDEX_TABLES		= 6;
		static const int NUM_QINDEX				= 17;

		static const int MAX_FRAME_SIZE_BYTES	= 4800;		// Maximum size of a frame in bytes, including header
		static const int HEADER_SIZE_BYTES		= 4;		// Size of a frame header in bytes
		static const int FRAME_NUM_SAMPLES		= 1152;

		enum EMPEGVersion
		{
			EMPEGVersion_MPEG1,
			EMPEGVersion_MPEG2,

			EMPEGVersion_Count,
		};

		struct MP2QuantInfo
		{
			lwmUInt8 codeLength;
			lwmUInt8 modGroup;

			// Generated
			lwmFixedReal29 rcp;
		};

		extern const lwmUInt16	MP2_BITRATE_KBPS[EMPEGVersion_Count][NUM_BITRATE_INDEXES];
		extern const lwmUInt8	MP2_TABLESET[EMPEGVersion_Count][MAX_CHANNELS][NUM_BITRATE_INDEXES];
		extern const lwmUInt16	MP2_SAMPLERATE[EMPEGVersion_Count][NUM_SAMPLERATE_INDEXES];
		extern const lwmUInt8	MP2_SBLIMITS_MPEG1[MAX_CHANNELS][NUM_SAMPLERATE_INDEXES][NUM_BITRATE_INDEXES];
		extern const lwmUInt8	MP2_BITALLOC_SIZE[NUM_TABLESETS][NUM_SUBBANDS];
		extern lwmCompressedSF	MP2_SCALEFACTORS[64];

		extern const lwmUInt8 MP2_QINDEX_TABLE_SELECT[NUM_TABLESETS][NUM_SUBBANDS];
		extern const lwmUInt8 MP2_QINDEX[NUM_QINDEX_TABLES][1 << MAX_BITALLOC];
		extern const lwmUInt8 MP2_QUANT_GROUP[NUM_QINDEX_TABLES][NUM_QINDEX];
		extern MP2QuantInfo MP2_QUANT_INFO[NUM_QINDEX];
		extern const lwmFixedReal29 MP2_DEWINDOW[FILTER_SIZE][NUM_SUBBANDS];
	}
}

#endif

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
#ifndef __LWMOVIE_PACKAGE_HPP__
#define __LWMOVIE_PACKAGE_HPP__

#include "../common/lwmovie_coretypes.h"
#include "../lwmovie/lwmovie_packetplan.hpp"
#include "../lwmovie/lwmovie.h"		// Needed for some enums

enum lwmEVersion
{
	lwmVERSION	= 2,
};

enum lwmEVideoStreamType
{
	lwmVST_Invalid		= 0,

	lwmVST_None				= 1,
	lwmVST_M1V_Variant		= 2,		// Variant M1V
	lwmVST_RoQ				= 3,		// RoQ VQ
	lwmVST_Theora_Variant	= 4,		// Variant Theora

	lwmVST_Count,
};

enum lwmEAudioStreamType
{
	lwmAST_Invalid		= 0,
	lwmAST_None			= 1,
	lwmAST_MP2			= 2,	// MPEG layer II
	lwmAST_OpusCustom	= 3,	// Opus Custom (1024 frame size)
	lwmAST_ADPCM		= 4,	// ADPCM

	lwmAST_Count,
};

struct lwmMovieHeader
{
	lwmEVideoStreamType	videoStreamType;
	lwmEAudioStreamType	audioStreamType;
	lwmUInt32			largestPacketSize;
	lwmUInt32			longestFrameReadahead;
	lwmUInt16			numTOC;
};
LWM_DECLARE_PLAN_SENTINEL			(0, lwmMovieHeader, lwmUInt32, 0x6c774d56);
LWM_DECLARE_PLAN_SENTINEL			(1, lwmMovieHeader, lwmUInt32, lwmVERSION);
LWM_DECLARE_PLAN_SENTINEL			(2, lwmMovieHeader, lwmUInt8, 0xff);
LWM_DECLARE_PLAN_ENUM_MEMBER		(3, lwmMovieHeader, lwmEVideoStreamType, lwmVST_Count, lwmUInt8, videoStreamType);
LWM_DECLARE_PLAN_ENUM_MEMBER		(4, lwmMovieHeader, lwmEAudioStreamType, lwmAST_Count, lwmUInt8, audioStreamType);
LWM_DECLARE_PLAN_MEMBER				(5, lwmMovieHeader, lwmUInt32, largestPacketSize);
LWM_DECLARE_PLAN_MEMBER				(6, lwmMovieHeader, lwmUInt16, numTOC);
LWM_DECLARE_PLAN_MEMBER				(7, lwmMovieHeader, lwmUInt32, longestFrameReadahead);
LWM_DECLARE_PLAN(lwmMovieHeader);

struct lwmVideoStreamInfo
{
	lwmUInt16 videoWidth;
	lwmUInt16 videoHeight;
	lwmUInt32 periodsPerSecondNum;
	lwmUInt16 periodsPerSecondDenom;
	lwmUInt8 numReadWriteWorkFrames;
	lwmUInt8 numWriteOnlyWorkFrames;
	lwmEFrameFormat frameFormat;
	lwmEVideoChannelLayout channelLayout;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmVideoStreamInfo, lwmUInt16, videoWidth);
LWM_DECLARE_PLAN_MEMBER_NONZERO	(1, lwmVideoStreamInfo, lwmUInt16, videoHeight);
LWM_DECLARE_PLAN_MEMBER_NONZERO	(2, lwmVideoStreamInfo, lwmUInt32, periodsPerSecondNum);
LWM_DECLARE_PLAN_MEMBER_NONZERO	(3, lwmVideoStreamInfo, lwmUInt16, periodsPerSecondDenom);
LWM_DECLARE_PLAN_MEMBER			(4, lwmVideoStreamInfo, lwmUInt8, numReadWriteWorkFrames);
LWM_DECLARE_PLAN_MEMBER			(5, lwmVideoStreamInfo, lwmUInt8, numWriteOnlyWorkFrames);
LWM_DECLARE_PLAN_ENUM_MEMBER	(6, lwmVideoStreamInfo, lwmEFrameFormat, lwmFRAMEFORMAT_Count, lwmUInt8, frameFormat);
LWM_DECLARE_PLAN_ENUM_MEMBER	(7, lwmVideoStreamInfo, lwmEVideoChannelLayout, lwmVIDEOCHANNELLAYOUT_Count, lwmUInt8, channelLayout);
LWM_DECLARE_PLAN(lwmVideoStreamInfo);

struct lwmAudioCommonInfo
{
	lwmUInt32			sampleRate;
	lwmUInt32			audioReadAhead;
	lwmUInt8			numAudioStreams;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmAudioCommonInfo, lwmUInt32, sampleRate);
LWM_DECLARE_PLAN_MEMBER			(1, lwmAudioCommonInfo, lwmUInt32, audioReadAhead);
LWM_DECLARE_PLAN_MEMBER_NONZERO	(2, lwmAudioCommonInfo, lwmUInt8, numAudioStreams);
LWM_DECLARE_PLAN(lwmAudioCommonInfo);


struct lwmAudioStreamInfo
{
	lwmESpeakerLayout	speakerLayout;
	lwmUInt32			metaID[2];
};
LWM_DECLARE_PLAN_ENUM_MEMBER	(0, lwmAudioStreamInfo, lwmESpeakerLayout, lwmSPEAKERLAYOUT_Count, lwmUInt8, speakerLayout);
LWM_DECLARE_PLAN_MEMBER			(1, lwmAudioStreamInfo, lwmUInt32, metaID[0]);
LWM_DECLARE_PLAN_MEMBER			(2, lwmAudioStreamInfo, lwmUInt32, metaID[1]);
LWM_DECLARE_PLAN(lwmAudioStreamInfo);

enum lwmEPacketType
{
	lwmEPT_Video_StreamParameters	= 1,
	lwmEPT_Video_InlinePacket,
	lwmEPT_Video_Synchronization,
	
	lwmEPT_Audio_StreamParameters,
	lwmEPT_Audio_Frame,
	lwmEPT_Audio_Synchronization,

	lwmEPT_Count,
};

struct lwmPacketHeader
{
	enum EFlag
	{
		EFlag_Escaped		= 0x80,
		EFlag_Checksum		= 0x40,
		EFlag_Compact		= 0x20,

		EFlag_All			= 0xe0,
	};
	lwmUInt8		packetTypeAndFlags;

	inline lwmEPacketType GetPacketType() const { return static_cast<lwmEPacketType>(packetTypeAndFlags & ~(EFlag_All)); }
};
LWM_DECLARE_PLAN_SENTINEL		(0, lwmPacketHeader, lwmUInt32, 0x6c77504b);
LWM_DECLARE_PLAN_MEMBER			(1, lwmPacketHeader, lwmUInt8, packetTypeAndFlags);
LWM_DECLARE_PLAN(lwmPacketHeader);

struct lwmPacketHeaderFull
{
	lwmUInt32		packetSize;
	lwmUInt8		streamIndex;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmPacketHeaderFull, lwmUInt32, packetSize);
LWM_DECLARE_PLAN_MEMBER			(1, lwmPacketHeaderFull, lwmUInt8, streamIndex);
LWM_DECLARE_PLAN(lwmPacketHeaderFull);

struct lwmPacketHeaderCompact
{
	lwmUInt16		packetSize;
};
LWM_DECLARE_PLAN_MEMBER			(0, lwmPacketHeaderCompact, lwmUInt16, packetSize);
LWM_DECLARE_PLAN(lwmPacketHeaderCompact);

struct lwmVideoSynchronizationPoint
{
	enum
	{
		EFlag_AlwaysSet		= 1,

		EFlag_RandomAccess	= 2,
	};

	lwmUInt32 videoPeriod;	// Time of decoded ready-to-display video
	lwmUInt8 flags;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmVideoSynchronizationPoint, lwmUInt32, videoPeriod);
LWM_DECLARE_PLAN_MEMBER_NONZERO	(1, lwmVideoSynchronizationPoint, lwmUInt8, flags);
LWM_DECLARE_PLAN(lwmVideoSynchronizationPoint);

struct lwmAudioSynchronizationPoint
{
	lwmUInt32 audioPeriod;	// Time of decoded ready-to-display audio
};
LWM_DECLARE_PLAN_MEMBER_NONZERO(0, lwmAudioSynchronizationPoint, lwmUInt32, audioPeriod);
LWM_DECLARE_PLAN(lwmAudioSynchronizationPoint);


#endif

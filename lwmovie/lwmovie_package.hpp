#ifndef __LWMOVIE_PACKAGE_HPP__
#define __LWMOVIE_PACKAGE_HPP__

#include "lwmovie_types.hpp"
#include "../lwmovie/lwmovie_packetplan.hpp"

enum lwmEVersion
{
	lwmVERSION	= 1,
};

/*
Version 1 file structure:
MovieHeader
*/

enum lwmEVideoStreamType
{
	lwmVST_Invalid		= 0,

	lwmVST_None			= 1,
	lwmVST_M1V_Variant	= 2,		// Variant M1V
	lwmVST_M1V_RoQ		= 3,		// RoQ

	lwmVST_Count,
};

enum lwmEAudioStreamType
{
	lwmAST_Invalid		= 0,
	lwmAST_None			= 1,	// MPEG layer 2
	lwmAST_MP2			= 2,	// MPEG layer 2
	lwmAST_RoQ_DPCM		= 3,	// RoQ DPCM
	lwmAST_IMA_ADPCM	= 4,	// IMA ADPCM

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
LWM_DECLARE_PLAN_MEMBER				(5, lwmMovieHeader, lwmUInt16, numTOC);
LWM_DECLARE_PLAN_MEMBER				(6, lwmMovieHeader, lwmUInt32, largestPacketSize);
LWM_DECLARE_PLAN_MEMBER				(7, lwmMovieHeader, lwmUInt32, longestFrameReadahead);
LWM_DECLARE_PLAN(lwmMovieHeader);

struct lwmVideoStreamInfo
{
	lwmUInt16 videoWidth;
	lwmUInt16 videoHeight;
	lwmUInt32 periodsPerSecondNum;
	lwmUInt16 periodsPerSecondDenom;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO(0, lwmVideoStreamInfo, lwmUInt16, videoWidth);
LWM_DECLARE_PLAN_MEMBER_NONZERO(1, lwmVideoStreamInfo, lwmUInt16, videoHeight);
LWM_DECLARE_PLAN_MEMBER_NONZERO(2, lwmVideoStreamInfo, lwmUInt32, periodsPerSecondNum);
LWM_DECLARE_PLAN_MEMBER_NONZERO(3, lwmVideoStreamInfo, lwmUInt16, periodsPerSecondDenom);
LWM_DECLARE_PLAN(lwmVideoStreamInfo);


struct lwmAudioStreamInfo
{
	enum EChannelLayout
	{
		CL_Mono		= 1,
		CL_Stereo,

		CL_Count,
	};

	lwmUInt32		sampleRate;
	EChannelLayout	channelLayout;
	lwmUInt32		audioReadAhead;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmAudioStreamInfo, lwmUInt32, sampleRate);
LWM_DECLARE_PLAN_ENUM_MEMBER	(1, lwmAudioStreamInfo, lwmAudioStreamInfo::EChannelLayout, lwmAudioStreamInfo::CL_Count, lwmUInt8, channelLayout);
LWM_DECLARE_PLAN_MEMBER			(2, lwmAudioStreamInfo, lwmUInt32, audioReadAhead);
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
LWM_DECLARE_PLAN_SENTINEL		(0, lwmPacketHeader, lwmUInt16, 0);
LWM_DECLARE_PLAN_SENTINEL		(1, lwmPacketHeader, lwmUInt16, 0x01ff);
LWM_DECLARE_PLAN_MEMBER			(2, lwmPacketHeader, lwmUInt8, packetTypeAndFlags);
LWM_DECLARE_PLAN(lwmPacketHeader);

struct lwmPacketHeaderFull
{
	lwmUInt32		packetSize;
};
LWM_DECLARE_PLAN_MEMBER_NONZERO	(0, lwmPacketHeaderFull, lwmUInt32, packetSize);
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
	lwmUInt8 flags;			// True if the synchronization point is random-access
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

class lwmOSFile;

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

#endif

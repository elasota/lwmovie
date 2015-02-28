#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "../lwmovie/lwmovie_types.hpp"
#include "../lwmovie/lwmovie_constants.hpp"
#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"

using namespace lwmovie;
using namespace lwmovie::constants;

struct SFourCC
{
	lwmUInt8 fcc[4];

	inline bool operator == (const SFourCC &rs) const
	{
		return memcmp(fcc, rs.fcc, 4) == 0;
	}

	inline bool operator != (const SFourCC &rs) const
	{
		return memcmp(fcc, rs.fcc, 4) != 0;
	}

	inline SFourCC()
	{
	}

	inline SFourCC(const SFourCC &rs)
	{
		memcpy(fcc, rs.fcc, 4);
	}

	inline SFourCC(char c1, char c2, char c3, char c4)
	{
		fcc[0] = static_cast<lwmUInt8>(c1);
		fcc[1] = static_cast<lwmUInt8>(c2);
		fcc[2] = static_cast<lwmUInt8>(c3);
		fcc[3] = static_cast<lwmUInt8>(c4);
	}

	inline SFourCC(lwmUInt32 dword)
	{
		fcc[0] = static_cast<lwmUInt8>((dword >> 0) & 0xff);
		fcc[1] = static_cast<lwmUInt8>((dword >> 8) & 0xff);
		fcc[2] = static_cast<lwmUInt8>((dword >> 16) & 0xff);
		fcc[3] = static_cast<lwmUInt8>((dword >> 24) & 0xff);
	}
};



static const SFourCC ATOM_TYPE_RIFF = SFourCC('R', 'I', 'F', 'F');
static const SFourCC ATOM_TYPE_LIST = SFourCC('L', 'I', 'S', 'T');

class CAVIAtom
{
	SFourCC m_atomType;
	lwmUInt32 m_chunkSize;

public:
	CAVIAtom(const SFourCC &atomType, lwmUInt32 chunkSize)
		: m_atomType(atomType)
		, m_chunkSize(chunkSize)
	{
	}

	virtual ~CAVIAtom()
	{
	}

	inline lwmUInt32 ChunkSize() const
	{
		return m_chunkSize;
	}

	inline const SFourCC &AtomType() const
	{
		return m_atomType;
	}
};

class CAVIDataChunk : public CAVIAtom
{
	lwmUInt64 m_dataFileOffset;

public:
	CAVIDataChunk(const SFourCC &atomType, lwmUInt32 dataSize, lwmUInt64 dataFileOffset)
		: CAVIAtom(atomType, dataSize)
		, m_dataFileOffset(dataFileOffset)
	{
	}

	inline lwmUInt64 FileOffset() const
	{
		return m_dataFileOffset;
	}
};

class CAVIDataList : public CAVIAtom
{
	std::vector<CAVIAtom*> m_children;
	SFourCC m_listFourCC;

public:
	CAVIDataList(const SFourCC &atomType, lwmUInt32 chunkSize, SFourCC listFourCC)
		: CAVIAtom(atomType, chunkSize)
		, m_listFourCC(listFourCC)
	{
	}

	virtual ~CAVIDataList()
	{
		for(std::vector<CAVIAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
			delete const_cast<CAVIAtom*>(*it);
	}

	void AddChild(CAVIAtom *child)
	{
		m_children.push_back(child);
	}

	CAVIDataChunk *FindDataChild(const SFourCC &atomType)
	{
		for(std::vector<CAVIAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
			if((*it)->AtomType() == atomType)
				return static_cast<CAVIDataChunk*>(*it);
		return NULL;
	}

	CAVIDataList *FindListChild(const SFourCC &atomType)
	{
		for(std::vector<CAVIAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
		{
			bool isList = ((*it)->AtomType() == ATOM_TYPE_RIFF || (*it)->AtomType() == ATOM_TYPE_LIST);
			if(isList)
			{
				if(static_cast<CAVIDataList*>(*it)->m_listFourCC == atomType)
					return static_cast<CAVIDataList*>(*it);
			}
		}
		return NULL;
	}

	std::vector<CAVIDataList*> FindListChildren(const SFourCC &atomType)
	{
		std::vector<CAVIDataList*> v;
		for(std::vector<CAVIAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
		{
			bool isList = ((*it)->AtomType() == ATOM_TYPE_RIFF || (*it)->AtomType() == ATOM_TYPE_LIST);
			if(isList)
			{
				if(static_cast<CAVIDataList*>(*it)->m_listFourCC == atomType)
					v.push_back(static_cast<CAVIDataList*>(*it));
			}
		}
		return v;
	}

	inline const std::vector<CAVIAtom*> &Children() const
	{
		return m_children;
	}

	inline const SFourCC &ListFourCC() const
	{
		return m_listFourCC;
	}
};

struct CAVIStreamInfo
{
	SFourCC m_fccType;
	SFourCC m_fccHandler;
	lwmUInt32 m_flags;
	lwmUInt16 m_priority;
	lwmUInt16 m_language;
	lwmUInt32 m_initialFrames;
	lwmUInt32 m_scale;
	lwmUInt32 m_rate;	// Rate/Scale = actual rate
	lwmUInt32 m_start;
	lwmUInt32 m_length;
	lwmUInt32 m_suggestedBufferSize;
	lwmUInt32 m_quality;
	lwmUInt32 m_sampleSize;
	lwmUInt32 m_rectLeft;
	lwmUInt32 m_rectTop;
	lwmUInt32 m_rectRight;
	lwmUInt32 m_rectBottom;

	std::vector<CAVIDataChunk*> m_chunks;
};

static lwmUInt32 readUInt16(lwmOSFile *inFile)
{
	lwmUInt8 buf[2];
	inFile->ReadBytes(buf, 2);
	return static_cast<lwmUInt16>(buf[0] | (buf[1] << 8));
}

static lwmUInt32 readUInt32(lwmOSFile *inFile)
{
	lwmUInt8 buf[4];
	inFile->ReadBytes(buf, 4);
	return static_cast<lwmUInt32>(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

CAVIAtom *ParseAtom_r(lwmOSFile *inFile);

CAVIAtom *ParseChunk(lwmOSFile *inFile, SFourCC atomType)
{
	lwmUInt32 chunkSize = readUInt32(inFile);
	lwmUInt64 dataLoc = inFile->FilePos();
	inFile->Seek(static_cast<lwmSInt64>(chunkSize), lwmOSFile::SM_Current);
	return new CAVIDataChunk(atomType, chunkSize, dataLoc);
}

CAVIAtom *ParseList(lwmOSFile *inFile, SFourCC atomType)
{
	lwmUInt32 chunkSize = readUInt32(inFile);
	SFourCC listFourCC = SFourCC(readUInt32(inFile));

	CAVIDataList *dataList = new CAVIDataList(atomType, chunkSize, listFourCC);

	lwmUInt32 listDataSize = chunkSize - 4;
	while(listDataSize != 0)
	{
		CAVIAtom *subChunk = ParseAtom_r(inFile);
		listDataSize -= subChunk->ChunkSize() + 8;
		dataList->AddChild(subChunk);
	}
	return dataList;
}

CAVIAtom *ParseAtom_r(lwmOSFile *inFile)
{
	SFourCC atomType = SFourCC(readUInt32(inFile));
	if(atomType == ATOM_TYPE_RIFF || atomType == ATOM_TYPE_LIST)
		return ParseList(inFile, atomType);
	return ParseChunk(inFile, atomType);
}

void AddChunkToStream(std::vector<CAVIStreamInfo> &streams, CAVIAtom *chunk)
{
	char sid0 = chunk->AtomType().fcc[0];
	char sid1 = chunk->AtomType().fcc[1];
	if(sid0 >= '0' && sid0 <= '9' && sid1 >= '0' && sid1 <= '9')
		streams[(sid0 - '0') * 10 + (sid1 - '0')].m_chunks.push_back(static_cast<CAVIDataChunk*>(chunk));
}

void ConvertCVIDChunk(const CAVIDataChunk *chunk, lwmUInt32 width, lwmUInt32 height, lwmOSFile *inFile, lwmOSFile *outFile)
{
	bool isPFrame = false;
	inFile->Seek(chunk->FileOffset());

	lwmUInt32 dOffset = 0;
	
	lwmUInt8 cvidFrameHeader[10];
	inFile->ReadBytes(cvidFrameHeader, sizeof(cvidFrameHeader));
	lwmUInt8 cvidFrameFlags = cvidFrameHeader[0];
	lwmUInt16 numStrips = static_cast<lwmUInt16>((cvidFrameHeader[8] << 8) | cvidFrameHeader[9]);

	lwmUInt32 cvidSize = static_cast<lwmUInt32>((cvidFrameHeader[1] << 16) | (cvidFrameHeader[2] << 8) | cvidFrameHeader[3]);

	lwmUInt8 minCVidHeader[4];
	minCVidHeader[0] = 0x40;
	minCVidHeader[1] = 0x00;
	minCVidHeader[2] = cvidFrameHeader[8];
	minCVidHeader[3] = cvidFrameHeader[9];
	
	{
		lwmPacketHeader packetHeader;
		lwmPacketHeaderFull packetHeaderFull;
		packetHeader.packetTypeAndFlags = lwmEPT_Video_InlinePacket;
		packetHeaderFull.packetSize = static_cast<lwmUInt32>(sizeof(minCVidHeader));
		lwmWritePlanToFile(packetHeader, outFile);
		lwmWritePlanToFile(packetHeaderFull, outFile);
		outFile->WriteBytes(minCVidHeader, sizeof(minCVidHeader));
	}

	std::vector<lwmUInt8> dirtyBits;

	dirtyBits.resize((width * height + 7) / 8);

	// Copy strips
	for(lwmLargeUInt i=0;i<numStrips;i++)
	{
		lwmUInt8 stripHeader[12];
		lwmUInt8 stripMinHeader[10];
		inFile->ReadBytes(stripHeader, sizeof(stripHeader));
		stripMinHeader[0] = stripHeader[0];
		stripMinHeader[1] = stripHeader[1];
		stripMinHeader[2] = stripHeader[4];
		stripMinHeader[3] = stripHeader[5];
		stripMinHeader[4] = stripHeader[6];
		stripMinHeader[5] = stripHeader[7];
		stripMinHeader[6] = stripHeader[8];
		stripMinHeader[7] = stripHeader[9];
		stripMinHeader[8] = stripHeader[10];
		stripMinHeader[9] = stripHeader[11];
		lwmUInt16 stripDataSize = static_cast<lwmUInt16>((stripHeader[2] << 8) | stripHeader[3]) - sizeof(stripHeader);

		lwmUInt16 stripMinY = static_cast<lwmUInt16>((stripHeader[4] << 8) | stripHeader[5]);
		lwmUInt16 stripMinX = static_cast<lwmUInt16>((stripHeader[6] << 8) | stripHeader[7]);
		lwmUInt16 stripMaxY = static_cast<lwmUInt16>((stripHeader[8] << 8) | stripHeader[9]);
		lwmUInt16 stripMaxX = static_cast<lwmUInt16>((stripHeader[10] << 8) | stripHeader[11]);

		lwmPacketHeader packetHeader;
		lwmPacketHeaderFull packetHeaderFull;
		packetHeader.packetTypeAndFlags = lwmEPT_Video_InlinePacket;
		packetHeaderFull.packetSize = static_cast<lwmUInt32>(sizeof(stripMinHeader) + stripDataSize);
		lwmWritePlanToFile(packetHeader, outFile);
		lwmWritePlanToFile(packetHeaderFull, outFile);

		outFile->WriteBytes(minCVidHeader, sizeof(stripMinHeader));

		// Copy strip data
		{
			lwmUInt8 copyBuffer[1000];
			while(stripDataSize > 0)
			{
				lwmLargeUInt copySize = sizeof(copyBuffer);
				if(copySize > stripDataSize)
					copySize = stripDataSize;
				inFile->ReadBytes(copyBuffer, copySize);
				outFile->WriteBytes(copyBuffer, copySize);
				stripDataSize -= copySize;
			}
		}
	}
}

void ConvertCVID(lwmOSFile *inFile, lwmOSFile *outFile)
{
	CAVIAtom *rootAtom = ParseAtom_r(inFile);

	CAVIDataList *hdrlList = static_cast<CAVIDataList*>(rootAtom)->FindListChild(SFourCC('h', 'd', 'r', 'l'));

	CAVIDataChunk *avihAtom = hdrlList->FindDataChild(SFourCC('a', 'v', 'i', 'h'));
	inFile->Seek(avihAtom->FileOffset());
	lwmUInt32 usecPerFrame = readUInt32(inFile);
	lwmUInt32 maxBytesPerSec = readUInt32(inFile);
	lwmUInt32 paddingGranularity = readUInt32(inFile);
	lwmUInt32 aviFlags = readUInt32(inFile);
	lwmUInt32 totalFrames = readUInt32(inFile);
	lwmUInt32 initialFrames = readUInt32(inFile);
	lwmUInt32 nStreams = readUInt32(inFile);
	lwmUInt32 suggestedBufferSize = readUInt32(inFile);
	lwmUInt32 vidWidth = readUInt32(inFile);
	lwmUInt32 vidHeight = readUInt32(inFile);

	std::vector<CAVIDataList*> streamListAtoms = hdrlList->FindListChildren(SFourCC('s', 't', 'r', 'l'));
	std::vector<CAVIStreamInfo> streams;
	streams.resize(nStreams);

	lwmUInt32 streamNo = 0;
	for(std::vector<CAVIDataList*>::const_iterator sIt = streamListAtoms.begin(), sItEnd = streamListAtoms.end(); sIt != sItEnd; ++sIt, ++streamNo)
	{
		CAVIDataList *streamListAtom = *sIt;
		CAVIDataChunk *strhChunk = streamListAtom->FindDataChild(SFourCC('s', 't', 'r', 'h'));
		inFile->Seek(strhChunk->FileOffset());

		CAVIStreamInfo &stream = streams[streamNo];
		stream.m_fccType = SFourCC(readUInt32(inFile));
		stream.m_fccHandler = SFourCC(readUInt32(inFile));
		stream.m_flags = readUInt32(inFile);
		stream.m_priority = readUInt16(inFile);
		stream.m_language = readUInt16(inFile);
		stream.m_initialFrames = readUInt32(inFile);
		stream.m_scale = readUInt32(inFile);
		stream.m_rate = readUInt32(inFile);
		stream.m_start = readUInt32(inFile);
		stream.m_length = readUInt32(inFile);
		stream.m_suggestedBufferSize = readUInt32(inFile);
		stream.m_quality = readUInt32(inFile);
		stream.m_sampleSize = readUInt32(inFile);
		stream.m_rectLeft = readUInt16(inFile);
		stream.m_rectTop = readUInt16(inFile);
		stream.m_rectRight = readUInt16(inFile);
		stream.m_rectBottom = readUInt16(inFile);
	}

	CAVIDataList *moviList = static_cast<CAVIDataList*>(rootAtom)->FindListChild(SFourCC('m', 'o', 'v', 'i'));
	for(std::vector<CAVIAtom*>::const_iterator it = moviList->Children().begin(), itEnd = moviList->Children().end(); it != itEnd; ++it)
	{
		CAVIAtom *moviSubAtom = *it;
		if(moviSubAtom->AtomType() == ATOM_TYPE_LIST)
		{
			CAVIDataList *moviSubList = static_cast<CAVIDataList*>(moviSubAtom);
			if(moviSubList->ListFourCC() == SFourCC('r', 'e', 'c', ' '))
			{
				for(std::vector<CAVIAtom*>::const_iterator msIt = moviSubList->Children().begin(), msItEnd = moviSubList->Children().begin(); msIt != msItEnd; ++msIt)
					AddChunkToStream(streams, *it);
			}
		}
		else
			AddChunkToStream(streams, *it);
	}

	const CAVIStreamInfo *videoStreamInfo = NULL;

	// Find video stream
	for(lwmUInt32 streamNo=0;streamNo<nStreams;streamNo++)
	{
		const CAVIStreamInfo *stream = &streams[streamNo];
		if(stream->m_fccType == SFourCC('v', 'i', 'd', 's'))
			videoStreamInfo = stream;
	}

	// Convert all frames
	{
		// Write movie header
		lwmMovieHeader movieHeader;
		movieHeader.audioStreamType = lwmAST_None;
		movieHeader.videoStreamType = lwmVST_CVID_Variant;
		movieHeader.numTOC = 0;
		lwmWritePlanToFile(movieHeader, outFile);
	}

	// Write VSI
	{
		lwmVideoStreamInfo vsi;
		vsi.periodsPerSecondNum = static_cast<lwmUInt16>(videoStreamInfo->m_rate);
		vsi.periodsPerSecondDenom = static_cast<lwmUInt16>(videoStreamInfo->m_scale);
		vsi.videoHeight = static_cast<lwmUInt16>(vidHeight);
		vsi.videoWidth = static_cast<lwmUInt16>(vidWidth);
		lwmWritePlanToFile(vsi, outFile);
	}

	// Don't need stream parameters for CVID
	// Write stream parameters (for CVID, this is empty)
	for(std::vector<CAVIDataChunk*>::const_iterator it=videoStreamInfo->m_chunks.begin(), itEnd=videoStreamInfo->m_chunks.end(); it != itEnd; ++it)
	{
		CAVIDataChunk *chunk = *it;
		ConvertCVIDChunk(chunk, vidWidth, vidHeight, inFile, outFile);
	}

	return;
}

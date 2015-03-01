#include <stdlib.h>

#include "lwmux_osfile.hpp"
#include "lwmux_util.hpp"
#include "lwmux_escape.hpp"
#include "../lwmovie/lwmovie_package.hpp"

void lwmux::util::WriteSyncedAudioPacket(lwmOSFile *outFile, const lwmUInt8 *bytes, lwmUInt32 sz, lwmUInt32 syncTime)
{
	lwmPacketHeader pktHeader;
	lwmPacketHeaderFull pktHeaderFull;
	pktHeaderFull.packetSize = sz;
	pktHeader.packetTypeAndFlags = lwmEPT_Audio_Frame;

	lwmUInt32 packetSizeEscaped = lwmComputeEscapes(bytes, pktHeaderFull.packetSize);
	lwmUInt8 *escapedPacketBytes = NULL;
	if(packetSizeEscaped != pktHeaderFull.packetSize)
	{
		escapedPacketBytes = new lwmUInt8[packetSizeEscaped];
		lwmGenerateEscapedBytes(escapedPacketBytes, bytes, pktHeaderFull.packetSize);
		pktHeader.packetTypeAndFlags |= lwmPacketHeader::EFlag_Escaped;
		pktHeaderFull.packetSize = packetSizeEscaped;
	}

	lwmWritePlanToFile(pktHeader, outFile);
	lwmWritePlanToFile(pktHeaderFull, outFile);

	if(escapedPacketBytes)
	{
		outFile->WriteBytes(escapedPacketBytes, pktHeaderFull.packetSize);
		delete[] escapedPacketBytes;
	}
	else
		outFile->WriteBytes(bytes, pktHeaderFull.packetSize);

	// Write synchronization
	{
		lwmAudioSynchronizationPoint syncPoint;
		syncPoint.audioPeriod = syncTime;

		lwmPacketHeader syncPacket;
		lwmPacketHeaderFull syncPacketFull;
		syncPacketFull.packetSize = lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE;
		syncPacket.packetTypeAndFlags = lwmEPT_Audio_Synchronization;

		lwmWritePlanToFile(syncPacket, outFile);
		lwmWritePlanToFile(syncPacketFull, outFile);
		lwmWritePlanToFile(syncPoint, outFile);
	}
}

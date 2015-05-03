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
#include <stdlib.h>

#include "lwmux_osfile.hpp"
#include "lwmux_util.hpp"
#include "lwmux_escape.hpp"
#include "lwmux_planio.hpp"
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/lwmovie_coretypes.h"
#include "lwmux_osfile.hpp"

void ConvertM1V(lwmOSFile *inFile, lwmOSFile *outFile);
void ConvertMP2(lwmOSFile *inFile, lwmOSFile *outFile);
void ConvertWAV_CELT(lwmOSFile *inFile, lwmOSFile *outFile, lwmUInt32 bitsPerSecond);
void ConvertWAV_ADPCM(lwmOSFile *inFile, lwmOSFile *outFile);
void ConvertCVID(lwmOSFile *inFile, lwmOSFile *outFile);
void Mux(lwmLargeUInt audioReadAhead, lwmOSFile *audioFile, lwmOSFile *videoFile, lwmOSFile *outFile);

int main(int argc, const char **argv)
{
	if(!strcmp(argv[1], "importm1v"))
	{
		lwmOSFile *mpegFile = lwmOSFile::Open(argv[2], lwmOSFile::FM_Read);
		lwmOSFile *outFile = lwmOSFile::Open(argv[3], lwmOSFile::FM_Create);
		ConvertM1V(mpegFile, outFile);
	}
	else if(!strcmp(argv[1], "importmp2"))
	{
		lwmOSFile *mp2file = lwmOSFile::Open(argv[2], lwmOSFile::FM_Read);
		lwmOSFile *outFile = lwmOSFile::Open(argv[3], lwmOSFile::FM_Create);
		ConvertMP2(mp2file, outFile);
	}
	else if(!strcmp(argv[1], "finalize"))
	{
		lwmLargeUInt audioReadAhead = static_cast<lwmLargeUInt>(atoi(argv[2]));
		lwmOSFile *audioFile = lwmOSFile::Open(argv[3], lwmOSFile::FM_Read);
		lwmOSFile *videoFile = lwmOSFile::Open(argv[4], lwmOSFile::FM_Read);
		lwmOSFile *outFile = lwmOSFile::Open(argv[5], lwmOSFile::FM_Create);
		
		Mux(audioReadAhead, audioFile, videoFile, outFile);
	}
	else if(!strcmp(argv[1], "importwav_celt"))
	{
		lwmOSFile *wavFile = lwmOSFile::Open(argv[3], lwmOSFile::FM_Read);
		lwmOSFile *outFile = lwmOSFile::Open(argv[4], lwmOSFile::FM_Create);
		ConvertWAV_CELT(wavFile, outFile, atoi(argv[2]));
	}
	else if(!strcmp(argv[1], "importwav_adpcm"))
	{
		lwmOSFile *wavFile = lwmOSFile::Open(argv[2], lwmOSFile::FM_Read);
		lwmOSFile *outFile = lwmOSFile::Open(argv[3], lwmOSFile::FM_Create);
		ConvertWAV_ADPCM(wavFile, outFile);
	}

	return 0;
}

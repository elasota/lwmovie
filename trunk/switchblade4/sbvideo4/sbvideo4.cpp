#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vfw.h>
#include "../sb4.h"
#include "../sb4image.h"
#include "../sb4videoenc.h"

#include <string>

FILE *roqFile;

void writecallback(void *h, const void *bytes, size_t numBytes)
{
	fwrite(bytes, 1, numBytes, (FILE *)h);
}

int main(int argc, char **argv)
{
	std::string baseFile;
	std::string baseName;
	std::string cbcName;
	std::string roqName;
	sb4aviinfo_t aviInfo;
	sb4vidstream_t *vs;
	std::vector<unsigned char> scratch;
	char status[1000];
	double npp;

	static const unsigned char roqHeader[] =
		{0x84, 0x10, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00};

	int numFrames;
	SB4Image *img;
	SB4Image pbar;

	sb4videoenc_t enc;

	SB4AVIInit();

	if(argc < 3)
		return 0;

	baseFile = argv[1];

	if(baseFile.substr(baseFile.size() - 4, 4) == ".avi")
		baseName = baseFile.substr(0, baseFile.size() - 4);
	else
		baseName = baseFile;

	roqName = baseName + "_video.roq";

	printf("Opening output %s\n", roqName.c_str());
	roqFile = fopen(roqName.c_str(), "wb");
	if(!roqFile)
	{
		MessageBox(NULL, "Couldn't open output RoQ file", NULL, 0);
		return 1;
	}

	SB4GetAVIInfo(baseFile.c_str(), &aviInfo);

	// Figure out how many rows to use for feedback

	SB4GetAVIVideoStream(baseFile.c_str(), &vs);


	if(aviInfo.width % 16 || aviInfo.height % 16)
	{
		MessageBox(NULL, "Input video size must be divisible by 16.", NULL, 0);
		exit(1);
	}

	// Write RoQ header
	fwrite( roqHeader, 8, 1, roqFile );

	pbar.Init(aviInfo.width, 8);

	roq_encode_init(&enc, aviInfo.width, aviInfo.height, 999999, atoi(argv[2]) * 2, atoi(argv[2]));

	numFrames = aviInfo.numFrames;
	while(numFrames--)
	{
		img = SB4GetAVIVideoFrame(vs);

		roq_encode_frame(&enc, img, roqFile, writecallback);

		npp = (double)(aviInfo.width * aviInfo.height);
		printf(status, "Frame %i / %i  Sz:%i  Slip: %i  Dpp:%4.4f", aviInfo.numFrames - numFrames, aviInfo.numFrames, enc.final_size, enc.slip, (double)enc.distTEMP / npp);

		scratch.clear();
		delete img;
	}
	SB4CloseAVIVideoStream(vs);

	fclose(roqFile);

	return 0;
}

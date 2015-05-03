#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <vector>


int main(int argc, const char **argv)
{
#ifdef _MSC_VER
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	FILE *inFile = stdin;
	FILE *outFile = stdout;

	std::vector<std::string> headerTags;

	{
		std::string currentTag;
		while(int c = fgetc(inFile))
		{
			if(c == ' ' || c == '\n')
			{
				headerTags.push_back(currentTag);
				if(c == 0x0a)
					break;
				currentTag = "";
			}
			else
				currentTag += static_cast<char>(c);
		}
	}

	bool foundCSTag = false;
	
	unsigned int width = 0;
	unsigned int height = 0;

	int ranges[2][2];

	// Process header tags
	for(std::vector<std::string>::iterator it = headerTags.begin(), itEnd = headerTags.end(); it != itEnd; ++it)
	{
		std::string &tag = *it;
		if(tag[0] == 'C')
		{
			if(tag == "C420p9")
			{
				foundCSTag = true;
				ranges[0][0] = 0x20;
				ranges[0][1] = 0x1d6;
				ranges[1][0] = 0x20;
				ranges[1][1] = 0x1e0;
			}
			else if(tag == "C420p10")
			{
				foundCSTag = true;
				ranges[0][0] = 0x40;
				ranges[0][1] = 0x3ac;
				ranges[1][0] = 0x40;
				ranges[1][1] = 0xec0;
			}
			else if(tag == "C420p12")
			{
				foundCSTag = true;
				ranges[0][0] = 0x100;
				ranges[0][1] = 0xeb0;
				ranges[1][0] = 0x100;
				ranges[1][1] = 0xf00;
			}
			else
			{
				fprintf(stderr, "ERROR: Color space %s not supported by lwrerange.  Use yuv420p9, yuv420p10, or yuv420p12.", tag.c_str());
				return -1;
			}
			tag = "C420jpeg";
		}
		else if(tag[0] == 'I')
		{
			if(tag != "Ip")
			{
				fprintf(stderr, "ERROR: Only progressive scan is supported.");
				return -1;
			}
		}
		else if(tag[0] == 'X')
		{
			
			if(tag != "XYSCSS=420P12" &&
				tag != "XYSCSS=420P10" &&
				tag != "XYSCSS=420P9")
			{
				fprintf(stderr, "ERROR: Color space %s not supported by lwrerange.  Use yuv420p9, yuv420p10, or yuv420p12.", tag.c_str());
				return -1;
			}
			tag = "XYSCSS=420JPEG";
		}
		else if(tag[0] == 'W')
			width = static_cast<unsigned int>(atoi(tag.c_str() + 1));
		else if(tag[0] == 'H')
			height = static_cast<unsigned int>(atoi(tag.c_str() + 1));
	}

	if(foundCSTag == false || width == 0 || height == 0)
	{
		fprintf(stderr, "lwrerange: Missing C, W, or H tag.\n");
		return -1;
	}

	// Flush header tags
	for(std::vector<std::string>::const_iterator it = headerTags.begin(), itEnd = headerTags.end(); it != itEnd; ++it)
	{
		const std::string &tag = *it;
		if(it != headerTags.begin())
			fputc(' ', outFile);
		for(std::string::const_iterator tagIt = tag.begin(), tagItEnd = tag.end(); tagIt != tagItEnd; ++tagIt)
			fputc(*tagIt, outFile);
	}
	fputc('\n', outFile);

	unsigned int nLumaSamples = width * height;
	unsigned int nChromaSamples = ((width + 1) / 2) * ((height + 1) / 2) * 2;

	short *inputSamples = new short[nLumaSamples];
	unsigned char *outputSamples = reinterpret_cast<unsigned char*>(inputSamples);

	// Read frames
	char frameTag[6];
	while(int frameTagSize = fread(frameTag, 1, 6, inFile))
	{
		if(frameTagSize != 6 || memcmp(frameTag, "FRAME\n", 6))
		{
			fprintf(stderr, "lwrerange: Unexpected frame boundary\n");
			return -1;
		}
		fwrite(frameTag, 6, 1, outFile);

		unsigned int sgSizes[2];
		sgSizes[0] = width * height;
		sgSizes[1] = ((width + 1) / 2) * ((height + 1) / 2) * 2;
		for(int sg=0;sg<2;sg++)
		{
			unsigned int sgSize = sgSizes[sg];
			int sMin = ranges[sg][0];
			int sRange = ranges[sg][1] - ranges[sg][0];
			fread(inputSamples, 2, sgSize, inFile);
			for(unsigned int i=0;i<sgSize;i++)
			{
				int inSample = inputSamples[i];
				int outSample = (inSample - sMin) * 255 / sRange;
				if(outSample < 0)
					outputSamples[i] = 0;
				else if(outSample > 255)
					outputSamples[i] = 255;
				else
					outputSamples[i] = static_cast<unsigned char>(outSample);
			}
			if(fwrite(outputSamples, 1, sgSize, outFile) != sgSize)
			{
				fprintf(stderr, "lwrerange: Broken pipe\n");
				return -1;
			}
		}
	}
	delete[] inputSamples;
	return 0;
}

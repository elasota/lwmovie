#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwmovie_videotypes.hpp"
#include "lwmovie_demux.hpp"
#include "lwmovie_profile.hpp"

using namespace lwmovie;

lwmVidStream *vidStream;

static void *MyAlloc(void *opaque, lwmLargeUInt sz)
{
	return _aligned_malloc(sz, 16);
}

static void MyFree(void *opaque, void *ptr)
{
	_aligned_free(ptr);
}

static lwmLargeUInt MyRead(void *f, void *buf, lwmLargeUInt nBytes)
{
	return fread(buf, 1, nBytes, static_cast<FILE *>(f));
}

int main(int argc, const char **argv)
{
	lwmSAllocator alloc;
	alloc.opaque = NULL;
	alloc.allocFunc = MyAlloc;
	alloc.freeFunc = MyFree;

	lwmMovieState *movieState = lwmInitialize(&alloc);

	FILE *f = fopen("D:\\vids\\output_mux.lwmv", "rb");

	lwmUInt8 buffer[32768];
	size_t bufferAvailable;
	lwmIVideoReconstructor *videoRecon = NULL;
	LARGE_INTEGER frameTime;
	lwmUInt64 frameTimeTotal = 0;
	
	lwmUInt32 width, height, ppsNum, ppsDenom;

	while(true)
	{
		bufferAvailable = fread(buffer, 1, sizeof(buffer), f);
		const lwmUInt8 *feedBuffer = buffer;

		if(!bufferAvailable)
			break;

		lwmUInt32 result = 0xffffffff;
		while(bufferAvailable || result != lwmDIGEST_Nothing)
		{
			LARGE_INTEGER perfStart, perfEnd;
			QueryPerformanceCounter(&perfStart);
			lwmUInt32 digested;
			lwmFeedData(movieState, feedBuffer, bufferAvailable, &result, &digested);
			QueryPerformanceCounter(&perfEnd);
			bufferAvailable -= digested;
			feedBuffer += digested;

			frameTimeTotal += static_cast<lwmUInt64>(perfEnd.QuadPart - perfStart.QuadPart);

			switch(result)
			{
			case lwmDIGEST_Initialize:
				{
					lwmUInt32 reconType;
					lwmGetVideoParameters(movieState, &width, &height, &ppsNum, &ppsDenom, &reconType);
					lwmCreateSoftwareVideoReconstructor(&alloc, width, height, reconType, NULL, &videoRecon);
					lwmSetVideoReconstructor(movieState, videoRecon);
				}
				break;
			case lwmDIGEST_VideoSync:
				{
					char outPath[1000];
					static int numFrames = 0;

					const lwmUInt8 *yBuffer;
					const lwmUInt8 *uBuffer;
					const lwmUInt8 *vBuffer;
					lwmUInt32 yStride, uStride, vStride;
					
					LARGE_INTEGER perfFreq;
					QueryPerformanceFrequency(&perfFreq);

					double pfDouble = static_cast<double>(perfFreq.QuadPart);
					double ptDouble = static_cast<double>(frameTimeTotal);

					printf("Frame time: %f ms\n", ptDouble * 1000.0 / pfDouble);
					frameTimeTotal = 0;

					lwmReconGetChannel(videoRecon, 0, &yBuffer, &yStride);
					lwmReconGetChannel(videoRecon, 1, &uBuffer, &uStride);
					lwmReconGetChannel(videoRecon, 2, &vBuffer, &vStride);

					if(numFrames % 100 == 0)
						printf("%i...\n", numFrames);
					if(numFrames < 300)
					{
						sprintf(outPath, "D:\\vids\\frames\\frame%4i.raw", numFrames++);
						FILE *frameF = fopen(outPath, "wb");
						for(lwmUInt32 row=0;row<height;row++)
						{
							for(lwmUInt32 col=0;col<width;col++)
							{
								lwmSInt32 y = yBuffer[row*yStride+col];
								lwmSInt32 u = uBuffer[(row/2)*uStride+(col/2)];
								lwmSInt32 v = vBuffer[(row/2)*vStride+(col/2)];

								lwmSInt32 yBase = y * 298;
								lwmSInt32 r = (yBase + 409*v - 57120) / 256;
								lwmSInt32 g = (yBase - 100*u - 208*v + 34656) / 256;
								lwmSInt32 b = (yBase + 516*u - 70816) / 256;

								if(r < 0) r = 0; else if(r > 255) r = 255;
								if(g < 0) g = 0; else if(g > 255) g = 255;
								if(b < 0) b = 0; else if(b > 255) b = 255;

								lwmUInt8 outPixel[3];
								//outPixel[0] = static_cast<lwmUInt8>(r);
								//outPixel[1] = static_cast<lwmUInt8>(g);
								//outPixel[2] = static_cast<lwmUInt8>(b);
								outPixel[0] = static_cast<lwmUInt8>(y);
								outPixel[1] = static_cast<lwmUInt8>(y);
								outPixel[2] = static_cast<lwmUInt8>(y);
								fwrite(outPixel, 3, 1, frameF);
							}
						}
						fclose(frameF);
					}
				}
				break;
			default:
				{
					int bp = 0;
				}
				break;
			};
		}
	}
	printf("Profile results:\n");
	printf("Deslice: %f\n", g_ptDeslice.GetTotalTime() * 1000.0);
	printf("    ParseBlock: %f\n", g_ptParseBlock.GetTotalTime() * 1000.0);
	printf("        ParseCoeffs: %f\n", g_ptParseCoeffs.GetTotalTime() * 1000.0);
	printf("            ParseCoeffsTest: %f\n", g_ptParseCoeffsTest.GetTotalTime() * 1000.0);
	printf("            ParseCoeffsIntra: %f\n", g_ptParseCoeffsIntra.GetTotalTime() * 1000.0);
	printf("            ParseCoeffsInter: %f\n", g_ptParseCoeffsInter.GetTotalTime() * 1000.0);
	printf("            ParseCoeffsCommit: %f\n", g_ptParseCoeffsCommit.GetTotalTime() * 1000.0);
	printf("                IDCT Sparse: %f\n", g_ptIDCTSparse.GetTotalTime() * 1000.0);
	printf("                IDCT Full: %f\n", g_ptIDCTFull.GetTotalTime() * 1000.0);
	printf("        ReconRow: %f\n", g_ptReconRow.GetTotalTime() * 1000.0);
	printf("            Motion: %f\n", g_ptMotion.GetTotalTime() * 1000.0);

	printf("Done\n");
	return 0;
}

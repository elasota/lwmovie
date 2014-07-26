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

VOID NTAPI MyVideoDigestWorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	lwmMovieState *movieState = static_cast<lwmMovieState *>(context);
	lwmVideoDigestParticipate(movieState);
}

VOID NTAPI MyVideoReconWorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	lwmIVideoReconstructor *recon = static_cast<lwmIVideoReconstructor *>(context);
	lwmVideoReconParticipate(recon);
}

void MyJoin(void *opaque)
{
	WaitForThreadpoolWorkCallbacks(static_cast<PTP_WORK>(opaque), FALSE);
}

void MyNotifyAvailable(void *opaque)
{
	SubmitThreadpoolWork(static_cast<PTP_WORK>(opaque));
}


int main(int argc, const char **argv)
{
	lwmSAllocator alloc;
	alloc.opaque = NULL;
	alloc.allocFunc = MyAlloc;
	alloc.freeFunc = MyFree;

	lwmMovieState *movieState = lwmInitialize(&alloc);

	PTP_WORK digestWork = CreateThreadpoolWork(MyVideoDigestWorkCallback, movieState, NULL);

	lwmSWorkNotifier digestNotifier;
	digestNotifier.join = MyJoin;
	digestNotifier.notifyAvailable = MyNotifyAvailable;
	digestNotifier.opaque = digestWork;

	lwmSetVideoDigestWorkNotifier(movieState, &digestNotifier);

	FILE *f = fopen("D:\\vids\\output_mux.lwmv", "rb");

	lwmUInt8 buffer[32768];
	size_t bufferAvailable;
	lwmIVideoReconstructor *videoRecon = NULL;
	LARGE_INTEGER frameTime;
	lwmUInt64 frameTimeTotal = 0;
	
	lwmUInt32 width, height, ppsNum, ppsDenom;
	lwmCProfileTagSet tagSet;

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
					lwmCreateSoftwareVideoReconstructor(movieState, &alloc, reconType, &videoRecon);
					lwmSetVideoReconstructor(movieState, videoRecon);
					
					PTP_WORK videoReconWork = CreateThreadpoolWork(MyVideoReconWorkCallback, videoRecon, NULL);

					lwmSWorkNotifier videoReconNotifier;
					videoReconNotifier.join = MyJoin;
					videoReconNotifier.notifyAvailable = MyNotifyAvailable;
					videoReconNotifier.opaque = videoReconWork;

					lwmSetVideoReconWorkNotifier(videoRecon, &videoReconNotifier);
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

					lwmFlushProfileTags(movieState, &tagSet);

					numFrames++;

					if(numFrames % 100 == 0)
						printf("%i...\n", numFrames);
					if(numFrames < 0)
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
done:


	printf("Profile results:\n");
	printf("Deslice: %f\n", tagSet.GetTag(lwmEPROFILETAG_Deslice)->GetTotalTime() * 1000.0);
	printf("    ParseBlock: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseBlock)->GetTotalTime() * 1000.0);
	printf("        ParseCoeffs: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseCoeffs)->GetTotalTime() * 1000.0);
	printf("            ParseCoeffsTest: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseCoeffsTest)->GetTotalTime() * 1000.0);
	printf("            ParseCoeffsIntra: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseCoeffsIntra)->GetTotalTime() * 1000.0);
	printf("            ParseCoeffsInter: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseCoeffsInter)->GetTotalTime() * 1000.0);
	printf("            ParseCoeffsCommit: %f\n", tagSet.GetTag(lwmEPROFILETAG_ParseCoeffsCommit)->GetTotalTime() * 1000.0);
	printf("                IDCT Sparse: %f\n", tagSet.GetTag(lwmEPROFILETAG_IDCTSparse)->GetTotalTime() * 1000.0);
	printf("                IDCT Full: %f\n", tagSet.GetTag(lwmEPROFILETAG_IDCTFull)->GetTotalTime() * 1000.0);
	printf("        ReconRow: %f\n", tagSet.GetTag(lwmEPROFILETAG_ReconRow)->GetTotalTime() * 1000.0);
	printf("            Motion: %f\n", tagSet.GetTag(lwmEPROFILETAG_Motion)->GetTotalTime() * 1000.0);

	printf("Done\n");
	return 0;
}

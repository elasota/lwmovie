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
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwmovie_videotypes.hpp"
#include "lwmovie_demux.hpp"
#include "lwmovie_profile.hpp"

using namespace lwmovie;

lwmVidStream *vidStream;

static void *MyAlloc(lwmSAllocator *alloc, lwmLargeUInt sz)
{
	return _aligned_malloc(sz, 16);
}

static void MyFree(lwmSAllocator *alloc, void *ptr)
{
	_aligned_free(ptr);
}

VOID NTAPI MyVideoDigestWorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	lwmMovieState *movieState = static_cast<lwmMovieState *>(context);
	lwmMovieState_VideoDigestParticipate(movieState);
}

VOID NTAPI MyVideoReconWorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	lwmIVideoReconstructor *recon = static_cast<lwmIVideoReconstructor *>(context);
	lwmVideoRecon_Participate(recon);
}

class CWinThreadPoolWorkNotifier : public lwmSWorkNotifier
{
public:
	CWinThreadPoolWorkNotifier()
	{
		this->joinFunc = StaticJoin;
		this->notifyAvailableFunc = StaticNotifyAvailable;
	}

	void SetWork(PTP_WORK work)
	{
		m_work = work;
	}

	static void StaticJoin(lwmSWorkNotifier *workNotifier)
	{
		WaitForThreadpoolWorkCallbacks(static_cast<CWinThreadPoolWorkNotifier*>(workNotifier)->m_work, FALSE);
	}

	static void StaticNotifyAvailable(lwmSWorkNotifier *workNotifier)
	{
		SubmitThreadpoolWork(static_cast<CWinThreadPoolWorkNotifier*>(workNotifier)->m_work);
	}

private:
	PTP_WORK m_work;
};


int main(int argc, const char **argv)
{
	if(argc < 2)
		return -1;

	lwmSAllocator alloc;
	alloc.allocFunc = MyAlloc;
	alloc.freeFunc = MyFree;

	lwmMovieState *movieState = lwmCreateMovieState(&alloc, lwmUSERFLAG_ThreadedDeslicer);

	PTP_WORK digestWork = CreateThreadpoolWork(MyVideoDigestWorkCallback, movieState, NULL);

	CWinThreadPoolWorkNotifier digestNotifier;
	digestNotifier.SetWork(digestWork);

	lwmMovieState_SetVideoDigestWorkNotifier(movieState, &digestNotifier);

	FILE *f = fopen(argv[1], "rb");

	lwmUInt8 buffer[32768];
	size_t bufferAvailable;
	lwmIVideoReconstructor *videoRecon = NULL;
	
	lwmUInt32 width, height;
#ifdef LWMOVIE_PROFILE
	lwmCProfileTagSet tagSet;
#endif
	lwmSVideoFrameProvider *frameProvider;

	CWinThreadPoolWorkNotifier videoReconNotifier;

	while(true)
	{
		bufferAvailable = fread(buffer, 1, sizeof(buffer), f);
		const lwmUInt8 *feedBuffer = buffer;

		if(!bufferAvailable)
			break;

		lwmUInt32 result = 0xffffffff;
		while(bufferAvailable || result != lwmDIGEST_Nothing)
		{
			lwmUInt32 digested;
			lwmMovieState_FeedData(movieState, feedBuffer, bufferAvailable, &result, &digested);
			bufferAvailable -= digested;
			feedBuffer += digested;

			switch(result)
			{
			case lwmDIGEST_Initialize:
				{
					lwmUInt32 reconType;
					lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, lwmSTREAMPARAM_U32_Width, &width);
					lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, lwmSTREAMPARAM_U32_Height, &height);
					lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, lwmSTREAMPARAM_U32_ReconType, &reconType);
					frameProvider = lwmCreateSystemMemoryFrameProvider(&alloc, movieState);
					videoRecon = lwmCreateSoftwareVideoReconstructor(movieState, &alloc, reconType, frameProvider);
					lwmMovieState_SetVideoReconstructor(movieState, videoRecon);
					
					PTP_WORK videoReconWork = CreateThreadpoolWork(MyVideoReconWorkCallback, videoRecon, NULL);

					videoReconNotifier.SetWork(videoReconWork);

					lwmVideoRecon_SetWorkNotifier(videoRecon, &videoReconNotifier);
				}
				break;
			case lwmDIGEST_VideoSync:
				{
					const lwmUInt8 *yBuffer;
					const lwmUInt8 *uBuffer;
					const lwmUInt8 *vBuffer;
					lwmUInt32 yStride, uStride, vStride;

					lwmUInt32 frameIndex = lwmVideoRecon_GetWorkFrameIndex(videoRecon);
					frameProvider->lockWorkFrameFunc(frameProvider, frameIndex, lwmVIDEOLOCK_Read);

					yBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 0));
					uBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 1));
					vBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 2));

					yStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 0);
					uStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 1);
					vStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 2);

#ifdef LWMOVIE_PROFILE
					lwmFlushProfileTags(movieState, &tagSet);
#endif
					// TODO: Output frame

					frameProvider->unlockWorkFrameFunc(frameProvider, frameIndex);
				}
				break;
			default:
				break;
			};
		}
	}
done:

	if(movieState)
		lwmMovieState_Destroy(movieState);
	if(videoRecon)
		lwmIVideoReconstructor_Destroy(videoRecon);
	if(frameProvider)
		lwmSVideoFrameProvider_Destroy(frameProvider);

#ifdef LWMOVIE_PROFILE
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
#endif

	printf("Done\n");
	return 0;
}

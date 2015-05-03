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
#include "lwmovie_recon_roqsw.hpp"
#include "lwmovie_external_types.h"
#include "lwmovie.h"

bool lwmovie::roq::CSoftwareReconstructor::Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState)
{
	lwmUInt32 width, height, numWFrames, numRWFrames;
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &width);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &height);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumReadWriteWorkFrames, &numRWFrames);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames, &numWFrames);

	m_frontFrame = 0;
	m_alloc = alloc;
	m_frameProvider = frameProvider;
	m_width = (width + 15) / 16 * 16;
	m_height = (height + 15) / 16 * 16;

	if (numWFrames != 0 || numRWFrames != 2)
		return false;

	m_frameProvider->createWorkFramesFunc(m_frameProvider, 2, 0, m_width, m_height, lwmFRAMEFORMAT_8Bit_3Channel_Interleaved);

	return true;
}

lwmovie::roq::CSoftwareReconstructor::~CSoftwareReconstructor()
{
	this->WaitForFinish();
}

void lwmovie::roq::CSoftwareReconstructor::Participate()
{
}

void lwmovie::roq::CSoftwareReconstructor::WaitForFinish()
{
}

void lwmovie::roq::CSoftwareReconstructor::SetWorkNotifier(lwmSWorkNotifier *workNotifier)
{

}

void lwmovie::roq::CSoftwareReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{

}

lwmUInt32 lwmovie::roq::CSoftwareReconstructor::GetWorkFrameIndex() const
{
	return m_frontFrame;
}

void lwmovie::roq::CSoftwareReconstructor::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	this->~CSoftwareReconstructor();
	alloc->Free(this);
}

lwmSVideoFrameProvider *lwmovie::roq::CSoftwareReconstructor::GetFrameProvider() const
{
	return m_frameProvider;
}

lwmUInt32 lwmovie::roq::CSoftwareReconstructor::StartFrame()
{
	return 1 - m_frontFrame;
}

void lwmovie::roq::CSoftwareReconstructor::FinishFrame()
{
	m_frontFrame = 1 - m_frontFrame;
}

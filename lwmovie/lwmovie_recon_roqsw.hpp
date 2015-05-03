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
#ifndef __LWMOVIE_RECON_ROQSW_HPP__
#define __LWMOVIE_RECON_ROQSW_HPP__

#include "lwmovie_recon.hpp"

struct lwmSAllocator;
struct lwmMovieState;

namespace lwmovie
{
	namespace roq
	{
		class CSoftwareReconstructor : public lwmIVideoReconstructor
		{
		public:
			bool Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState);

			virtual ~CSoftwareReconstructor();
			virtual void Participate();
			virtual void WaitForFinish();
			virtual void SetWorkNotifier(lwmSWorkNotifier *workNotifier);
			virtual void FlushProfileTags(lwmCProfileTagSet *tagSet);
			virtual lwmUInt32 GetWorkFrameIndex() const;
			virtual void Destroy();
			virtual lwmSVideoFrameProvider *GetFrameProvider() const;

			lwmUInt32 StartFrame();
			void FinishFrame();

		private:
			lwmUInt32 m_frontFrame;

			lwmSAllocator *m_alloc;
			lwmSVideoFrameProvider *m_frameProvider;
			lwmUInt32 m_width;
			lwmUInt32 m_height;
		};
	}
}

#endif

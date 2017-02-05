#pragma once
#ifndef __LWMOVIE_THEORADEC_HPP__
#define __LWMOVIE_THEORADEC_HPP__

#include "../common/lwmovie_coretypes.h"
#include "lwmovie.h"
#include "lwmovie_recon.hpp"

struct lwmSAllocator;
struct lwmMovieState;
struct lwmSWorkNotifier;

#ifndef LWMOVIE_NO_THEORA

namespace lwmovie
{
	namespace theora
	{
		class CTheoraDecoderInternal;
		class CSoftwareReconstructor;

		class CTheoraDecoder
		{
		public:
			CTheoraDecoder(lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, lwmEFrameFormat frameFormat,
				lwmMovieState *movieState, lwmSWorkNotifier *workNotifier, bool useThreadedDeslicer);
			~CTheoraDecoder();

			bool DigestStreamParameters(const void *bytes, lwmUInt32 packetSize);
			bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, lwmUInt32 *outResult);
			void WaitForDigestFinish();
			bool EmitFrame();
			void SetDropAggressiveness(lwmEDropAggressiveness dropAggressiveness);

			void Participate();

			void SetReconstructor(lwmIVideoReconstructor *videoRecon);

		private:
			static void *Realloc(void *ctx, void *ptr, size_t sz);

			lwmSAllocator *m_alloc;
			lwmUInt32 m_width;
			lwmUInt32 m_height;
			lwmMovieState *m_movieState;
			lwmSWorkNotifier *m_workNotifier;
			lwmEDropAggressiveness m_dropAggressiveness;
			lwmEFrameFormat m_frameFormat;
			CSoftwareReconstructor *m_recon;
			bool m_useThreadedDeslicer;
			bool m_mustCheckHeader;
			bool m_frameWasDropped;

			CTheoraDecoderInternal *m_internal;
		};

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

#endif

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
#ifndef __LWMOVIE_RECON_D3D11_HPP__
#define __LWMOVIE_RECON_D3D11_HPP__

#include "lwmovie_recon_m1v.hpp"

struct lwmSAllocator;
struct lwmSVideoFrameProvider;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace lwmovie
{
	namespace d3d11
	{
		struct SDCTBufferBlock;

		LWMOVIE_ATTRIB_ALIGN(16) struct SReconConstants
		{
			lwmFloat32 fDimensionsL[2];
			lwmFloat32 fDimensionsC[2];
			lwmUInt32 mbDimensions[2];
		};

		struct SMBlockReconInfo
		{
			lwmSInt32 motionVectors[4];	// forwRight, forwDown, backRight, backDown
			lwmUInt32 flags;	// Low to high: zbf0, zbf1, zbf2, zbf3, zbf4, zbf5, zbf6, mfor, mback
			lwmUInt32 padding[3];
		};
	}
}

namespace lwmovie
{
	namespace d3d11
	{
		class CFrameProvider;

		class CM1VReconstructor : public lwmovie::m1v::IM1VReconstructor
		{
		public:
			CM1VReconstructor(lwmSAllocator *alloc, lwmMovieState *movieState, lwmSVideoFrameProvider *frameProvider, ID3D11Device *device, ID3D11DeviceContext *context);
			virtual ~CM1VReconstructor();
			virtual void Participate();
			virtual void WaitForFinish();
			virtual void SetWorkNotifier(lwmSWorkNotifier *workNotifier);
			virtual void FlushProfileTags(lwmCProfileTagSet *tagSet);
			virtual lwmUInt32 GetWorkFrameIndex() const;
			virtual void Destroy();
			virtual lwmSVideoFrameProvider *GetFrameProvider() const;

			virtual lwmovie::m1v::IM1VBlockCursor *CreateBlockCursor();

			virtual void MarkRowFinished(lwmSInt32 firstMBAddress);

			virtual void StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past, bool currentIsB);
			virtual void PresentFrame(lwmUInt32 outFrame);

			bool Initialize(lwmUInt32 width, lwmUInt32 height);

			lwmovie::d3d11::SDCTBufferBlock *GetPackedDCTBlock(lwmUInt32 blockAddress);
			lwmovie::d3d11::SMBlockReconInfo *GetMBlockInfo(lwmUInt32 mblockAddress);

		private:
			void CloseFrame();
			void OpenIDCTMap();
			void CloseIDCTMap();

			bool m_forwPred, m_backPred;
			bool m_workingOnFrame;
			lwmUInt32 m_current, m_future, m_past;

			lwmUInt32 m_mbWidth, m_mbHeight;

			lwmSAllocator *m_alloc;
			lwmMovieState *m_movieState;
			ID3D11Device *m_device;
			ID3D11DeviceContext *m_context;

			lwmUInt32 m_presentedFrame;
			lwmovie::d3d11::CFrameProvider *m_frameProvider;

			ID3D11Buffer *m_dctInputBuffer;
			ID3D11Buffer *m_dctOutputBuffer;
			ID3D11Buffer *m_mblockInfoBuffer;
			ID3D11Buffer *m_psConstantBuffer;
			ID3D11Buffer *m_reconVertexBuffer;

			ID3D11ComputeShader *m_idctCS;
			ID3D11VertexShader *m_reconVS;
			ID3D11PixelShader *m_reconLumaPS;
			ID3D11PixelShader *m_reconChromaPS;

			ID3D11InputLayout *m_reconIA;

			ID3D11SamplerState *m_planeLinearSampler;

			ID3D11RasterizerState *m_solidRasterState;

			ID3D11ShaderResourceView *m_dctInputSRV;
			ID3D11ShaderResourceView *m_mblockInfoSRV;
			ID3D11ShaderResourceView *m_dctOutputSRV;
			ID3D11UnorderedAccessView *m_dctOutputUAV;

			void *m_mappedDCTInputs;
			void *m_mappedMBlockInfos;
		};

		class CM1VBlockCursor : public lwmovie::m1v::IM1VBlockCursor
		{
		public:
			explicit CM1VBlockCursor(CM1VReconstructor *reconstructor);
			virtual ~CM1VBlockCursor();
			virtual void OpenMB(lwmSInt32 mbAddress);
			virtual void CloseMB();
			virtual void SetMBlockInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back,
				lwmSInt32 recon_right_for, lwmSInt32 recon_down_for,
				lwmSInt32 recon_right_back, lwmSInt32 recon_down_back);

			virtual void SetBlockInfo(lwmSInt32 blockIndex, bool zero_block_flag);
			virtual idct::DCTBLOCK *StartReconBlock(lwmSInt32 subBlockIndex);

			virtual void CommitZero();
			virtual void CommitSparse(lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff);
			virtual void CommitFull();

		private:
			lwmUInt32 m_mbAddress;
			lwmovie::d3d11::SMBlockReconInfo m_reconMBlockInfo;
			CM1VReconstructor *m_reconstructor;
			idct::DCTBLOCK *m_activeDCTBlock;
		};
	}
}

#endif

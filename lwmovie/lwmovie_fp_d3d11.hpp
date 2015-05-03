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
#ifndef __LWMOVIE_FP_D3D11_HPP__
#define __LWMOVIE_FP_D3D11_HPP__

#include "../lwmovie/lwmovie_cpp_shims.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace lwmovie
{
	namespace d3d11
	{
		class CFrameProvider : public lwmIVideoFrameProvider
		{
		public:
			explicit CFrameProvider(lwmSAllocator *alloc, ID3D11Device *device, ID3D11DeviceContext *context, bool isUsingHardwareReconstructor);
			~CFrameProvider();

			virtual int CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);
			virtual void LockWorkFrame(lwmUInt32 workFrameIndex, lwmEVideoLockType lockType);
			virtual void UnlockWorkFrame(lwmUInt32 workFrameIndex);
			virtual void *GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch);
			virtual lwmUInt32 GetWorkFramePlaneWidth(lwmUInt32 planeIndex) const;
			virtual lwmUInt32 GetWorkFramePlaneHeight(lwmUInt32 planeIndex) const;
			virtual void Destroy();

			ID3D11ShaderResourceView *GetWorkFramePlaneSRV(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);
			ID3D11RenderTargetView *GetWorkFramePlaneRTV(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);

		private:
			static const lwmLargeUInt MAX_CHANNELS = 3;

			struct ChannelTexture
			{
				ID3D11Texture2D *m_texture;
				ID3D11ShaderResourceView *m_srv;
				ID3D11RenderTargetView *m_rtv;
				lwmEVideoLockType m_activeLock;
				lwmLargeUInt m_lockPitch;	// D3D lock pitch
				void *m_lockData;
			};

			struct ChannelTextureBundle
			{
				ChannelTexture m_channelTextures[MAX_CHANNELS];
				lwmUInt8 *m_frameBytes;		// Indexed into m_frameBytes
			};

			ID3D11Texture2D *CreatePlaneTexture(lwmUInt32 width, lwmUInt32 height, bool isWriteOnly);

			ID3D11Device *m_device;
			ID3D11DeviceContext *m_context;
			lwmSAllocator *m_alloc;

			bool m_isUsingHardwareReconstructor;

			ChannelTextureBundle *m_channelTextureBundles;

			// Read-write frames only
			lwmUInt8 *m_frameBytes;
			lwmLargeUInt m_frameSize;
			lwmLargeUInt m_channelOffsets[MAX_CHANNELS];
			lwmLargeUInt m_channelStrides[MAX_CHANNELS];

			lwmLargeUInt m_numDynFrames;
			lwmLargeUInt m_numChannels;

			lwmUInt32 m_channelWidths[MAX_CHANNELS];
			lwmUInt32 m_channelHeights[MAX_CHANNELS];
		};
	}
}

#endif

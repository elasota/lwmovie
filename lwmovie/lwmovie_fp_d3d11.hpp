#ifndef __LWMOVIE_FP_D3D11_HPP__
#define __LWMOVIE_FP_D3D11_HPP__

#include "../lwmovie/lwmovie_cpp_shims.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace lwmovie
{
	class lwmCD3D11FrameProvider : public lwmIVideoFrameProvider
	{
	public:
		explicit lwmCD3D11FrameProvider(lwmSAllocator *alloc, ID3D11Device *device, ID3D11DeviceContext *context, bool isUsingHardwareReconstructor);
		~lwmCD3D11FrameProvider();

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

#endif

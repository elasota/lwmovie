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
#include <stdlib.h>
#include <new>
#include <d3d11.h>
#include "lwmovie.h"
#include "lwmovie_cpp_shims.hpp"
#include "lwmovie_simd_defs.hpp"
#include "lwmovie_fp_d3d11.hpp"

#include <stdio.h>


lwmovie::lwmCD3D11FrameProvider::lwmCD3D11FrameProvider(lwmSAllocator *alloc, ID3D11Device *device, ID3D11DeviceContext *context, bool isUsingHardwareReconstructor)
	: lwmIVideoFrameProvider()
	, m_device(device)
	, m_context(context)
	, m_alloc(alloc)
	, m_isUsingHardwareReconstructor(isUsingHardwareReconstructor)
	, m_frameBytes(NULL)
	, m_numDynFrames(0)
{
}

lwmovie::lwmCD3D11FrameProvider::~lwmCD3D11FrameProvider()
{
	if (m_channelTextureBundles != NULL)
	{
		for (lwmLargeUInt i = 0; i < m_numDynFrames; i++)
		{
			ChannelTextureBundle *ctb = m_channelTextureBundles + i;
			for (lwmLargeUInt j = 0; j < MAX_CHANNELS; j++)
			{
				ChannelTexture *ctex = ctb->m_channelTextures + j;
				if (ctex->m_texture != NULL)
					ctex->m_texture->Release();
				if (ctex->m_srv != NULL)
					ctex->m_srv->Release();
				if (ctex->m_rtv != NULL)
					ctex->m_rtv->Release();
			}
		}
		m_alloc->Free(m_channelTextureBundles);
	}
	if (m_frameBytes)
		m_alloc->Free(m_frameBytes);
}

ID3D11Texture2D *lwmovie::lwmCD3D11FrameProvider::CreatePlaneTexture(lwmUInt32 width, lwmUInt32 height, bool isWriteOnly)
{
	D3D11_TEXTURE2D_DESC texture2Ddesc;
	memset(&texture2Ddesc, 0, sizeof(D3D11_TEXTURE2D_DESC));
	texture2Ddesc.Width = width;
	texture2Ddesc.Height = height;
	texture2Ddesc.MipLevels = 1;
	texture2Ddesc.ArraySize = 1;
	texture2Ddesc.Format = DXGI_FORMAT_R8_UNORM;
	texture2Ddesc.SampleDesc.Count = 1;
	texture2Ddesc.SampleDesc.Quality = 0;

	if (m_isUsingHardwareReconstructor)
	{
		texture2Ddesc.CPUAccessFlags = 0;
		texture2Ddesc.Usage = D3D11_USAGE_DEFAULT;
		texture2Ddesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	}
	else
	{
		texture2Ddesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texture2Ddesc.Usage = D3D11_USAGE_DYNAMIC;
		texture2Ddesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}

	texture2Ddesc.MiscFlags = 0;

	ID3D11Texture2D *texture2D;
	if (FAILED(m_device->CreateTexture2D(&texture2Ddesc, NULL, &texture2D)))
		return NULL;
	return texture2D;
}

ID3D11ShaderResourceView *lwmovie::lwmCD3D11FrameProvider::GetWorkFramePlaneSRV(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex)
{
	return m_channelTextureBundles[workFrameIndex].m_channelTextures[planeIndex].m_srv;
}

ID3D11RenderTargetView *lwmovie::lwmCD3D11FrameProvider::GetWorkFramePlaneRTV(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex)
{
	return m_channelTextureBundles[workFrameIndex].m_channelTextures[planeIndex].m_rtv;
}

int lwmovie::lwmCD3D11FrameProvider::CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat)
{
	lwmLargeUInt channelStrides[MAX_CHANNELS];
	lwmLargeUInt channelOffsets[MAX_CHANNELS + 1];
	lwmUInt32 channelWidths[MAX_CHANNELS];
	lwmUInt32 channelHeights[MAX_CHANNELS];
	lwmLargeUInt numChannels;

	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
	textureViewDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDesc.Texture2D.MipLevels = 1;
	textureViewDesc.Texture2D.MostDetailedMip = 0;

	switch (frameFormat)
	{
	case lwmFRAMEFORMAT_8Bit_420P_Planar:
		channelStrides[0] = workFrameWidth;
		channelStrides[1] = channelStrides[2] = (workFrameWidth + 1) / 2;
		channelWidths[0] = workFrameWidth;
		channelHeights[0] = workFrameHeight;
		channelWidths[1] = channelWidths[2] = workFrameWidth / 2;
		channelHeights[1] = channelHeights[2] = workFrameHeight / 2;
		numChannels = 3;
		break;
	default:
		return 0;
	};

	channelOffsets[0] = 0;
	for (lwmLargeUInt i = 0; i<numChannels; i++)
	{
		channelStrides[i] += (lwmovie::SIMD_ALIGN - 1);
		channelStrides[i] -= channelStrides[i] % lwmovie::SIMD_ALIGN;
		channelOffsets[i + 1] = channelOffsets[i] + channelStrides[i] * channelHeights[i];
	}

	for (lwmLargeUInt i = 0; i<numChannels; i++)
	{
		m_channelOffsets[i] = channelOffsets[i];
		m_channelStrides[i] = channelStrides[i];
		m_channelWidths[i] = channelWidths[i];
		m_channelHeights[i] = channelHeights[i];
	}

	m_frameSize = channelOffsets[numChannels];

	// TODO: Overflow check
	m_frameBytes = m_alloc->NAlloc<lwmUInt8>(m_frameSize * numRWFrames);
	if (!m_frameBytes)
		return 0;

	m_numChannels = numChannels;
	m_numDynFrames = numRWFrames + numWriteOnlyFrames;
	m_channelTextureBundles = m_alloc->NAlloc<ChannelTextureBundle>(m_numDynFrames);
	if (m_channelTextureBundles == NULL)
		return 0;

	memset(m_channelTextureBundles, 0, sizeof(ChannelTextureBundle) * m_numDynFrames);

	for (lwmLargeUInt i = 0; i < m_numDynFrames; i++)
	{
		ChannelTextureBundle *ctb = m_channelTextureBundles + i;
		for (lwmLargeUInt j = 0; j < numChannels; j++)
		{
			ChannelTexture *ctex = ctb->m_channelTextures + j;
			ID3D11Texture2D *texture = CreatePlaneTexture(m_channelWidths[j], m_channelHeights[j], j < numRWFrames);
			if (texture == NULL)
				return 0;

			ID3D11ShaderResourceView *srv;
			if (FAILED(m_device->CreateShaderResourceView(texture, &textureViewDesc, &srv)))
				return 0;

			if (m_isUsingHardwareReconstructor)
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
				memset(&rtvDesc, 0, sizeof(rtvDesc));

				rtvDesc.Format = DXGI_FORMAT_R8_UNORM;
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = 0;

				ID3D11RenderTargetView *rtv;
				if (FAILED(m_device->CreateRenderTargetView(texture, &rtvDesc, &rtv)))
					return 0;
				ctex->m_rtv = rtv;
			}
			
			ctex->m_texture = texture;
			ctex->m_srv = srv;
			ctex->m_activeLock = lwmVIDEOLOCK_None;
		}
		m_channelTextureBundles[i].m_frameBytes = m_frameBytes + m_frameSize * i;
	}

	return 1;
}

void lwmovie::lwmCD3D11FrameProvider::LockWorkFrame(lwmUInt32 workFrameIndex, lwmEVideoLockType lockType)
{
	printf("LockWorkFrame: %i / %i\n", workFrameIndex, lockType);
	ChannelTextureBundle *ctb = m_channelTextureBundles + workFrameIndex;
	for (lwmLargeUInt ch = 0; ch < m_numChannels; ch++)
	{
		ChannelTexture *ctex = ctb->m_channelTextures + ch;

		switch (lockType)
		{
		case lwmVIDEOLOCK_Write_ReadLater:
		case lwmVIDEOLOCK_Write_Only:
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			printf("Map %p\n", ctex->m_texture);
			if (FAILED(m_context->Map(ctex->m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
				return;
			ctex->m_activeLock = lockType;
			ctex->m_lockPitch = mappedResource.RowPitch;
			ctex->m_lockData = mappedResource.pData;
			break;
		case lwmVIDEOLOCK_Read:
			ctex->m_activeLock = lockType;
			break;
		};
	};
}

#include <intrin.h>

void lwmovie::lwmCD3D11FrameProvider::UnlockWorkFrame(lwmUInt32 workFrameIndex)
{
	printf("UnlockWorkFrame: %i\n", workFrameIndex);
	ChannelTextureBundle *ctb = m_channelTextureBundles + workFrameIndex;
	for (lwmLargeUInt ch = 0; ch < m_numChannels; ch++)
	{
		ChannelTexture *ctex = ctb->m_channelTextures + ch;
		if (ctex->m_activeLock == lwmVIDEOLOCK_Write_ReadLater)
		{
			// Copy the system memory portion into the hardware texture
			// TODO: SSE stream copy this!
			lwmUInt32 width = m_channelWidths[ch];
			lwmUInt32 height = m_channelHeights[ch];
			lwmUInt8 *destBytes = static_cast<lwmUInt8*>(ctex->m_lockData);
			const lwmUInt8 *srcBytes = ctb->m_frameBytes + this->m_channelOffsets[ch];
			for (lwmLargeUInt row = 0; row < m_channelHeights[ch]; row++)
			{
				memcpy(destBytes, srcBytes, width);
				destBytes += ctex->m_lockPitch;
				srcBytes += m_channelStrides[ch];
			}
			printf("Unmap %p\n", ctex->m_texture);
			m_context->Unmap(ctex->m_texture, 0);
		}
		else if (ctex->m_activeLock == lwmVIDEOLOCK_Write_Only)
		{
			m_context->Unmap(ctex->m_texture, 0);
		}
		ctex->m_activeLock = lwmVIDEOLOCK_None;
	}
}

void *lwmovie::lwmCD3D11FrameProvider::GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch)
{
	ChannelTextureBundle *ctb = m_channelTextureBundles + workFrameIndex;
	ChannelTexture *ctex = ctb->m_channelTextures + planeIndex;
	switch (ctex->m_activeLock)
	{
	case lwmVIDEOLOCK_Write_Only:
		if (outPitch)
			*outPitch = ctex->m_lockPitch;
		return ctex->m_lockData;
	case lwmVIDEOLOCK_Write_ReadLater:
	case lwmVIDEOLOCK_Read:
		if (outPitch)
			*outPitch = m_channelStrides[planeIndex];
		return ctb->m_frameBytes + m_channelOffsets[planeIndex];
	default:
		if (outPitch)
			*outPitch = 0;
		return NULL;
	}
}

lwmUInt32 lwmovie::lwmCD3D11FrameProvider::GetWorkFramePlaneWidth(lwmUInt32 planeIndex) const
{
	return this->m_channelWidths[planeIndex];
}

lwmUInt32 lwmovie::lwmCD3D11FrameProvider::GetWorkFramePlaneHeight(lwmUInt32 planeIndex) const
{
	return this->m_channelHeights[planeIndex];
}

void lwmovie::lwmCD3D11FrameProvider::Destroy()
{
	lwmCD3D11FrameProvider *self = this;
	lwmSAllocator *alloc = self->m_alloc;
	self->~lwmCD3D11FrameProvider();
	alloc->Free(self);
}

LWMOVIE_API_LINK lwmSVideoFrameProvider *lwmCreateD3D11FrameProvider(lwmSAllocator *alloc, ID3D11Device *device, ID3D11DeviceContext *context, int isUsingHardwareReconstructor)
{
	lwmovie::lwmCD3D11FrameProvider *fp = alloc->NAlloc<lwmovie::lwmCD3D11FrameProvider>(1);
	if (!fp)
		return NULL;
	new (fp)lwmovie::lwmCD3D11FrameProvider(alloc, device, context, (isUsingHardwareReconstructor != 0));
	return fp;
}

LWMOVIE_API_LINK ID3D11ShaderResourceView *lwmD3D11FrameProvider_GetWorkFramePlaneSRV(lwmSVideoFrameProvider *vfp, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex)
{
	return static_cast<lwmovie::lwmCD3D11FrameProvider*>(vfp)->GetWorkFramePlaneSRV(workFrameIndex, planeIndex);
}

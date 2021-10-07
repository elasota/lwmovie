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
#include <string.h>
#include <d3d11.h>
#include <new>
#include "../common/lwmovie_atomicint_funcs.hpp"
#include "../common/lwmovie_coretypes.h"
#include "lwmovie.h"
#include "lwmovie_recon_d3d11.hpp"
#include "lwmovie_simd_defs.hpp"
#include "lwmovie_profile.hpp"
#include "lwmovie_idct.hpp"
#include "lwmovie_fp_d3d11.hpp"

#ifdef LWMOVIE_SSE2
#include "lwmovie_recon_m1vsw_sse2.inl"
#endif

#ifdef LWMOVIE_NOSIMD
#include "lwmovie_recon_m1vsw_c.inl"
#endif

#include "d3d11/shaders/commondefs.h"

namespace lwmovie
{
	namespace d3d11
	{
		struct SDCTBufferBlock
		{
			lwmSInt32 coeffs[32];
		};

		struct ReconVertex
		{
			lwmFloat32 position[2];
			lwmFloat32 texCoord[2];
		};

		namespace shaderblobs
		{
			const lwmUInt8 idct_cs[] = {
				#include "d3d11/shaders/idct_cs.compiled.h"
			};
			const lwmUInt8 recon_vs[] = {
				#include "d3d11/shaders/recon_vs.compiled.h"
			};
			const lwmUInt8 recon_ps_luma[] = {
				#include "d3d11/shaders/recon_ps_luma.compiled.h"
			};
			const lwmUInt8 recon_ps_chroma[] = {
				#include "d3d11/shaders/recon_ps_chroma.compiled.h"
			};
		}
	}
}


lwmovie::d3d11::CM1VReconstructor::CM1VReconstructor(lwmSAllocator *alloc, lwmMovieState *movieState, lwmSVideoFrameProvider *frameProvider, ID3D11Device *device, ID3D11DeviceContext *context, bool isMPEG2)
	: m_alloc(alloc)
	, m_movieState(movieState)
	, m_frameProvider(static_cast<lwmovie::d3d11::CFrameProvider*>(frameProvider))
	, m_device(device)
	, m_context(context)
	, m_dctInputBuffer(NULL)
	, m_mblockInfoBuffer(NULL)
	, m_dctOutputBuffer(NULL)
	, m_psConstantBuffer(NULL)
	, m_mappedDCTInputs(NULL)
	, m_idctCS(NULL)
	, m_reconVS(NULL)
	, m_reconIA(NULL)
	, m_reconVertexBuffer(NULL)
	, m_planeLinearSampler(NULL)
	, m_solidRasterState(NULL)
	, m_reconLumaPS(NULL)
	, m_reconChromaPS(NULL)
	, m_mbWidth(0)
	, m_mbHeight(0)
	, m_forwPred(false)
	, m_backPred(false)
	, m_workingOnFrame(false)
	, m_isMPEG2(isMPEG2)
	, m_current(0)
	, m_future(0)
	, m_past(0)
	, m_presentedFrame(0)

{
}

lwmovie::d3d11::CM1VReconstructor::~CM1VReconstructor()
{
	if (m_context)
		this->CloseIDCTMap();

	IUnknown *cleanups[] =
	{
		m_dctInputBuffer,
		m_dctOutputBuffer,
		m_psConstantBuffer,
		m_mblockInfoBuffer,

		m_idctCS,

		m_reconVS,
		m_reconLumaPS,
		m_reconChromaPS,
		m_reconIA,
		m_reconVertexBuffer,

		m_dctInputSRV,
		m_dctOutputSRV,
		m_dctOutputUAV,
		
		m_solidRasterState,

		m_planeLinearSampler,
	};

	for (int i = 0; i < sizeof(cleanups) / sizeof(cleanups[0]); i++)
	{
		if (cleanups[i])
			cleanups[i]->Release();
	}
}

void lwmovie::d3d11::CM1VReconstructor::Participate()
{
}

void lwmovie::d3d11::CM1VReconstructor::WaitForFinish()
{
}

void lwmovie::d3d11::CM1VReconstructor::SetWorkNotifier(lwmSWorkNotifier *workNotifier)
{
}

void lwmovie::d3d11::CM1VReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
}

lwmUInt32 lwmovie::d3d11::CM1VReconstructor::GetWorkFrameIndex() const
{
	return m_presentedFrame;
}

void lwmovie::d3d11::CM1VReconstructor::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	this->~CM1VReconstructor();
	alloc->Free(this);
}

lwmSVideoFrameProvider *lwmovie::d3d11::CM1VReconstructor::GetFrameProvider() const
{
	return this->m_frameProvider;
}

lwmovie::m1v::IM1VBlockCursor *lwmovie::d3d11::CM1VReconstructor::CreateBlockCursor()
{
	CM1VBlockCursor *blockCursor = m_alloc->NAlloc<CM1VBlockCursor>(1);
	if (!blockCursor)
		return NULL;
	new (blockCursor) CM1VBlockCursor(this);
	return blockCursor;
}

void lwmovie::d3d11::CM1VReconstructor::MarkRowFinished(lwmSInt32 firstMBAddress)
{
}


void lwmovie::d3d11::CM1VReconstructor::StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past, bool currentIsB)
{
	this->CloseFrame();

	if (current == m1v::lwmRECONSLOT_Dropped_IP || current == m1v::lwmRECONSLOT_Dropped_B)
		return;

	m_current = current - 1;
	m_forwPred = (future && future != current);
	m_backPred = (past && past != current);

	if (m_forwPred)
		m_future = future - 1;
	if (m_backPred)
		m_past = past - 1;

	m_workingOnFrame = true;

	OpenIDCTMap();
}

void lwmovie::d3d11::CM1VReconstructor::OpenIDCTMap()
{
	m_mappedDCTInputs = NULL;
	m_mappedMBlockInfos = NULL;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_context->Map(this->m_dctInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return;
	this->m_mappedDCTInputs = mappedResource.pData;

	if (FAILED(m_context->Map(this->m_mblockInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return;
	this->m_mappedMBlockInfos = mappedResource.pData;
}

void lwmovie::d3d11::CM1VReconstructor::CloseFrame()
{
	if (!m_workingOnFrame)
		return;

	CloseIDCTMap();

	// Execute IDCTs
	{
		ID3D11ShaderResourceView *srvs[] =
		{
			m_dctInputSRV,
			m_mblockInfoSRV,
		};
		m_context->ClearState();
		m_context->CSSetShader(m_idctCS, NULL, 0);
		m_context->CSSetShaderResources(0, sizeof(srvs) / sizeof(srvs[0]), srvs);
		m_context->CSSetUnorderedAccessViews(0, 1, &m_dctOutputUAV, NULL);

		lwmUInt32 numJobs = (m_mbHeight * m_mbWidth * 6 + LWMOVIE_D3D11_IDCT_NUMTHREADS - 1) / LWMOVIE_D3D11_IDCT_NUMTHREADS;
		m_context->Dispatch(numJobs, 1, 1);
	}

	// Reconstruct luma frame
	{
		ID3D11ShaderResourceView *forwSRV = NULL;
		ID3D11ShaderResourceView *backSRV = NULL;
		if (m_forwPred)
			forwSRV = m_frameProvider->GetWorkFramePlaneSRV(m_future, 0);
		if (m_backPred)
			backSRV = m_frameProvider->GetWorkFramePlaneSRV(m_past, 0);

		ID3D11ShaderResourceView *shaderResourceViews[] =
		{
			m_dctOutputSRV,
			m_mblockInfoSRV,
			forwSRV,
			backSRV,
		};

		D3D11_VIEWPORT viewport;
		memset(&viewport, 0, sizeof(viewport));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		viewport.Width = static_cast<lwmFloat32>(m_mbWidth * 16);
		viewport.Height = static_cast<lwmFloat32>(m_mbHeight * 16);

		UINT vertStride = sizeof(lwmovie::d3d11::ReconVertex);
		UINT vertOffset = 0;

		ID3D11RenderTargetView *rtv = m_frameProvider->GetWorkFramePlaneRTV(m_current, 0);

		m_context->ClearState();
		//m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_context->IASetInputLayout(this->m_reconIA);
		m_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetVertexBuffers(0, 1, &m_reconVertexBuffer, &vertStride, &vertOffset);
		m_context->VSSetShader(m_reconVS, NULL, 0);
		m_context->PSSetShader(m_reconLumaPS, NULL, 0);
		m_context->PSSetConstantBuffers(0, 1, &m_psConstantBuffer);
		m_context->PSSetSamplers(0, 1, &m_planeLinearSampler);
		m_context->PSSetShaderResources(0, sizeof(shaderResourceViews) / sizeof(shaderResourceViews[0]), shaderResourceViews);
		m_context->RSSetViewports(1, &viewport);
		m_context->RSSetState(m_solidRasterState);
		m_context->OMSetRenderTargets(1, &rtv, NULL);
		m_context->Draw(6, 0);
	}

	// Reconstruct chroma frames frame
	{
		ID3D11ShaderResourceView *forwSRV[2] = { NULL, NULL };
		ID3D11ShaderResourceView *backSRV[2] = { NULL, NULL };

		if (m_forwPred)
		{
			forwSRV[0] = m_frameProvider->GetWorkFramePlaneSRV(m_future, 1);
			forwSRV[1] = m_frameProvider->GetWorkFramePlaneSRV(m_future, 2);
		}
		if (m_backPred)
		{
			backSRV[0] = m_frameProvider->GetWorkFramePlaneSRV(m_past, 1);
			backSRV[1] = m_frameProvider->GetWorkFramePlaneSRV(m_past, 2);
		}

		ID3D11ShaderResourceView *shaderResourceViews[] =
		{
			m_dctOutputSRV,
			m_mblockInfoSRV,
			forwSRV[0],
			forwSRV[1],
			backSRV[0],
			backSRV[1],
		};

		D3D11_VIEWPORT viewport;
		memset(&viewport, 0, sizeof(viewport));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		viewport.Width = static_cast<lwmFloat32>(m_mbWidth * 8);
		viewport.Height = static_cast<lwmFloat32>(m_mbHeight * 8);

		UINT vertStride = sizeof(lwmovie::d3d11::ReconVertex);
		UINT vertOffset = 0;

		ID3D11RenderTargetView *rtvs[] =
		{
			NULL,
			NULL,
		};
		rtvs[0] = m_frameProvider->GetWorkFramePlaneRTV(m_current, 1);
		rtvs[1] = m_frameProvider->GetWorkFramePlaneRTV(m_current, 2);

		m_context->ClearState();
		//m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_context->IASetInputLayout(this->m_reconIA);
		m_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetVertexBuffers(0, 1, &m_reconVertexBuffer, &vertStride, &vertOffset);
		m_context->VSSetShader(m_reconVS, NULL, 0);
		m_context->PSSetShader(m_reconChromaPS, NULL, 0);
		m_context->PSSetConstantBuffers(0, 1, &m_psConstantBuffer);
		m_context->PSSetSamplers(0, 1, &m_planeLinearSampler);
		m_context->PSSetShaderResources(0, sizeof(shaderResourceViews) / sizeof(shaderResourceViews[0]), shaderResourceViews);
		m_context->RSSetViewports(1, &viewport);
		m_context->RSSetState(m_solidRasterState);
		m_context->OMSetRenderTargets(sizeof(rtvs) / sizeof(rtvs[0]), rtvs, NULL);
		m_context->Draw(6, 0);
	}
}

void lwmovie::d3d11::CM1VReconstructor::CloseIDCTMap()
{
	if (m_mappedDCTInputs)
		m_context->Unmap(m_dctInputBuffer, 0);
	if (m_mappedMBlockInfos)
		m_context->Unmap(m_mblockInfoBuffer, 0);
	m_mappedDCTInputs = NULL;
	m_mappedMBlockInfos = NULL;
}

void lwmovie::d3d11::CM1VReconstructor::PresentFrame(lwmUInt32 outFrame)
{
	CloseFrame();
	m_presentedFrame = outFrame - 1;
}

bool lwmovie::d3d11::CM1VReconstructor::Initialize(lwmUInt32 width, lwmUInt32 height)
{
	lwmUInt32 mbWidth = (width + 15) / 16;
	lwmUInt32 mbHeight = (height + 15) / 16;
	lwmUInt32 numMB = mbWidth * mbHeight;
	lwmUInt32 numDCTBlocks = numMB * 6;

	lwmUInt32 numRWFrames, numWFrames;
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumReadWriteWorkFrames, &numRWFrames);
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames, &numWFrames);

	if (numWFrames > 1 || numRWFrames != 2)
		return false;

	if (!m_frameProvider->createWorkFramesFunc(m_frameProvider, numRWFrames, numWFrames, mbWidth * 16, mbHeight * 16, lwmFRAMEFORMAT_8Bit_420P_Planar))
		return false;

	m_mbWidth = mbWidth;
	m_mbHeight = mbHeight;

	{
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = numDCTBlocks * sizeof(lwmovie::d3d11::SDCTBufferBlock);
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeof(lwmovie::d3d11::SDCTBufferBlock);
		if (FAILED(m_device->CreateBuffer(&bufferDesc, NULL, &m_dctInputBuffer)))
			return false;
	}

	{
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = numMB * sizeof(lwmovie::d3d11::SMBlockReconInfo);
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeof(lwmovie::d3d11::SMBlockReconInfo);
		if (FAILED(m_device->CreateBuffer(&bufferDesc, NULL, &m_mblockInfoBuffer)))
			return false;
	}

	{
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = numDCTBlocks * sizeof(lwmovie::d3d11::SDCTBufferBlock);
		bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeof(lwmovie::d3d11::SDCTBufferBlock);
		if (FAILED(m_device->CreateBuffer(&bufferDesc, NULL, &m_dctOutputBuffer)))
			return false;
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC inputBufferViewDesc;
		memset(&inputBufferViewDesc, 0, sizeof(inputBufferViewDesc));
		inputBufferViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		inputBufferViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		inputBufferViewDesc.Buffer.FirstElement = 0;
		inputBufferViewDesc.Buffer.NumElements = numDCTBlocks;
		if (FAILED(m_device->CreateShaderResourceView(m_dctInputBuffer, &inputBufferViewDesc, &m_dctInputSRV)))
			return false;
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC inputBufferViewDesc;
		memset(&inputBufferViewDesc, 0, sizeof(inputBufferViewDesc));
		inputBufferViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		inputBufferViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		inputBufferViewDesc.Buffer.FirstElement = 0;
		inputBufferViewDesc.Buffer.NumElements = numDCTBlocks;
		if (FAILED(m_device->CreateShaderResourceView(m_dctOutputBuffer, &inputBufferViewDesc, &m_dctOutputSRV)))
			return false;
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC mblockInfoViewDesc;
		memset(&mblockInfoViewDesc, 0, sizeof(mblockInfoViewDesc));
		mblockInfoViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		mblockInfoViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		mblockInfoViewDesc.Buffer.FirstElement = 0;
		mblockInfoViewDesc.Buffer.NumElements = numMB;
		if (FAILED(m_device->CreateShaderResourceView(m_mblockInfoBuffer, &mblockInfoViewDesc, &m_mblockInfoSRV)))
			return false;
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		memset(&uavDesc, 0, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = numDCTBlocks;
		if (FAILED(m_device->CreateUnorderedAccessView(m_dctOutputBuffer, &uavDesc, &m_dctOutputUAV)))
			return false;
	}

	{
		lwmovie::d3d11::SReconConstants reconConstants;
		reconConstants.fDimensionsL[0] = static_cast<lwmFloat32>(m_mbWidth * 16);
		reconConstants.fDimensionsL[1] = static_cast<lwmFloat32>(m_mbHeight * 16);
		reconConstants.fDimensionsC[0] = static_cast<lwmFloat32>(m_mbWidth * 8);
		reconConstants.fDimensionsC[1] = static_cast<lwmFloat32>(m_mbHeight * 8);
		reconConstants.mbDimensions[0] = m_mbWidth;
		reconConstants.mbDimensions[1] = m_mbHeight;

		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(reconConstants);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(bufferDesc);

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &reconConstants;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(m_device->CreateBuffer(&bufferDesc, &initData, &m_psConstantBuffer)))
			return false;
	}

	if (FAILED(m_device->CreateComputeShader(lwmovie::d3d11::shaderblobs::idct_cs, sizeof(lwmovie::d3d11::shaderblobs::idct_cs), NULL, &m_idctCS)))
		return false;
	if (FAILED(m_device->CreateVertexShader(lwmovie::d3d11::shaderblobs::recon_vs, sizeof(lwmovie::d3d11::shaderblobs::recon_vs), NULL, &m_reconVS)))
		return false;
	if (FAILED(m_device->CreatePixelShader(lwmovie::d3d11::shaderblobs::recon_ps_luma, sizeof(lwmovie::d3d11::shaderblobs::recon_ps_luma), NULL, &m_reconLumaPS)))
		return false;
	if (FAILED(m_device->CreatePixelShader(lwmovie::d3d11::shaderblobs::recon_ps_chroma, sizeof(lwmovie::d3d11::shaderblobs::recon_ps_chroma), NULL, &m_reconChromaPS)))
		return false;

	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		if (FAILED(m_device->CreateInputLayout(layout, sizeof(layout) / sizeof(layout[0]), lwmovie::d3d11::shaderblobs::recon_vs, sizeof(lwmovie::d3d11::shaderblobs::recon_vs), &this->m_reconIA)))
			return false;
	}

	{
		D3D11_SAMPLER_DESC textureSamplerDesc;
		textureSamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		textureSamplerDesc.MipLODBias = 0.f;
		textureSamplerDesc.MaxAnisotropy = 0;
		textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		memset(textureSamplerDesc.BorderColor, 0, sizeof(textureSamplerDesc.BorderColor));
		textureSamplerDesc.MinLOD = 0.f;
		textureSamplerDesc.MaxLOD = 0.f;

		if (FAILED(m_device->CreateSamplerState(&textureSamplerDesc, &m_planeLinearSampler)))
			return false;
	}

	{
		D3D11_RASTERIZER_DESC rasterDesc;
		memset(&rasterDesc, 0, sizeof(rasterDesc));

		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FillMode = D3D11_FILL_SOLID;

		if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_solidRasterState)))
			return false;
	}

	{
		lwmovie::d3d11::ReconVertex verts[] =
		{
			{ { -1.f, 1.f }, { 0.f, 0.f } },
			{ { 1.f, 1.f }, { 1.f, 0.f } },
			{ { 1.0f, -1.f }, { 1.f, 1.f } },

			{ { -1.f, 1.f }, { 0.f, 0.f } },
			{ { 1.0f, -1.f }, { 1.f, 1.f } },
			{ { -1.0f, -1.f }, { 0.f, 1.f } },
		};

		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(verts);
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(verts[0]);

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = verts;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(m_device->CreateBuffer(&bufferDesc, &initData, &m_reconVertexBuffer)))
			return false;
	}

	return true;
}

lwmovie::d3d11::SDCTBufferBlock *lwmovie::d3d11::CM1VReconstructor::GetPackedDCTBlock(lwmUInt32 blockAddress)
{
	return static_cast<lwmovie::d3d11::SDCTBufferBlock*>(this->m_mappedDCTInputs) + blockAddress;
}

lwmovie::d3d11::SMBlockReconInfo *lwmovie::d3d11::CM1VReconstructor::GetMBlockInfo(lwmUInt32 mblockAddress)
{
	return static_cast<lwmovie::d3d11::SMBlockReconInfo*>(this->m_mappedMBlockInfos) + mblockAddress;
}

//==============================================================================================================
lwmovie::d3d11::CM1VBlockCursor::CM1VBlockCursor(CM1VReconstructor *reconstructor)
	: m_mbAddress(0)
	, m_reconstructor(reconstructor)
	, m_activeDCTBlock(NULL)
{
}

lwmovie::d3d11::CM1VBlockCursor::~CM1VBlockCursor()
{
}

void lwmovie::d3d11::CM1VBlockCursor::OpenMB(lwmSInt32 mbAddress)
{
	m_mbAddress = mbAddress;
	memset(&m_reconMBlockInfo, 0, sizeof(m_reconMBlockInfo));
}

void lwmovie::d3d11::CM1VBlockCursor::CloseMB()
{
	*m_reconstructor->GetMBlockInfo(m_mbAddress) = m_reconMBlockInfo;
}

void lwmovie::d3d11::CM1VBlockCursor::SetMBlockInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back,
	lwmSInt32 recon_right_for, lwmSInt32 recon_down_for,
	lwmSInt32 recon_right_back, lwmSInt32 recon_down_back)
{
	this->m_reconMBlockInfo.motionVectors[0] = recon_right_for;
	this->m_reconMBlockInfo.motionVectors[1] = recon_down_for;
	this->m_reconMBlockInfo.motionVectors[2] = recon_right_back;
	this->m_reconMBlockInfo.motionVectors[3] = recon_down_back;
	this->m_reconMBlockInfo.flags = 0;	// Clear MV flags
	if (mb_motion_forw)
		this->m_reconMBlockInfo.flags |= LWMOVIE_D3D11_BLOCK_FLAG_FORW_MOTION;
	if (mb_motion_back)
		this->m_reconMBlockInfo.flags |= LWMOVIE_D3D11_BLOCK_FLAG_BACK_MOTION;
	if (skipped)
		this->m_reconMBlockInfo.flags |= 0x3f;
}

void lwmovie::d3d11::CM1VBlockCursor::SetBlockInfo(lwmSInt32 blockIndex, bool zero_block_flag)
{
	if (zero_block_flag)
		this->m_reconMBlockInfo.flags |= (1 << blockIndex);
	else
		this->m_reconMBlockInfo.flags &= ~(1 << blockIndex);
}

lwmovie::idct::DCTBLOCK *lwmovie::d3d11::CM1VBlockCursor::StartReconBlock(lwmSInt32 subBlockIndex)
{
	m_activeDCTBlock = reinterpret_cast<idct::DCTBLOCK*>(m_reconstructor->GetPackedDCTBlock(m_mbAddress * 6 + subBlockIndex));
	return m_activeDCTBlock;
}

void lwmovie::d3d11::CM1VBlockCursor::CommitZero()
{
	memset(m_activeDCTBlock, 0, sizeof(idct::DCTBLOCK));
}

void lwmovie::d3d11::CM1VBlockCursor::CommitSparse(lwmUInt8 lastCoeffPosAndParity, lwmSInt16 lastCoeff)
{
	m_activeDCTBlock->FastZeroFill();
	m_activeDCTBlock->data[63] = lastCoeffPosAndParity >> 6;
	m_activeDCTBlock->data[lastCoeffPosAndParity & 63] = lastCoeff;
}

void lwmovie::d3d11::CM1VBlockCursor::CommitFull()
{
}

///////////////////////////////////////////////////////////////////////////////
// C API
LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCreateD3D11VideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, ID3D11Device *device, ID3D11DeviceContext *context, lwmSVideoFrameProvider *frameProvider)
{
	switch (reconstructorType)
	{
	case lwmRC_MPEG1Video:
	case lwmRC_MPEG2Video:
		{
			lwmUInt32 width, height;

			lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &width);
			lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &height);

			lwmovie::d3d11::CM1VReconstructor *recon = alloc->NAlloc<lwmovie::d3d11::CM1VReconstructor>(1);
			if (!recon)
				return NULL;
			new (recon) lwmovie::d3d11::CM1VReconstructor(alloc, movieState, frameProvider, device, context, reconstructorType == lwmRC_MPEG2Video);
			// TODO: Low memory flag
			if (!recon->Initialize(width, height))
			{
				lwmIVideoReconstructor_Destroy(recon);
				recon = NULL;
			}
			return recon;
		}
		break;
	};
	return NULL;
}

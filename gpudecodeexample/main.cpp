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
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <D3Dcompiler.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lwmovie/lwmovie.h"
#include "../lwmovie/lwmovie_cake_cppshims.hpp"
#include "../lwmovie/lwmovie_cpp_shims.hpp"
#include "../lwmovie/lwmovie_cake.h"

#include "../lwplay/lwplay_interfaces.hpp"

#include "d3d11_shaders.h"

// Stupid macros
#undef CreateWindow

HWND g_hwindow;

class Allocator : public lwmIAllocator
{
public:
	virtual void *Realloc(void *ptr, lwmLargeUInt sz)
	{
		return _aligned_realloc(ptr, sz, 16);
	}
};

class FileReader : public lwmICakeFileReader
{
private:
	FILE *m_f;

public:
	bool IsEOF()
	{
		return (feof(m_f) != 0);
	}

	lwmLargeUInt ReadBytes(void *dest, lwmLargeUInt numBytes)
	{
		return fread(dest, 1, numBytes, m_f);
	}

	explicit FileReader(FILE *f)
	{
		m_f = f;
	}
};

class TimeReader : public lwmICakeTimeReader
{
public:
	lwmUInt64 GetTimeMilliseconds()
	{
		return GetTickCount64();
	}

	lwmUInt32 GetResolutionMilliseconds()
	{
		return 10;
	}
};

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

void CreateWindow(lwmUInt32 width, lwmUInt32 height)
{
	const char *name = "lwmovie GPU Decode Example";
	const char *className = "lwmovieplayer";
	WNDCLASSEX windowClass;
	HINSTANCE hinstance;

	hinstance = GetModuleHandle(NULL);

	memset(&windowClass, 0, sizeof(windowClass));
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = MyWndProc;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.lpszClassName = className;
	windowClass.cbSize = sizeof(windowClass);

	ATOM test = RegisterClassExA(&windowClass);

	RECT windowRect;
	windowRect.left = 60;
	windowRect.top = 60;
	windowRect.right = windowRect.left + width;
	windowRect.bottom = windowRect.top + height;

	DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	BOOL adjusted = AdjustWindowRectEx(&windowRect, windowStyle | WS_OVERLAPPEDWINDOW, FALSE, 0);

	g_hwindow = CreateWindowExA(WS_EX_APPWINDOW, className, name, windowStyle | WS_OVERLAPPEDWINDOW, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hinstance, NULL);
	DWORD error = GetLastError();

	ShowWindow(g_hwindow, SW_SHOW);
	SetForegroundWindow(g_hwindow);
	SetFocus(g_hwindow);
}

ID3DBlob *CompileShaderString(const char *shaderString, const char *mainProc, const char *fileName, const char *target)
{
	ID3DBlob *compiledShader;
	ID3DBlob *errors;
	D3DCompile(shaderString, strlen(shaderString), fileName, NULL, NULL, mainProc, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &compiledShader, &errors);

	if(errors)
	{
		fprintf(stderr, "%s", errors->GetBufferPointer());
		errors->Release();
		return NULL;
	}

	return compiledShader;
}

ID3D11VertexShader *CreateVertexShader(ID3D11Device *device, ID3DBlob *blob)
{
	ID3D11VertexShader *shader;
	device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &shader);
	blob->Release();
	return shader;
}

ID3D11PixelShader *CreatePixelShader(ID3D11Device *device, ID3D10Blob *blob)
{
	ID3D11PixelShader *shader;
	SIZE_T size = blob->GetBufferSize();
	void *data = blob->GetBufferPointer();
	device->CreatePixelShader(data, size, NULL, &shader);
	blob->Release();
	return shader;
}


struct renderVertex
{
	float position[2];
	float texCoord[2];
};

struct renderGlobals
{
	IDXGIFactory *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput *output;

	IDXGISwapChain *swapChain;
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	ID3D11Resource *backBuffer;
	ID3D11RenderTargetView *renderTargetView;

	ID3D11VertexShader *vs;
	ID3D11PixelShader *ps_ycbcr;
	ID3D11PixelShader *ps_rgb;

	ID3D11RasterizerState *rasterState;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *ycbcrWeightsBuffer;

	ID3D11InputLayout *inputLayout;

	ID3D11SamplerState *textureSamplerState;
};

renderGlobals g_renderGlobals;

void InitD3D(lwmUInt32 width, lwmUInt32 height, lwmCake *cake, lwmSAllocator *alloc)
{
	CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&g_renderGlobals.factory));
	g_renderGlobals.factory->EnumAdapters(0, &g_renderGlobals.adapter);
	g_renderGlobals.adapter->EnumOutputs(0, &g_renderGlobals.output);

	UINT numModes;
	g_renderGlobals.output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL);

	DXGI_MODE_DESC *modeList = new DXGI_MODE_DESC[numModes];
	g_renderGlobals.output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modeList);

	DXGI_ADAPTER_DESC adapterDesc;
	g_renderGlobals.adapter->GetDesc(&adapterDesc);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = g_hwindow;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL askFeatureLevels[] = { D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL featureLevel;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, askFeatureLevels, sizeof(askFeatureLevels) / sizeof(askFeatureLevels[0]),
		D3D11_SDK_VERSION, &swapChainDesc, &g_renderGlobals.swapChain, &g_renderGlobals.device, &featureLevel, &g_renderGlobals.deviceContext);
	g_renderGlobals.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&g_renderGlobals.backBuffer));
	g_renderGlobals.device->CreateRenderTargetView(g_renderGlobals.backBuffer, NULL, &g_renderGlobals.renderTargetView);

	g_renderGlobals.backBuffer->Release();
	g_renderGlobals.backBuffer = NULL;

	D3D11_RASTERIZER_DESC rasterDesc;
	memset(&rasterDesc, 0, sizeof(rasterDesc));

	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;

	g_renderGlobals.device->CreateRasterizerState(&rasterDesc, &g_renderGlobals.rasterState);

	ID3DBlob *vsBlob = CompileShaderString(displayVS, "mainVS", "vs.hlsl", "vs_4_0");
	vsBlob->AddRef();
	g_renderGlobals.vs = CreateVertexShader(g_renderGlobals.device, vsBlob);
	g_renderGlobals.ps_ycbcr = CreatePixelShader(g_renderGlobals.device, CompileShaderString(displayPS_YCbCr, "mainPS", "ps.hlsl", "ps_4_0"));
	g_renderGlobals.ps_rgb = CreatePixelShader(g_renderGlobals.device, CompileShaderString(displayPS_RGB, "mainPS", "ps.hlsl", "ps_4_0"));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	g_renderGlobals.device->CreateInputLayout(layout, sizeof(layout) / sizeof(layout[0]), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_renderGlobals.inputLayout);
	vsBlob->Release();

	D3D11_SAMPLER_DESC textureSamplerDesc;
	textureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	textureSamplerDesc.MipLODBias = 0.f;
	textureSamplerDesc.MaxAnisotropy = 0;
	textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	memset(textureSamplerDesc.BorderColor, 0, sizeof(textureSamplerDesc.BorderColor));
	textureSamplerDesc.MinLOD = 0.f;
	textureSamplerDesc.MaxLOD = 0.f;

	g_renderGlobals.device->CreateSamplerState(&textureSamplerDesc, &g_renderGlobals.textureSamplerState);
}

void CreateRenderingConstants(lwmCake *cake)
{
	{
		lwmCakeYCbCrWeights weights;
		lwmCake_GetYCbCrWeights(cake, &weights);

		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(lwmCakeYCbCrWeights);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(weights);

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &weights;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		g_renderGlobals.device->CreateBuffer(&bufferDesc, &initData, &g_renderGlobals.ycbcrWeightsBuffer);
	}

	{
		lwmFloat32 topLeft[2];
		lwmFloat32 bottomRight[2];

		lwmCake_GetDrawTexCoords(cake, topLeft, bottomRight);
		renderVertex verts[] =
		{
			{ { -1.f, 1.f }, { topLeft[0], topLeft[1] } },	// Upper-left
			{ { 1.f, 1.f }, { bottomRight[0], topLeft[1]  } },	// Upper-right
			{ { 1.f, -1.f }, { bottomRight[0], bottomRight[1] } },	// Lower-right

			{ { -1.f, 1.f }, { topLeft[0], topLeft[1] } },	// Upper-left
			{ { 1.f, -1.f }, { bottomRight[0], bottomRight[1] } },	// Lower-right
			{ { -1.f, -1.f }, { topLeft[0], bottomRight[1] } },	// Lower-left
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

		g_renderGlobals.device->CreateBuffer(&bufferDesc, &initData, &g_renderGlobals.vertexBuffer);
	}
}

void DisplayCakeFrame(lwmCake *cake, lwmUInt32 workFrameIndex, lwmUInt32 width, lwmUInt32 height, bool isYCbCr)
{
	UINT vertexStride = sizeof(renderVertex);
	UINT vertexOffset = 0;

	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);

	lwmSVideoFrameProvider *vfp = lwmCake_GetVideoFrameProvider(cake);

	g_renderGlobals.deviceContext->ClearState();

	if (isYCbCr)
	{
		ID3D11ShaderResourceView *srvs[] =
		{
			lwmD3D11FrameProvider_GetWorkFramePlaneSRV(vfp, workFrameIndex, 0),
			lwmD3D11FrameProvider_GetWorkFramePlaneSRV(vfp, workFrameIndex, 1),
			lwmD3D11FrameProvider_GetWorkFramePlaneSRV(vfp, workFrameIndex, 2),
		};
		g_renderGlobals.deviceContext->PSSetShaderResources(0, sizeof(srvs) / sizeof(srvs[0]), srvs);
		g_renderGlobals.deviceContext->PSSetConstantBuffers(0, 1, &g_renderGlobals.ycbcrWeightsBuffer);
		g_renderGlobals.deviceContext->PSSetShader(g_renderGlobals.ps_ycbcr, NULL, 0);
	}
	else
	{
		ID3D11ShaderResourceView *srvs[] =
		{
			lwmD3D11FrameProvider_GetWorkFramePlaneSRV(vfp, workFrameIndex, 0),
		};
		g_renderGlobals.deviceContext->PSSetShaderResources(0, sizeof(srvs) / sizeof(srvs[0]), srvs);
		g_renderGlobals.deviceContext->PSSetShader(g_renderGlobals.ps_rgb, NULL, 0);
	}

	g_renderGlobals.deviceContext->IASetInputLayout(g_renderGlobals.inputLayout);
	g_renderGlobals.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_renderGlobals.deviceContext->IASetVertexBuffers(0, 1, &g_renderGlobals.vertexBuffer, &vertexStride, &vertexOffset);
	g_renderGlobals.deviceContext->PSSetSamplers(0, 1, &g_renderGlobals.textureSamplerState);
	g_renderGlobals.deviceContext->RSSetViewports(1, &viewport);
	g_renderGlobals.deviceContext->RSSetState(g_renderGlobals.rasterState);
	g_renderGlobals.deviceContext->VSSetShader(g_renderGlobals.vs, NULL, 0);
	g_renderGlobals.deviceContext->OMSetRenderTargets(1, &g_renderGlobals.renderTargetView, NULL);
	g_renderGlobals.deviceContext->Draw(6, 0);
	g_renderGlobals.swapChain->Present(0, 0);
}

int main(int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: gpudecodeexample input.lwmv");
		return -1;
	}

	FILE *inputFile = fopen(argv[1], "rb");
	if(!inputFile)
		return -1;

	Allocator myAlloc;
	FileReader myFileReader(inputFile);
	TimeReader myTimeReader;

	lwmInitialize();

	lwmCake *cake;
	lwmCakeCreateOptions createOptions;
	memset(&createOptions, 0, sizeof(createOptions));
	cake = lwmCake_Create(&myAlloc, &myFileReader, &myTimeReader, &createOptions);

	lwmCakeMovieInfo movieInfo;
	while(true)
	{
		lwmECakeResult cakeResult = lwmCake_ReadMovieInfo(cake, &movieInfo);
		if(cakeResult == lwmCAKE_RESULT_Waiting)
			continue;
		else if(cakeResult == lwmCAKE_RESULT_Initialized)
			break;
		else if(cakeResult == lwmCAKE_RESULT_Error)
			return 0;
	}

	CreateWindow(movieInfo.videoWidth, movieInfo.videoHeight);
	InitD3D(movieInfo.videoWidth, movieInfo.videoHeight, cake, &myAlloc);

	lwmCakeDecodeOptions decodeOptions;
	memset(&decodeOptions, 0, sizeof(decodeOptions));
	if (!lwmCake_SetD3D11DecodeOptions(cake, &decodeOptions, g_renderGlobals.device, g_renderGlobals.deviceContext, 1))
	{
		fprintf(stderr, "Error occurred while setting D3D11 decode options");
		return 0;
	}

	bool canPlay = (lwmCake_BeginDecoding(cake, &decodeOptions) != 0);

	if (canPlay)
	{
		CreateRenderingConstants(cake);
	}

	lwmUInt32 numFrames = 0;
	lwmUInt64 qpcDuration = 0;
	while (canPlay)
	{
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		lwmCakeDecodeOutput decodeOutput;
		LARGE_INTEGER qpcStart;
		LARGE_INTEGER qpcEnd;
		QueryPerformanceCounter(&qpcStart);
		lwmECakeResult cakeResult = lwmCake_Decode(cake, &decodeOutput);
		QueryPerformanceCounter(&qpcEnd);
		qpcDuration += static_cast<lwmUInt64>(qpcEnd.QuadPart - qpcStart.QuadPart);

		switch (cakeResult)
		{
		case lwmCAKE_RESULT_Error:
		case lwmCAKE_RESULT_Finished:
			canPlay = false;
			break;
		case lwmCAKE_RESULT_Waiting:
			if (decodeOutput.displayDelay > 0)
				Sleep(static_cast<DWORD>(decodeOutput.displayDelay));
			break;
		case lwmCAKE_RESULT_NewVideoFrame:
			numFrames++;
			DisplayCakeFrame(cake, decodeOutput.workFrameIndex, movieInfo.videoWidth, movieInfo.videoHeight, movieInfo.videoFrameFormat == lwmFRAMEFORMAT_8Bit_420P_Planar);
			break;
		}
	}

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	double milliseconds = static_cast<double>(qpcDuration)* 1000.0 / static_cast<double>(frequency.QuadPart) / static_cast<double>(numFrames);
	printf("MS per frame: %f", milliseconds);


	lwmCake_Destroy(cake);

	return 0;
}

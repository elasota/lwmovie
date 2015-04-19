#define _CRT_SECURE_NO_WARNINGS	// Shut up

#include <d3dcompiler.h>
#include <d3d11.h>
#include <stdio.h>

void CompileShaderFile(const wchar_t *hlslFileName, const char *entryPoint, const char *target, const char *outFileName)
{
	ID3DBlob *compiledShader = NULL;
	ID3DBlob *errors = NULL;

	HRESULT result = D3DCompileFromFile(hlslFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &compiledShader, &errors);

	if (errors)
	{
		fwrite(errors->GetBufferPointer(), 1, errors->GetBufferSize(), stderr);
		errors->Release();
	}

	if (compiledShader)
	{
		ID3DBlob *strippedBlob = NULL;
		UINT stripFlags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA;
		D3DStripShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), stripFlags, &strippedBlob);

		FILE *outFile = fopen(outFileName, "wb");
		if (outFile)
		{
			const unsigned char *shaderBytes = static_cast<const unsigned char*>(compiledShader->GetBufferPointer());
			size_t shaderSize = compiledShader->GetBufferSize();
			for (size_t i = 0; i < shaderSize; i++)
			{
				if (i % 32 == 0 && i != 0)
					fputs("\n", outFile);
				fprintf(outFile, "%i, ", static_cast<int>(shaderBytes[i]));
			}
			fclose(outFile);
		}
		else
			fprintf(stderr, "Could not open output file %s", outFileName);
		compiledShader->Release();
		strippedBlob->Release();
	}
}

int main(int argc, const char **argv)
{
	CompileShaderFile(L"lwmovie/d3d11/shaders/idct_cs.hlsl", "mainCS", "cs_4_0", "lwmovie/d3d11/shaders/idct_cs.compiled.h");
	CompileShaderFile(L"lwmovie/d3d11/shaders/recon_vs.hlsl", "mainVS", "vs_4_0", "lwmovie/d3d11/shaders/recon_vs.compiled.h");
	CompileShaderFile(L"lwmovie/d3d11/shaders/recon_ps_luma.hlsl", "mainPS", "ps_4_0", "lwmovie/d3d11/shaders/recon_ps_luma.compiled.h");
	CompileShaderFile(L"lwmovie/d3d11/shaders/recon_ps_chroma.hlsl", "mainPS", "ps_4_0", "lwmovie/d3d11/shaders/recon_ps_chroma.compiled.h");
	return 0;
}

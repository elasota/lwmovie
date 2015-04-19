#define DISPLAY_SHADER_SHARED_DEFS	\
"struct PS_INPUT\n"\
"{\n"\
"    float4 pos : SV_POSITION;\n"\
"    float2 texCoord : TEXCOORD0;\n"\
"};\n"\
"struct PSConstants\n"\
"{\n"\
"    float4 ycbcrWeights1;\n"\
"    float4 ycbcrWeights2;\n"\
"};\n"\
""

static const char *displayVS =
DISPLAY_SHADER_SHARED_DEFS
"PS_INPUT mainVS( float4 pos : POSITION, float2 texCoord : TEXCOORD0 )\n"
"{\n"
"    PS_INPUT psInput;\n"
"    psInput.pos = pos;\n"
"    psInput.texCoord = texCoord;\n"
"    return psInput;\n"
"}\n"
;

static const char *displayPS =
DISPLAY_SHADER_SHARED_DEFS
"Texture2D myTextureY : register(t0);\n"
"Texture2D myTextureCb : register(t1);\n"
"Texture2D myTextureCr : register(t2);\n"
"PSConstants myConstants : register(b0);\n"
"SamplerState mySampler : register(s0);\n"
"float4 mainPS( PS_INPUT psInput ) : SV_Target\n"
"{\n"
"    float3 ycbcr;\n"
"    float y = myTextureY.Sample(mySampler, psInput.texCoord.xy).r;\n"
"    float cb = myTextureCb.Sample(mySampler, psInput.texCoord.xy).r;\n"
"    float cr = myTextureCr.Sample(mySampler, psInput.texCoord.xy).r;\n"
"    float3 rgb = y * myConstants.ycbcrWeights1.www + myConstants.ycbcrWeights1.xyz;\n"
"    rgb.r += cr * myConstants.ycbcrWeights2.x;\n"
"    rgb.g += cr * myConstants.ycbcrWeights2.y + cb * myConstants.ycbcrWeights2.z;\n"
"    rgb.b += cb * myConstants.ycbcrWeights2.w;\n"
"    //return float4(y, cb, cr, 1.0);\n"
"    return float4(rgb, 1.0);\n"
"}\n"
;

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
#include "lwmovie.h"
#include "lwmovie_recon.hpp"
#include "lwmovie_simd_defs.hpp"
#include "../common/lwmovie_atomicint_type.hpp"
#include "../common/lwmovie_atomicint_funcs.hpp"


namespace lwmovie
{
	namespace pixelconv
	{
		// Channel packer
		template<int TFinalLayout>
		struct CRGBChannelPacker
		{
		};

		template<>
		struct CRGBChannelPacker<lwmVIDEOCHANNELLAYOUT_ARGB>
		{
			static LWMOVIE_FORCEINLINE void Pack(lwmUInt8 r, lwmUInt8 g, lwmUInt8 b, lwmUInt8 *outInterleaved)
			{
				outInterleaved[0] = 255; outInterleaved[1] = r; outInterleaved[2] = g; outInterleaved[3] = b;
			}
		};

		template<>
		struct CRGBChannelPacker<lwmVIDEOCHANNELLAYOUT_ABGR>
		{
			static LWMOVIE_FORCEINLINE void Pack(lwmUInt8 r, lwmUInt8 g, lwmUInt8 b, lwmUInt8 *outInterleaved)
			{
				outInterleaved[0] = 255; outInterleaved[1] = b; outInterleaved[2] = g; outInterleaved[3] = r;
			}
		};

		template<>
		struct CRGBChannelPacker<lwmVIDEOCHANNELLAYOUT_RGBA>
		{
			static LWMOVIE_FORCEINLINE void Pack(lwmUInt8 r, lwmUInt8 g, lwmUInt8 b, lwmUInt8 *outInterleaved)
			{
				outInterleaved[0] = r; outInterleaved[1] = g; outInterleaved[2] = b; outInterleaved[3] = 255;
			}
		};

		template<>
		struct CRGBChannelPacker<lwmVIDEOCHANNELLAYOUT_BGRA>
		{
			static LWMOVIE_FORCEINLINE void Pack(lwmUInt8 r, lwmUInt8 g, lwmUInt8 b, lwmUInt8 *outInterleaved)
			{
				outInterleaved[0] = b; outInterleaved[1] = g; outInterleaved[2] = r; outInterleaved[3] = 255;
			}
		};

		// Colorspace converters
		template<int TChannelLayout>
		struct CYUVToRGBConverter
		{
		};
		
		template<>
		struct CYUVToRGBConverter<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601>
		{
			static LWMOVIE_FORCEINLINE void Convert(lwmUInt8 y, lwmUInt8 u, lwmUInt8 v, lwmUInt8 &outR, lwmUInt8 &outG, lwmUInt8 &outB)
			{
				lwmFastSInt16 ytoall = (static_cast<lwmSInt32>(y << 7) * 9539) >> 16;

				lwmFastSInt16 vtor = (static_cast<lwmSInt32>(v << 7) * 13075) >> 16;
				lwmFastSInt16 offsetr = -3566;

				lwmFastSInt16 r = (ytoall + vtor - 3566) >> 4;

				lwmFastSInt16 utog = (static_cast<lwmSInt32>(u << 7) * -3209) >> 16;
				lwmFastSInt16 vtog = (static_cast<lwmSInt32>(v << 7) * -6660) >> 16;

				lwmFastSInt16 g = (ytoall + utog + vtog + 2170) >> 4;
				
				lwmFastSInt16 utob = (static_cast<lwmSInt32>(u << 7) * 16525) >> 16;
				
				lwmFastSInt16 b = (ytoall + utob - 4131) >> 4;

				if(r < 0) r = 0; else if(r > 255) r = 255;
				if(g < 0) g = 0; else if(g > 255) g = 255;
				if(b < 0) b = 0; else if(b > 255) b = 255;
				outR = r;
				outG = g;
				outB = b;
			}
		};

		template<>
		struct CYUVToRGBConverter<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG>
		{
			static LWMOVIE_FORCEINLINE void Convert(lwmUInt8 y, lwmUInt8 u, lwmUInt8 v, lwmUInt8 &outR, lwmUInt8 &outG, lwmUInt8 &outB)
			{
				lwmFastSInt16 ytoall = static_cast<lwmFastSInt16>(y << 4);

				lwmFastSInt16 vtor = (static_cast<lwmSInt32>(v << 7) * 11485) >> 16;
				lwmFastSInt16 r = (ytoall + vtor - 2871) >> 4;

				lwmFastSInt16 utog = (static_cast<lwmSInt32>(u << 7) * -2819) >> 16;
				lwmFastSInt16 vtog = (static_cast<lwmSInt32>(v << 7) * -5850) >> 16;

				lwmFastSInt16 g = (ytoall + utog + vtog + 2166) >> 4;
				
				lwmFastSInt16 utob = (static_cast<lwmSInt32>(u << 7) * 14516) >> 16;
				
				lwmFastSInt16 b = (ytoall + utob - 3629) >> 4;

				if(r < 0) r = 0; else if(r > 255) r = 255;
				if(g < 0) g = 0; else if(g > 255) g = 255;
				if(b < 0) b = 0; else if(b > 255) b = 255;
				outR = r;
				outG = g;
				outB = b;
			}
		};

		// Primary interleaver
		struct CInterleavingDoubler
		{
			static void VerticalMixPrevOnly(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyPrevRow, lwmLargeUInt width)
			{
				for(lwmLargeUInt i=0;i<width;i++)
				{
					lwmUInt8 prev = prevRow[i];
					lwmUInt8 next = nextRow[i];
					mostlyPrevRow[i] = (prev * 3 + next) >> 2;
				}
			}

			static void VerticalMixNextOnly(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyNextRow, lwmLargeUInt width)
			{
				for(lwmLargeUInt i=0;i<width;i++)
				{
					lwmUInt8 prev = prevRow[i];
					lwmUInt8 next = nextRow[i];
					mostlyNextRow[i] = (prev + next * 3) >> 2;
				}
			}

			static void VerticalMixLines(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyPrevRow, lwmUInt8 *mostlyNextRow, lwmLargeUInt width)
			{
				for(lwmLargeUInt i=0;i<width;i++)
				{
					lwmUInt8 prev = prevRow[i];
					lwmUInt8 next = nextRow[i];
					mostlyNextRow[i] = (prev + next * 3) >> 2;
					mostlyPrevRow[i] = (prev * 3 + next) >> 2;
				}
			}
			
			static void HorizInterleaveChromaRowFlagged(const lwmUInt8 *inRowBytes, lwmUInt8 *outBytes, lwmLargeUInt width, bool leftOpen, bool rightOpen, bool highQuality)
			{
				if(width == 0)
					return;

				if(highQuality)
				{
					lwmUInt8 next = inRowBytes[0];
					if(leftOpen)
					{
						lwmUInt8 prev = inRowBytes[-1];
						outBytes[0] = (prev + next * 3) >> 2;
					}
					else
						outBytes[0] = next;

					outBytes++;
					inRowBytes++;
					for(lwmLargeUInt i=1;i<width-1;i+=2)
					{
						lwmUInt8 prev = next;
						next = *inRowBytes++;
						outBytes[0] = (prev * 3 + next) >> 2;
						outBytes[1] = (prev + next * 3) >> 2;
						outBytes += 2;
					}
					if(rightOpen && width % 2 == 0)
					{
						lwmUInt8 prev = next;
						next = *inRowBytes++;
						outBytes[0] = (prev * 3 + next) >> 2;
					}
					else
						outBytes[0] = next;
				}
				else
				{
					for(lwmLargeUInt i=0;i<width-1;i+=2)
					{
						lwmUInt8 b = *inRowBytes++;
						outBytes[i] = b;
						outBytes[i+1] = b;
					}
				}
			}

			static LWMOVIE_FORCEINLINE void HorizInterleaveChromaRow(const lwmUInt8 *inRowBytes, lwmUInt8 *outBytes, lwmLargeUInt width, bool highQuality)
			{
				HorizInterleaveChromaRowFlagged(inRowBytes, outBytes, width, false, false, highQuality);
			}
		};

		// Main entry point
		template<int TOriginalLayout, int TFinalLayout, int TInterleavedUnitSize>
		void CTransformColors(const lwmUInt8 *yPlane, const lwmUInt8 *uPlane, const lwmUInt8 *vPlane, lwmUInt8 *outInterleaved, lwmLargeUInt outWidth)
		{
			for(lwmLargeUInt px=0;px<outWidth;px++)
			{
				lwmUInt8 y = yPlane[px];
				lwmUInt8 u = uPlane[px];
				lwmUInt8 v = vPlane[px];
				lwmUInt8 r, g, b;
				CYUVToRGBConverter<TOriginalLayout>::Convert(y, u, v, r, g, b);

				CRGBChannelPacker<TFinalLayout>::Pack(r, g, b, outInterleaved);

				outInterleaved += TInterleavedUnitSize;
			}
		}
	}
}

#ifdef LWMOVIE_SSE2

#include <emmintrin.h>

namespace lwmovie
{
	namespace pixelconv
	{
		template<int TIntermediateLayout, int TFinalLayout>
		struct SSE2RGBChannelPacker
		{
		};
		
		template<>
		struct SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA>
		{
			static LWMOVIE_FORCEINLINE void Pack(const __m128i &rgba0, const __m128i &rgba1, const __m128i &rgba2, const __m128i &rgba3, lwmUInt8 *outInterleaved)
			{
				__m128i rgba0temp = rgba0;
				__m128i rgba1temp = rgba1;
				__m128i rgba2temp = rgba2;
				__m128i rgba3temp = rgba3;
				_mm_storeu_si128(reinterpret_cast<__m128i*>(outInterleaved   ), rgba0temp);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(outInterleaved+16), rgba1temp);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(outInterleaved+32), rgba2temp);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(outInterleaved+48), rgba3temp);
			}
		};

		template<>
		struct SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_BGRA, lwmVIDEOCHANNELLAYOUT_BGRA>
		{
			static LWMOVIE_FORCEINLINE void Pack(const __m128i &rgba0, const __m128i &rgba1, const __m128i &rgba2, const __m128i &rgba3, lwmUInt8 *outInterleaved)
			{
				SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA>::Pack(rgba0, rgba1, rgba2, rgba3, outInterleaved);
			}
		};

		template<>
		struct SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_ARGB, lwmVIDEOCHANNELLAYOUT_ARGB>
		{
			static LWMOVIE_FORCEINLINE void Pack(const __m128i &rgba0, const __m128i &rgba1, const __m128i &rgba2, const __m128i &rgba3, lwmUInt8 *outInterleaved)
			{
				SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA>::Pack(rgba0, rgba1, rgba2, rgba3, outInterleaved);
			}
		};

		template<>
		struct SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_ABGR, lwmVIDEOCHANNELLAYOUT_ABGR>
		{
			static LWMOVIE_FORCEINLINE void Pack(const __m128i &rgba0, const __m128i &rgba1, const __m128i &rgba2, const __m128i &rgba3, lwmUInt8 *outInterleaved)
			{
				SSE2RGBChannelPacker<lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA>::Pack(rgba0, rgba1, rgba2, rgba3, outInterleaved);
			}
		};

		template<int TFinalChannelLayout>
		struct SSE2IntermediateLayoutAlphaResolver
		{
		};

		template<> struct SSE2IntermediateLayoutAlphaResolver<lwmVIDEOCHANNELLAYOUT_ARGB> { static LWMOVIE_FORCEINLINE __m128i Generate() { return _mm_set1_epi8(-1); } };
		template<> struct SSE2IntermediateLayoutAlphaResolver<lwmVIDEOCHANNELLAYOUT_ABGR> { static LWMOVIE_FORCEINLINE __m128i Generate() { return _mm_set1_epi8(-1); } };
		template<> struct SSE2IntermediateLayoutAlphaResolver<lwmVIDEOCHANNELLAYOUT_RGBA> { static LWMOVIE_FORCEINLINE __m128i Generate() { return _mm_set1_epi8(-1); } };
		template<> struct SSE2IntermediateLayoutAlphaResolver<lwmVIDEOCHANNELLAYOUT_BGRA> { static LWMOVIE_FORCEINLINE __m128i Generate() { return _mm_set1_epi8(-1); } };

		struct SSE2ChannelInterleaver
		{
			static LWMOVIE_FORCEINLINE void Interleave(const __m128i &ch0, const __m128i &ch1, const __m128i &ch2, const __m128i &ch3, __m128i &interleaved0, __m128i &interleaved1, __m128i &interleaved2, __m128i &interleaved3)
			{
				__m128i left01 = _mm_unpacklo_epi8(ch0, ch1);
				__m128i right01 = _mm_unpackhi_epi8(ch0, ch1);
				__m128i left23 = _mm_unpacklo_epi8(ch2, ch3);
				__m128i right23 = _mm_unpackhi_epi8(ch2, ch3);

				interleaved0 = _mm_unpacklo_epi16(left01, left23);
				interleaved1 = _mm_unpackhi_epi16(left01, left23);
				interleaved2 = _mm_unpacklo_epi16(right01, right23);
				interleaved3 = _mm_unpackhi_epi16(right01, right23);
			}
		};

		template<int TChannelLayout>
		struct SSE2RGBAInterleaver
		{
		};

		template<>
		struct SSE2RGBAInterleaver<lwmVIDEOCHANNELLAYOUT_RGBA>
		{
			static LWMOVIE_FORCEINLINE void Interleave(const __m128i &r, const __m128i &g, const __m128i &b, const __m128i &a, __m128i &interleaved0, __m128i &interleaved1, __m128i &interleaved2, __m128i &interleaved3)
			{
				SSE2ChannelInterleaver::Interleave(r, g, b, a, interleaved0, interleaved1, interleaved2, interleaved3);
			}
		};

		template<>
		struct SSE2RGBAInterleaver<lwmVIDEOCHANNELLAYOUT_BGRA>
		{
			static LWMOVIE_FORCEINLINE void Interleave(const __m128i &r, const __m128i &g, const __m128i &b, const __m128i &a, __m128i &interleaved0, __m128i &interleaved1, __m128i &interleaved2, __m128i &interleaved3)
			{
				SSE2ChannelInterleaver::Interleave(b, g, r, a, interleaved0, interleaved1, interleaved2, interleaved3);
			}
		};

		template<>
		struct SSE2RGBAInterleaver<lwmVIDEOCHANNELLAYOUT_ARGB>
		{
			static LWMOVIE_FORCEINLINE void Interleave(const __m128i &r, const __m128i &g, const __m128i &b, const __m128i &a, __m128i &interleaved0, __m128i &interleaved1, __m128i &interleaved2, __m128i &interleaved3)
			{
				SSE2ChannelInterleaver::Interleave(a, r, g, b, interleaved0, interleaved1, interleaved2, interleaved3);
			}
		};

		template<>
		struct SSE2RGBAInterleaver<lwmVIDEOCHANNELLAYOUT_ABGR>
		{
			static LWMOVIE_FORCEINLINE void Interleave(const __m128i &r, const __m128i &g, const __m128i &b, const __m128i &a, __m128i &interleaved0, __m128i &interleaved1, __m128i &interleaved2, __m128i &interleaved3)
			{
				SSE2ChannelInterleaver::Interleave(a, r, g, b, interleaved0, interleaved1, interleaved2, interleaved3);
			}
		};

		template<int TChannelLayout>
		struct SSE2YUVToRGBConverter
		{
		};

		template<>
		struct SSE2YUVToRGBConverter<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601>
		{
			static LWMOVIE_FORCEINLINE void Convert(const __m128i &y, const __m128i &u, const __m128i &v, __m128i &outR, __m128i &outG, __m128i &outB)
			{
				__m128i zero = _mm_setzero_si128();
				__m128i yleft = _mm_slli_epi16(_mm_unpacklo_epi8(y, zero), 7);
				__m128i yright = _mm_slli_epi16(_mm_unpackhi_epi8(y, zero), 7);
				__m128i uleft = _mm_slli_epi16(_mm_unpacklo_epi8(u, zero), 7);
				__m128i uright = _mm_slli_epi16(_mm_unpackhi_epi8(u, zero), 7);
				__m128i vleft = _mm_slli_epi16(_mm_unpacklo_epi8(v, zero), 7);
				__m128i vright = _mm_slli_epi16(_mm_unpackhi_epi8(v, zero), 7);

				__m128i ytoall = _mm_set1_epi16(9539);

				__m128i ytoallleft = _mm_mulhi_epi16(yleft, ytoall);
				__m128i ytoallright = _mm_mulhi_epi16(yright, ytoall);

				__m128i vtor = _mm_set1_epi16(13075);
				__m128i vtorleft = _mm_mulhi_epi16(vleft, vtor);
				__m128i vtorright = _mm_mulhi_epi16(vright, vtor);

				__m128i offsetr = _mm_set1_epi16(-3566);

				__m128i rleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, vtorleft), offsetr), 4);
				__m128i rright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, vtorright), offsetr), 4);

				__m128i utog = _mm_set1_epi16(-3209);
				__m128i utogleft = _mm_mulhi_epi16(uleft, utog);
				__m128i utogright = _mm_mulhi_epi16(uright, utog);

				__m128i vtog = _mm_set1_epi16(-6660);
				__m128i vtogleft = _mm_mulhi_epi16(vleft, vtog);
				__m128i vtogright = _mm_mulhi_epi16(vright, vtog);

				__m128i offsetg = _mm_set1_epi16(2170);
			
				__m128i gleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, utogleft), vtogleft), offsetg), 4);
				__m128i gright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, utogright), vtogright), offsetg), 4);

				__m128i utob = _mm_set1_epi16(16525);
				__m128i utobleft = _mm_mulhi_epi16(uleft, utob);
				__m128i utobright = _mm_mulhi_epi16(uright, utob);
			
				__m128i offsetb = _mm_set1_epi16(-4131);
			
				__m128i bleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, utobleft), offsetb), 4);
				__m128i bright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, utobright), offsetb), 4);

				__m128i r = _mm_packus_epi16(rleft, rright);
				__m128i g = _mm_packus_epi16(gleft, gright);
				__m128i b = _mm_packus_epi16(bleft, bright);

				outR = r;
				outG = g;
				outB = b;
			}
		};

		template<>
		struct SSE2YUVToRGBConverter<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG>
		{
			static LWMOVIE_FORCEINLINE void Convert(const __m128i &y, const __m128i &u, const __m128i &v, __m128i &outR, __m128i &outG, __m128i &outB)
			{
				__m128i zero = _mm_setzero_si128();
				__m128i ytoallleft = _mm_slli_epi16(_mm_unpacklo_epi8(y, zero), 4);
				__m128i ytoallright = _mm_slli_epi16(_mm_unpackhi_epi8(y, zero), 4);
				__m128i uleft = _mm_slli_epi16(_mm_unpacklo_epi8(u, zero), 7);
				__m128i uright = _mm_slli_epi16(_mm_unpackhi_epi8(u, zero), 7);
				__m128i vleft = _mm_slli_epi16(_mm_unpacklo_epi8(v, zero), 7);
				__m128i vright = _mm_slli_epi16(_mm_unpackhi_epi8(v, zero), 7);

				__m128i vtor = _mm_set1_epi16(11485);
				__m128i vtorleft = _mm_mulhi_epi16(vleft, vtor);
				__m128i vtorright = _mm_mulhi_epi16(vright, vtor);

				__m128i offsetr = _mm_set1_epi16(-2871);

				__m128i rleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, vtorleft), offsetr), 4);
				__m128i rright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, vtorright), offsetr), 4);

				__m128i utog = _mm_set1_epi16(-2819);
				__m128i utogleft = _mm_mulhi_epi16(uleft, utog);
				__m128i utogright = _mm_mulhi_epi16(uright, utog);

				__m128i vtog = _mm_set1_epi16(-5850);
				__m128i vtogleft = _mm_mulhi_epi16(vleft, vtog);
				__m128i vtogright = _mm_mulhi_epi16(vright, vtog);

				__m128i offsetg = _mm_set1_epi16(2166);
			
				__m128i gleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, utogleft), vtogleft), offsetg), 4);
				__m128i gright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, utogright), vtogright), offsetg), 4);

				__m128i utob = _mm_set1_epi16(14516);
				__m128i utobleft = _mm_mulhi_epi16(uleft, utob);
				__m128i utobright = _mm_mulhi_epi16(uright, utob);
			
				__m128i offsetb = _mm_set1_epi16(-3629);
			
				__m128i bleft = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallleft, utobleft), offsetb), 4);
				__m128i bright = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(ytoallright, utobright), offsetb), 4);

				__m128i r = _mm_packus_epi16(rleft, rright);
				__m128i g = _mm_packus_epi16(gleft, gright);
				__m128i b = _mm_packus_epi16(bleft, bright);

				outR = r;
				outG = g;
				outB = b;
			}
		};

		// Interleaver
		struct SSE2InterpolatingDoubler
		{
			static void VerticalMixPrevOnly(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyPrevRow, lwmLargeUInt width)
			{
				lwmLargeUInt numUnits = width / 16;
				__m128i zero = _mm_setzero_si128();
				for(lwmLargeUInt i=0;i<numUnits;i++)
				{
					__m128i prev = _mm_load_si128(reinterpret_cast<const __m128i*>(prevRow));
					__m128i next = _mm_load_si128(reinterpret_cast<const __m128i*>(nextRow));

					__m128i prevLeft = _mm_unpacklo_epi8(prev, zero);
					__m128i prevRight = _mm_unpackhi_epi8(prev, zero);
					__m128i nextLeft = _mm_unpacklo_epi8(next, zero);
					__m128i nextRight = _mm_unpackhi_epi8(next, zero);

					__m128i mostlyPrevLeft = _mm_srli_epi16(_mm_add_epi16(nextLeft, _mm_add_epi16(prevLeft, _mm_slli_epi16(prevLeft, 1))), 2);
					__m128i mostlyPrevRight = _mm_srli_epi16(_mm_add_epi16(nextRight, _mm_add_epi16(prevRight, _mm_slli_epi16(prevRight, 1))), 2);

					__m128i mostlyPrev = _mm_packus_epi16(mostlyPrevLeft, mostlyPrevRight);

					_mm_store_si128(reinterpret_cast<__m128i*>(mostlyPrevRow), mostlyPrev);

					prevRow += 16;
					nextRow += 16;
					mostlyPrevRow += 16;
				}
			}

			static void VerticalMixNextOnly(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyNextRow, lwmLargeUInt width)
			{
				lwmLargeUInt numUnits = width / 16;
				__m128i zero = _mm_setzero_si128();
				for(lwmLargeUInt i=0;i<numUnits;i++)
				{
					__m128i prev = _mm_load_si128(reinterpret_cast<const __m128i*>(prevRow));
					__m128i next = _mm_load_si128(reinterpret_cast<const __m128i*>(nextRow));

					__m128i prevLeft = _mm_unpacklo_epi8(prev, zero);
					__m128i prevRight = _mm_unpackhi_epi8(prev, zero);
					__m128i nextLeft = _mm_unpacklo_epi8(next, zero);
					__m128i nextRight = _mm_unpackhi_epi8(next, zero);

					__m128i mostlyNextLeft = _mm_srli_epi16(_mm_add_epi16(prevLeft, _mm_add_epi16(nextLeft, _mm_slli_epi16(nextLeft, 1))), 2);
					__m128i mostlyNextRight = _mm_srli_epi16(_mm_add_epi16(prevRight, _mm_add_epi16(nextRight, _mm_slli_epi16(nextRight, 1))), 2);

					__m128i mostlyNext = _mm_packus_epi16(mostlyNextLeft, mostlyNextRight);

					_mm_store_si128(reinterpret_cast<__m128i*>(mostlyNextRow), mostlyNext);

					prevRow += 16;
					nextRow += 16;
					mostlyNextRow += 16;
				}
			}

			static void VerticalMixLines(const lwmUInt8 *prevRow, const lwmUInt8 *nextRow, lwmUInt8 *mostlyPrevRow, lwmUInt8 *mostlyNextRow, lwmLargeUInt width)
			{
				lwmLargeUInt numUnits = width / 16;
				__m128i zero = _mm_setzero_si128();
				for(lwmLargeUInt i=0;i<numUnits;i++)
				{
					__m128i prev = _mm_load_si128(reinterpret_cast<const __m128i*>(prevRow));
					__m128i next = _mm_load_si128(reinterpret_cast<const __m128i*>(nextRow));

					__m128i prevLeft = _mm_unpacklo_epi8(prev, zero);
					__m128i prevRight = _mm_unpackhi_epi8(prev, zero);
					__m128i nextLeft = _mm_unpacklo_epi8(next, zero);
					__m128i nextRight = _mm_unpackhi_epi8(next, zero);

					__m128i mostlyNextLeft = _mm_srli_epi16(_mm_add_epi16(prevLeft, _mm_add_epi16(nextLeft, _mm_slli_epi16(nextLeft, 1))), 2);
					__m128i mostlyPrevLeft = _mm_srli_epi16(_mm_add_epi16(nextLeft, _mm_add_epi16(prevLeft, _mm_slli_epi16(prevLeft, 1))), 2);
					__m128i mostlyNextRight = _mm_srli_epi16(_mm_add_epi16(prevRight, _mm_add_epi16(nextRight, _mm_slli_epi16(nextRight, 1))), 2);
					__m128i mostlyPrevRight = _mm_srli_epi16(_mm_add_epi16(nextRight, _mm_add_epi16(prevRight, _mm_slli_epi16(prevRight, 1))), 2);

					__m128i mostlyNext = _mm_packus_epi16(mostlyNextLeft, mostlyNextRight);
					__m128i mostlyPrev = _mm_packus_epi16(mostlyPrevLeft, mostlyPrevRight);

					_mm_store_si128(reinterpret_cast<__m128i*>(mostlyPrevRow), mostlyPrev);
					_mm_store_si128(reinterpret_cast<__m128i*>(mostlyNextRow), mostlyNext);

					prevRow += 16;
					nextRow += 16;
					mostlyPrevRow += 16;
					mostlyNextRow += 16;
				}
			}

			static void HorizInterleaveChromaRow(const lwmUInt8 *inRowBytes, lwmUInt8 *outBytes, lwmLargeUInt width, bool highQuality)
			{
				if(width == 0)
					return;

				lwmLargeUInt numUnits;
				if(highQuality)
				{
					numUnits = (width - 1) / 32;

					if(numUnits > 0)
					{
						__m128i zero = _mm_setzero_si128();

						lwmSInt16 prevLast;
						prevLast = inRowBytes[0];

						for(lwmLargeUInt i=0;i<numUnits;i++)
						{
							__m128i currentBlock;

							lwmSInt16 nextFirst;
							currentBlock = _mm_load_si128(reinterpret_cast<const __m128i*>(inRowBytes));
							nextFirst = inRowBytes[16];

							__m128i influencesOffsetLeft = _mm_or_si128(_mm_slli_si128(currentBlock, 1), _mm_insert_epi16(zero, prevLast, 0));
							__m128i influencesOffsetRight = _mm_or_si128(_mm_srli_si128(currentBlock, 1), _mm_insert_epi16(zero, nextFirst << 8, 7));

							prevLast = static_cast<lwmSInt16>(_mm_extract_epi16(currentBlock, 7) >> 8);

							__m128i weakInfluencesLeft = _mm_unpacklo_epi8(influencesOffsetLeft, influencesOffsetRight);
							__m128i strongInfluencesLeft = _mm_unpacklo_epi8(currentBlock, currentBlock);
							__m128i weakInfluences0 = _mm_unpacklo_epi8(weakInfluencesLeft, zero);
							__m128i strongInfluences0 = _mm_unpacklo_epi8(strongInfluencesLeft, zero);
							__m128i weakInfluences1 = _mm_unpackhi_epi8(weakInfluencesLeft, zero);
							__m128i strongInfluences1 = _mm_unpackhi_epi8(strongInfluencesLeft, zero);

							__m128i weakInfluencesRight = _mm_unpackhi_epi8(influencesOffsetLeft, influencesOffsetRight);
							__m128i strongInfluencesRight = _mm_unpackhi_epi8(currentBlock, currentBlock);
							__m128i weakInfluences2 = _mm_unpacklo_epi8(weakInfluencesRight, zero);
							__m128i strongInfluences2 = _mm_unpacklo_epi8(strongInfluencesRight, zero);
							__m128i weakInfluences3 = _mm_unpackhi_epi8(weakInfluencesRight, zero);
							__m128i strongInfluences3 = _mm_unpackhi_epi8(strongInfluencesRight, zero);

							__m128i merged0 = _mm_srli_epi16(_mm_add_epi16(weakInfluences0, _mm_add_epi16(strongInfluences0, _mm_slli_epi16(strongInfluences0, 1))), 2);
							__m128i merged1 = _mm_srli_epi16(_mm_add_epi16(weakInfluences1, _mm_add_epi16(strongInfluences1, _mm_slli_epi16(strongInfluences1, 1))), 2);
							__m128i merged2 = _mm_srli_epi16(_mm_add_epi16(weakInfluences2, _mm_add_epi16(strongInfluences2, _mm_slli_epi16(strongInfluences2, 1))), 2);
							__m128i merged3 = _mm_srli_epi16(_mm_add_epi16(weakInfluences3, _mm_add_epi16(strongInfluences3, _mm_slli_epi16(strongInfluences3, 1))), 2);

							__m128i mergedLeft = _mm_packus_epi16(merged0, merged1);
							__m128i mergedRight = _mm_packus_epi16(merged2, merged3);
							_mm_store_si128(reinterpret_cast<__m128i*>(outBytes), mergedLeft);
							_mm_store_si128(reinterpret_cast<__m128i*>(outBytes + 16), mergedRight);
							inRowBytes += 16;
							outBytes += 32;
						}
					}
				}
				else
				{
					numUnits = width / 32;

					for(lwmLargeUInt i=0;i<numUnits;i++)
					{
						__m128i currentBlock;

						currentBlock = _mm_load_si128(reinterpret_cast<const __m128i*>(inRowBytes));
						__m128i left = _mm_unpacklo_epi8(currentBlock, currentBlock);
						__m128i right = _mm_unpackhi_epi8(currentBlock, currentBlock);

						_mm_store_si128(reinterpret_cast<__m128i*>(outBytes), left);
						_mm_store_si128(reinterpret_cast<__m128i*>(outBytes + 16), right);
						inRowBytes += 16;
						outBytes += 32;
					}
				}

				CInterleavingDoubler::HorizInterleaveChromaRowFlagged(inRowBytes, outBytes, width - (32 * numUnits), numUnits != 0, false, highQuality);
			}
		};

		template<int TOriginalLayout, int TIntermediateLayout, int TFinalLayout, int TInterleavedUnitSize>
		void SSE2TransformColors(const lwmUInt8 *yPlane, const lwmUInt8 *uPlane, const lwmUInt8 *vPlane, lwmUInt8 *outInterleaved, lwmLargeUInt outWidth)
		{
			lwmLargeUInt numUnits = outWidth / 16;
			lwmLargeUInt tailWidth = outWidth - (numUnits * 16);
			__m128i alpha = SSE2IntermediateLayoutAlphaResolver<TFinalLayout>::Generate();
			while(numUnits--)
			{
				__m128i y = _mm_load_si128(reinterpret_cast<const __m128i*>(yPlane));
				__m128i u = _mm_load_si128(reinterpret_cast<const __m128i*>(uPlane));
				__m128i v = _mm_load_si128(reinterpret_cast<const __m128i*>(vPlane));
				__m128i r, g, b;
				SSE2YUVToRGBConverter<TOriginalLayout>::Convert(y, u, v, r, g, b);

				__m128i rgba0, rgba1, rgba2, rgba3;
				SSE2RGBAInterleaver<TIntermediateLayout>::Interleave(r, g, b, alpha, rgba0, rgba1, rgba2, rgba3);
				SSE2RGBChannelPacker<TIntermediateLayout, TFinalLayout>::Pack(rgba0, rgba1, rgba2, rgba3, outInterleaved);
					
				yPlane += 16;
				uPlane += 16;
				vPlane += 16;
				outInterleaved += TInterleavedUnitSize;
			}

			CTransformColors<TOriginalLayout, TFinalLayout, TInterleavedUnitSize / 16>(yPlane, uPlane, vPlane, outInterleaved, tailWidth);
		}
	}
}
#endif

namespace lwmovie
{
	namespace pixelconv
	{
		typedef void (*TransformColorsFunc_t)(const lwmUInt8 *yPlane, const lwmUInt8 *uPlane, const lwmUInt8 *vPlane, lwmUInt8 *outInterleaved, lwmLargeUInt width);

		struct PixelConverterEntry
		{
			TransformColorsFunc_t transformColorsFunc;
			int sourceChannelLayout, targetChannelLayout;
		};

#ifdef LWMOVIE_SSE2
		PixelConverterEntry pixelConverters[] =
		{
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ARGB, lwmVIDEOCHANNELLAYOUT_ARGB, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ARGB },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ABGR, lwmVIDEOCHANNELLAYOUT_ABGR, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ABGR },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_RGBA },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_BGRA, lwmVIDEOCHANNELLAYOUT_BGRA, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_BGRA },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ARGB, lwmVIDEOCHANNELLAYOUT_ARGB, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ARGB },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ABGR, lwmVIDEOCHANNELLAYOUT_ABGR, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ABGR },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_RGBA, lwmVIDEOCHANNELLAYOUT_RGBA, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_RGBA },
			{ (SSE2TransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_BGRA, lwmVIDEOCHANNELLAYOUT_BGRA, 64>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_BGRA },
			{ NULL }
		};
#endif
#ifdef LWMOVIE_NOSIMD
		PixelConverterEntry pixelConverters[] =
		{
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ARGB, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ARGB },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ABGR, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_ABGR },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_RGBA, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_RGBA },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_BGRA, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_BT601, lwmVIDEOCHANNELLAYOUT_BGRA },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ARGB, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ARGB },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ABGR, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_ABGR },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_RGBA, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_RGBA },
			{ (CTransformColors<lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_BGRA, 4>), lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG, lwmVIDEOCHANNELLAYOUT_BGRA },
			{ NULL }
		};
#endif
	}
}

#ifdef LWMOVIE_SSE2
typedef lwmovie::pixelconv::SSE2InterpolatingDoubler InterpolatingDoubler_t;
#endif
#ifdef LWMOVIE_NOSIMD
typedef lwmovie::pixelconv::CInterleavingDoubler InterpolatingDoubler_t;
#endif

struct lwmRGBConverterBlock
{
	lwmovie::pixelconv::TransformColorsFunc_t m_conversionFunc;
	bool m_topOpen;
	bool m_bottomOpen;

	lwmUInt8 *m_outInterleaved;
	lwmUInt32 m_outWidth, m_outHeight;
	lwmLargeUInt m_outPitch;

	lwmLargeUInt m_yPitch, m_uPitch, m_vPitch;

	const lwmUInt8 *m_yPlane;
	const lwmUInt8 *m_uPlane;
	const lwmUInt8 *m_vPlane;
	
	lwmUInt32 m_workLineWidth;

	lwmLargeUInt m_convLinePitch;	// Pitch of a work buffer line, including padding for 4-to-3 conversion if necessary
	void *m_convBuffer;

	void Process420(bool highQuality)
	{
		lwmovie::pixelconv::TransformColorsFunc_t convertFunc = this->m_conversionFunc;
		lwmUInt8 *convLines[8];
		convLines[0] = static_cast<lwmUInt8*>(m_convBuffer);

		for(int i=1;i<8;i++)
			convLines[i] = convLines[i-1] + m_convLinePitch;

		lwmUInt8 *prevLines[2];
		lwmUInt8 *nextLines[2];
		lwmUInt8 *mostlyPrevLines[2];
		lwmUInt8 *mostlyNextLines[2];

		prevLines[0] = convLines[0];
		prevLines[1] = convLines[1];
		nextLines[0] = convLines[2];
		nextLines[1] = convLines[3];
		mostlyPrevLines[0] = convLines[4];
		mostlyPrevLines[1] = convLines[5];
		mostlyNextLines[0] = convLines[6];
		mostlyNextLines[1] = convLines[7];

		const lwmUInt8 *yPlane = m_yPlane;
		const lwmUInt8 *uPlane = m_uPlane;
		const lwmUInt8 *vPlane = m_vPlane;

		lwmLargeUInt yPitch = this->m_yPitch;
		lwmLargeUInt uPitch = this->m_uPitch;
		lwmLargeUInt vPitch = this->m_vPitch;

		lwmUInt8 *outInterleaved = m_outInterleaved;
		lwmUInt32 outWidth = m_outWidth;
		lwmUInt32 outHeight = m_outHeight;
		lwmLargeUInt outPitch = this->m_outPitch;

		lwmUInt32 numMixedRows = (m_outWidth - 2) / 2;

		lwmUInt32 workLineWidth = m_workLineWidth;

		// Process first row

		InterpolatingDoubler_t::HorizInterleaveChromaRow(uPlane, nextLines[0], workLineWidth, highQuality);
		InterpolatingDoubler_t::HorizInterleaveChromaRow(vPlane, nextLines[1], workLineWidth, highQuality);

		if(this->m_topOpen && highQuality)
		{
			InterpolatingDoubler_t::HorizInterleaveChromaRow(uPlane - uPitch, prevLines[0], workLineWidth, highQuality);
			InterpolatingDoubler_t::HorizInterleaveChromaRow(vPlane - vPitch, prevLines[1], workLineWidth, highQuality);
			for(int p=0;p<2;p++)
				InterpolatingDoubler_t::VerticalMixPrevOnly(prevLines[p], nextLines[p], mostlyPrevLines[p], workLineWidth);
			convertFunc(yPlane, mostlyPrevLines[0], mostlyPrevLines[1], outInterleaved, outWidth);
		}
		else
			convertFunc(yPlane, nextLines[0], nextLines[1], outInterleaved, outWidth);
		yPlane += yPitch;
		outInterleaved += outPitch;

		// Y plane should be on row 1
		// UV planes should be on row 1 (in subsampled chroma space)
		for(lwmUInt32 row=1;row<outHeight-1;row+=2)
		{
			for(int i=0;i<2;i++)
			{
				lwmUInt8 *temp = prevLines[i];
				prevLines[i] = nextLines[i];
				nextLines[i] = temp;
			}
			uPlane += uPitch;
			vPlane += vPitch;
			InterpolatingDoubler_t::HorizInterleaveChromaRow(uPlane, nextLines[0], workLineWidth, highQuality);
			InterpolatingDoubler_t::HorizInterleaveChromaRow(vPlane, nextLines[1], workLineWidth, highQuality);

			lwmUInt8 *outInterleavedRowMP = outInterleaved;
			outInterleaved += outPitch;
			lwmUInt8 *outInterleavedRowMN = outInterleaved;
			outInterleaved += outPitch;

			if(highQuality)
			{
				for(int p=0;p<2;p++)
					InterpolatingDoubler_t::VerticalMixLines(prevLines[p], nextLines[p], mostlyPrevLines[p], mostlyNextLines[p], workLineWidth);
				convertFunc(yPlane, mostlyPrevLines[0], mostlyPrevLines[1], outInterleavedRowMP, outWidth);
				yPlane += yPitch;
				convertFunc(yPlane, mostlyNextLines[0], mostlyNextLines[1], outInterleavedRowMN, outWidth);
				yPlane += yPitch;
			}
			else
			{
				convertFunc(yPlane, prevLines[0], prevLines[1], outInterleavedRowMP, outWidth);
				yPlane += yPitch;
				convertFunc(yPlane, nextLines[0], nextLines[1], outInterleavedRowMN, outWidth);
				yPlane += yPitch;
			}
		}

		if(outHeight % 2 == 0)
		{
			if(this->m_bottomOpen && highQuality)
			{
				// Prev and next are swapped
				InterpolatingDoubler_t::HorizInterleaveChromaRow(uPlane + uPitch, prevLines[0], workLineWidth, highQuality);
				InterpolatingDoubler_t::HorizInterleaveChromaRow(vPlane + vPitch, prevLines[1], workLineWidth, highQuality);
				for(int p=0;p<2;p++)
					InterpolatingDoubler_t::VerticalMixNextOnly(nextLines[p], prevLines[p], mostlyNextLines[p], workLineWidth);
				convertFunc(yPlane, mostlyNextLines[0], mostlyNextLines[1], outInterleaved, outWidth);
			}
			else
				convertFunc(yPlane, nextLines[0], nextLines[1], outInterleaved, outWidth);
		}
	}

	void Process444()
	{
		lwmovie::pixelconv::TransformColorsFunc_t convertFunc = this->m_conversionFunc;
		lwmUInt8 *outInterleaved = m_outInterleaved;
		lwmUInt32 outWidth = m_outWidth;
		lwmUInt32 outHeight = m_outHeight;
		lwmLargeUInt outPitch = m_outPitch;


		const lwmUInt8 *yPlane = m_yPlane;
		const lwmUInt8 *uPlane = m_uPlane;
		const lwmUInt8 *vPlane = m_vPlane;

		lwmLargeUInt yPitch = m_yPitch;
		lwmLargeUInt uPitch = m_uPitch;
		lwmLargeUInt vPitch = m_vPitch;

		for(lwmUInt32 row=0;row<outHeight;row++)
		{
			convertFunc(yPlane, uPlane, vPlane, outInterleaved, outWidth);
			outInterleaved += outPitch;
			yPlane += yPitch;
			uPlane += uPitch;
			vPlane += vPitch;
		}
	}

	void Process(bool is444, bool highQuality)
	{
		if (is444)
			Process444();
		else
			Process420(highQuality);
	}
};

struct lwmVideoRGBConverter
{
	lwmLargeUInt numBlocks;
	lwmLargeUInt workBlockRowCount;
	lwmIVideoReconstructor *videoRecon;
	lwmRGBConverterBlock *blocks;
	lwmSWorkNotifier *workNotifier;
	lwmEFrameFormat frameFormat;

	lwmSAllocator *alloc;
	void *convBuffer;

	lwmAtomicInt currentRowIndex;
	bool highQuality;
};

LWMOVIE_API_LINK struct lwmVideoRGBConverter *lwmVideoRGBConverter_CreateSliced(struct lwmSAllocator *alloc, struct lwmIVideoReconstructor *recon, lwmEFrameFormat inFrameFormat, lwmEVideoChannelLayout inChannelLayout, lwmUInt32 outWidth, lwmUInt32 outHeight, lwmEVideoChannelLayout outChannelLayout, lwmLargeUInt numSlices)
{
	lwmovie::pixelconv::TransformColorsFunc_t convertFunc = NULL;

	if(inFrameFormat != lwmFRAMEFORMAT_8Bit_420P_Planar && inFrameFormat != lwmFRAMEFORMAT_8Bit_3Channel_Planar)
		return NULL;

	const lwmovie::pixelconv::PixelConverterEntry *pce = lwmovie::pixelconv::pixelConverters;
	while(true)
	{
		if(pce->transformColorsFunc == NULL)
			return NULL;
		if(pce->sourceChannelLayout == inChannelLayout && pce->targetChannelLayout == outChannelLayout)
			break;
		pce++;
	}

	convertFunc = pce->transformColorsFunc;

	lwmUInt32 workFrameIndex = recon->GetWorkFrameIndex();
	lwmSVideoFrameProvider *frameProvider = recon->GetFrameProvider();
	lwmUInt32 yWorkWidth = frameProvider->getWorkFramePlaneWidthFunc(frameProvider, 0);
	lwmUInt32 yWorkHeight = frameProvider->getWorkFramePlaneHeightFunc(frameProvider, 0);

	// Determine how many jobs can actually be allocated
	lwmLargeUInt workBlockRowCount = (outHeight + numSlices - 1) / numSlices;
	if(workBlockRowCount == 0)
		workBlockRowCount = 1;
	if(workBlockRowCount % 2 == 1)
		workBlockRowCount++;

	numSlices = (outHeight + workBlockRowCount - 1) / workBlockRowCount;
	
	lwmVideoRGBConverter *conv = alloc->NAlloc<lwmVideoRGBConverter>(1);
	if(!conv)
		return NULL;

	conv->blocks = alloc->NAlloc<lwmRGBConverterBlock>(numSlices);
	if(!conv->blocks)
	{
		alloc->Free(conv);
		return NULL;
	}

	
	lwmLargeUInt convLinePitch = lwmovie::PadToSIMD(yWorkWidth);
	lwmLargeUInt convBlockSize = convLinePitch * 8;

	lwmUInt8 *convBuffer = alloc->NAlloc<lwmUInt8>(convBlockSize * numSlices);
	if(!convBuffer)
	{
		alloc->Free(conv->blocks);
		alloc->Free(conv);
		return NULL;
	}

	conv->convBuffer = convBuffer;

	for(lwmLargeUInt i=0;i<numSlices;i++)
	{
		lwmLargeUInt firstRow = i * workBlockRowCount;

		lwmRGBConverterBlock *block = conv->blocks + i;
		block->m_conversionFunc = convertFunc;
		block->m_topOpen = (i != 0);
		block->m_outWidth = outWidth;

		if(i == numSlices - 1)
		{
			lwmLargeUInt blockHeight = outHeight - firstRow;
			block->m_bottomOpen = (blockHeight != workBlockRowCount);
			block->m_outHeight = static_cast<lwmUInt32>(blockHeight);
		}
		else
		{
			block->m_outHeight = outHeight;
			block->m_bottomOpen = true;
			block->m_outHeight = workBlockRowCount;
		}

		block->m_workLineWidth = yWorkWidth;
		block->m_convLinePitch = convLinePitch;
		block->m_convBuffer = convBuffer + (convBlockSize * i);
	}
	
	conv->workBlockRowCount = workBlockRowCount;
	conv->numBlocks = numSlices;
	conv->videoRecon = recon;
	conv->alloc = alloc;
	conv->workNotifier = NULL;
	conv->frameFormat = inFrameFormat;

	return conv;
}

LWMOVIE_API_LINK struct lwmVideoRGBConverter *lwmVideoRGBConverter_Create(struct lwmSAllocator *alloc, struct lwmIVideoReconstructor *recon, lwmEFrameFormat inFrameFormat, lwmEVideoChannelLayout inChannelLayout, lwmUInt32 outWidth, lwmUInt32 outHeight, lwmEVideoChannelLayout outChannelLayout)
{
	return lwmVideoRGBConverter_CreateSliced(alloc, recon, inFrameFormat, inChannelLayout, outWidth, outHeight, outChannelLayout, 1);
}

LWMOVIE_API_LINK void lwmVideoRGBConverter_SetWorkNotifier(struct lwmVideoRGBConverter *converter, struct lwmSWorkNotifier *workNotifier)
{
	converter->workNotifier = workNotifier;
}

LWMOVIE_API_LINK void lwmVideoRGBConverter_Convert(struct lwmVideoRGBConverter *converter, void *outPixels, lwmLargeUInt outStride, int conversionFlags)
{
	lwmIVideoReconstructor *recon = converter->videoRecon;
	lwmUInt32 workFrameIndex = recon->GetWorkFrameIndex();
	lwmSVideoFrameProvider *frameProvider = recon->GetFrameProvider();

	frameProvider->lockWorkFrameFunc(frameProvider, workFrameIndex, lwmVIDEOLOCK_Read);
	lwmLargeUInt numParallelJobs = converter->numBlocks;

	lwmUInt32 yPitch, uPitch, vPitch;

	const lwmUInt8 *yPlane = static_cast<const lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, workFrameIndex, 0, &yPitch));
	const lwmUInt8 *uPlane = static_cast<const lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, workFrameIndex, 1, &uPitch));
	const lwmUInt8 *vPlane = static_cast<const lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, workFrameIndex, 2, &vPitch));
		
	lwmLargeUInt workBlockRowCount = converter->workBlockRowCount;

	for(lwmLargeUInt i=0;i<numParallelJobs;i++)
	{
		lwmRGBConverterBlock *block = converter->blocks + i;
		block->m_outInterleaved = reinterpret_cast<lwmUInt8*>(outPixels);
		block->m_outPitch = outStride;

		block->m_yPlane = yPlane + block->m_yPitch * (i * workBlockRowCount);
		block->m_uPlane = uPlane + block->m_uPitch * (i * workBlockRowCount);
		block->m_vPlane = vPlane + block->m_vPitch * (i * workBlockRowCount);
		block->m_yPitch = yPitch;
		block->m_uPitch = uPitch;
		block->m_vPitch = vPitch;
	}

	bool highQuality = ((conversionFlags & lwmPIXELCONVERTFLAG_Interpolate) != 0);

	if(converter->workNotifier)
	{
		lwmSWorkNotifier *workNotifier = converter->workNotifier;
		converter->currentRowIndex = 0;
		converter->highQuality = highQuality;
		
		for(lwmLargeUInt i=0;i<numParallelJobs;i++)
			workNotifier->notifyAvailableFunc(workNotifier);
		workNotifier->joinFunc(workNotifier);
	}
	else
	{
		for(lwmLargeUInt i=0;i<numParallelJobs;i++)
			converter->blocks[i].Process(converter->frameFormat == lwmFRAMEFORMAT_8Bit_3Channel_Planar, highQuality);
	}

	frameProvider->unlockWorkFrameFunc(frameProvider, workFrameIndex);
}

LWMOVIE_API_LINK void lwmVideoRGBConverter_Destroy(struct lwmVideoRGBConverter *converter)
{
	lwmSAllocator *alloc = converter->alloc;
	alloc->Free(converter->blocks);
	alloc->Free(converter->convBuffer);
	alloc->Free(converter);
}

LWMOVIE_API_LINK void lwmVideoRGBConverter_ConvertParticipate(struct lwmVideoRGBConverter *converter)
{
	lwmAtomicInt nextRowIndex = lwmAtomicIncrement(&converter->currentRowIndex);

	converter->blocks[nextRowIndex - 1].Process(converter->frameFormat == lwmFRAMEFORMAT_8Bit_3Channel_Planar, converter->highQuality);
}

#include "../x86_vc/x86int.h"
#include "../dct.h"
#include "../config.h"

#ifdef OC_X86_64_ASM

#include <emmintrin.h>

static void transpose_8x8_sse(ogg_int16_t *_y, const ogg_int16_t *_x) {
	__m128i flipped2x2[8];
	__m128i flipped4x4[8];
	__m128i t0, t1, t2, t3, t4, t5, t6, t7;
	int i, j;

	/* Flip 2x2 blocks */
	for (i = 0; i < 8; i += 2) {
		t0 = _mm_load_si128((const __m128i *)(_x + i * 8 + 0));
		t1 = _mm_load_si128((const __m128i *)(_x + i * 8 + 8));

		/* t2 = 0 8 1 9 2 a 3 b */
		t2 = _mm_unpacklo_epi16(t0, t1);
		/* t3 = 4 c 5 d 6 e 7 f */
		t3 = _mm_unpackhi_epi16(t0, t1);

		/* t4 = 0 8 2 a 1 9 3 b */
		t4 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(3, 1, 2, 0));

		/* t5 = 4 c 6 e 5 d 7 f */
		t5 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(3, 1, 2, 0));

		/* t6 = 0 8 2 a 4 c 6 e */
		t6 = _mm_unpacklo_epi64(t4, t5);

		/* t6 = 0 8 2 a 4 c 6 e */
		t7 = _mm_unpackhi_epi64(t4, t5);

		flipped2x2[i + 0] = t6;
		flipped2x2[i + 1] = t7;
	}

	/* Flip 4x4 block */
	for (i = 0; i < 8; i += 4) {
		for (j = 0; j < 2; j++) {
			t0 = flipped2x2[i + j];
			t1 = flipped2x2[i + j + 2];

			/* t2 = 0 4 1 5 */
			t2 = _mm_unpacklo_epi32(t0, t1);
			/* t3 = 2 6 3 7 */
			t3 = _mm_unpackhi_epi32(t0, t1);

			/* t4 = 0 4 2 6 */
			t4 = _mm_unpacklo_epi64(t2, t3);

			/* t5 = 1 5 3 7 */
			t5 = _mm_unpackhi_epi64(t2, t3);

			flipped4x4[i + j] = t4;
			flipped4x4[i + j + 2] = t5;
		}
	}

	/* Flip 8x8 block */
	for (i = 0; i < 4; i++) {
		t0 = flipped4x4[i];
		t1 = flipped4x4[i + 4];

		/* t2 = 0 2 */
		t2 = _mm_unpacklo_epi64(t0, t1);
		/* t2 = 1 3 */
		t3 = _mm_unpackhi_epi64(t0, t1);

		_mm_store_si128((__m128i*)(_y + i * 8), t2);
		_mm_store_si128((__m128i*)(_y + i * 8 + 32), t3);
	}
}

static void idct8x8_b2_vert_sse(ogg_int16_t *_y, const ogg_int16_t *_x, int _descale) {
	__m128i d4, d5, d6, d7;
	__m128i c0, c1, c2, c3, c4, c5, c6, c7;
	__m128i b0, b1, b2, b3, b4, b5, b6, b7;
	__m128i a0, a1, a2, a3, a4, a5, a6, a7;
	__m128i o0, o1, o2, o3, o4, o5, o6, o7;
	__m128i bias;

	c0 = _mm_load_si128((const __m128i*)(_x + 8 * 0));
	d4 = _mm_load_si128((const __m128i*)(_x + 8 * 1));
	c2 = _mm_load_si128((const __m128i*)(_x + 8 * 2));
	d6 = _mm_load_si128((const __m128i*)(_x + 8 * 3));
	c1 = _mm_load_si128((const __m128i*)(_x + 8 * 4));
	d5 = _mm_load_si128((const __m128i*)(_x + 8 * 5));
	c3 = _mm_load_si128((const __m128i*)(_x + 8 * 6));
	d7 = _mm_load_si128((const __m128i*)(_x + 8 * 7));

	// odd stage 4
	c4 = d4;
	c5 = _mm_add_epi16(d5, d6);
	c7 = _mm_sub_epi16(d5, d6);
	c6 = d7;

	// odd stage 3
	b4 = _mm_add_epi16(c4, c5);
	b5 = _mm_sub_epi16(c4, c5);
	b6 = _mm_add_epi16(c6, c7);
	b7 = _mm_sub_epi16(c6, c7);

	// even stage 3
	b0 = _mm_add_epi16(c0, c1);
	b1 = _mm_sub_epi16(c0, c1);
	b2 = _mm_add_epi16(_mm_add_epi16(c2, _mm_srai_epi16(c2, 2)), _mm_srai_epi16(c3, 1));
	b3 = _mm_sub_epi16(_mm_sub_epi16(_mm_srai_epi16(c2, 1), c3), _mm_srai_epi16(c3, 2));

	// odd stage 2
	a4 = _mm_sub_epi16(_mm_add_epi16(_mm_add_epi16(_mm_srai_epi16(b7, 2), b4), _mm_srai_epi16(b4, 2)), _mm_srai_epi16(b4, 4));
	a5 = _mm_add_epi16(_mm_add_epi16(_mm_sub_epi16(b5, b6), _mm_srai_epi16(b6, 2)), _mm_srai_epi16(b6, 4));
	a6 = _mm_sub_epi16(_mm_sub_epi16(_mm_add_epi16(b6, b5), _mm_srai_epi16(b5, 2)), _mm_srai_epi16(b5, 4));
	a7 = _mm_add_epi16(_mm_sub_epi16(_mm_sub_epi16(_mm_srai_epi16(b4, 2), b7), _mm_srai_epi16(b7, 2)), _mm_srai_epi16(b7, 4));

	// even stage 2
	a0 = _mm_add_epi16(b0, b2);
	a1 = _mm_add_epi16(b1, b3);
	a2 = _mm_sub_epi16(b1, b3);
	a3 = _mm_sub_epi16(b0, b2);

	// stage 1
	o0 = _mm_add_epi16(a0, a4);
	o1 = _mm_add_epi16(a1, a5);
	o2 = _mm_add_epi16(a2, a6);
	o3 = _mm_add_epi16(a3, a7);
	o4 = _mm_sub_epi16(a3, a7);
	o5 = _mm_sub_epi16(a2, a6);
	o6 = _mm_sub_epi16(a1, a5);
	o7 = _mm_sub_epi16(a0, a4);

	if (_descale != 0) {
		bias = _mm_set1_epi16(16);
		o0 = _mm_srai_epi16(_mm_add_epi16(o0, bias), 5);
		o1 = _mm_srai_epi16(_mm_add_epi16(o1, bias), 5);
		o2 = _mm_srai_epi16(_mm_add_epi16(o2, bias), 5);
		o3 = _mm_srai_epi16(_mm_add_epi16(o3, bias), 5);
		o4 = _mm_srai_epi16(_mm_add_epi16(o4, bias), 5);
		o5 = _mm_srai_epi16(_mm_add_epi16(o5, bias), 5);
		o6 = _mm_srai_epi16(_mm_add_epi16(o6, bias), 5);
		o7 = _mm_srai_epi16(_mm_add_epi16(o7, bias), 5);
	}

	_mm_store_si128((__m128i*)(_y + 0 * 8), o0);
	_mm_store_si128((__m128i*)(_y + 1 * 8), o1);
	_mm_store_si128((__m128i*)(_y + 2 * 8), o2);
	_mm_store_si128((__m128i*)(_y + 3 * 8), o3);
	_mm_store_si128((__m128i*)(_y + 4 * 8), o4);
	_mm_store_si128((__m128i*)(_y + 5 * 8), o5);
	_mm_store_si128((__m128i*)(_y + 6 * 8), o6);
	_mm_store_si128((__m128i*)(_y + 7 * 8), o7);
}

/*Performs an inverse 8x8 Type-II DCT transform.
The input is assumed to be scaled by a factor of 4 relative to orthonormal
version of the transform.
_y: The buffer to store the result in.
This may be the same as _x.
_x: The input coefficients.*/
static void oc_idct8x8_sse2_slow(ogg_int16_t _y[64], ogg_int16_t _x[64]) {
  OC_ALIGN16(ogg_int16_t w[64]);
  int         i;

  idct8x8_b2_vert_sse(w, _x, 0);
  transpose_8x8_sse(w, w);
  idct8x8_b2_vert_sse(_y, w, 1);

  /*Clear input data for next block.*/
  for (i = 0; i<64; i+=8)
    _mm_store_si128((__m128i*)(&_x[i]), _mm_setzero_si128());
}

void oc_idct8x8_sse2(ogg_int16_t _y[64], ogg_int16_t _x[64],int _last_zzi) {
  oc_idct8x8_sse2_slow(_y, _x);
}

#endif

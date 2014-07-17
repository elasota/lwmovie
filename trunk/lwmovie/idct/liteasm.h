#include <emmintrin.h>

#define SSE2_SETZERO(reg)					reg = _mm_setzero_si128();
#define SSE2_PXOR_RR(dest, rs)				reg = _mm_xor_si128(dest, rs);
#define SSE2_PSUBW_RR(dest, rs)				dest = _mm_sub_epi16(dest, rs);
#define SSE2_PADDD_RCONST(dest, rs)			dest = _mm_add_epi32(dest, rs);
#define SSE2_PSUBD_RR(dest, rs)				dest = _mm_sub_epi32(dest, rs);


#define SSE2_LOADCONSTANT(reg, value)		reg = _mm_load_si128(&value);
#define SSE2_MOVDQA_RR(dest, src)			dest = src;
#define SSE2_MOVDQA_MEMR(dest, reg)			_mm_store_si128(dest, reg);
#define SSE2_MOVDQA_RMEM(reg, srcloc)		reg = _mm_load_si128(srcloc);
#define SSE2_MOVDQA_RXREG(reg, srcreg)		reg = _mm_load_si128((const __m128i *)srcreg);

#define SSE2_PMULHUW_RR(dest, rs)			dest = _mm_mulhi_epu16(dest, rs);
#define SSE2_PMULHUW_RCONST(dest, cloc)		dest = _mm_mulhi_epu16(dest, cloc);
#define SSE2_PMULHW_RR(dest, rs)			dest = _mm_mulhi_epi16(dest, rs);
#define SSE2_PMULHW_RCONST(dest, rs)		dest = _mm_mulhi_epi16(dest, cloc);
#define SSE2_PSLLD_R(reg, count)			reg = _mm_slli_epi32(reg, count);
#define SSE2_PMULLW_RR(dest, rs)			dest = _mm_mullo_epi16(dest, rs);
#define SSE2_PMULLW_RCONST(dest, cloc)		dest = _mm_mullo_epi16(dest, cloc);
#define SSE2_PADDD_RR(dest, src)			dest = _mm_add_epi32(dest, src);
#define SSE2_PUNPCKLWD_RR(dest, src)		dest = _mm_unpacklo_epi16(dest, src);
#define SSE2_PUNPCKLWD_RR(dest, src)		dest = _mm_unpacklo_epi16(dest, src);
#define SSE2_PSRAD_R(reg, count)			reg = _mm_srai_epi32(reg, count);
#define SSE2_PUNPCKHWD_RR(dest, rs)			dest = _mm_unpackhi_epi16(dest, rs);
#define SSE2_PSHUFHW_RR(dest, src, imm)		dest = _mm_shufflehi_epi16(src, imm);
#define SSE2_PSHUFLW_RR(dest, src, imm)		dest = _mm_shufflelo_epi16(src, imm);
#define SSE2_PSHUFD_RR(dest, src, imm)		dest = _mm_shuffle_epi32(src, imm);
#define SSE2_PINSRW_RR(dest, srcreg, offs)	dest = _mm_insert_epi16(dest, srcreg, offs);
#define SSE2_PSRLD_R(reg, count)			reg = _mm_srli_epi32(reg, count);
#define SSE2_PACKSSDW_RMEM(dest, srcloc)	dest = _mm_packs_epi32(dest, _mm_load_si128(srcloc));
#define SSE2_PACKSSDW_RR(dest, rs)			dest = _mm_packs_epi32(dest, rs);
#define SSE2_PMADDWD_RR(dest, rs)			dest = _mm_madd_epi16(dest, rs);


#define LOCAL_M128I_OFFS(symbol, index)	(symbol + (index))
#define XMMDATA_M128I_OFFS(index)		((__m128i*)(bx + (index)*16))
#define REG_AS_ADDRESS(reg)				((__m128i*)reg)


#define SET_REG(reg, imm)					reg = imm;
#define ADD_REG(reg, imm)					reg += imm;
#define SUB_REG(reg, imm)					reg -= imm;
#define DEC_REG_JNZ(reg, label)				if((--reg) != 0) goto label;
#define FCLABEL(label)						label:

#define SSE2_DECLCONSTANT_32(name, v0, v1, v2, v3)						static const __m128i name = _mm_set_epi32(v3, v2, v1, v0);
#define SSE2_DECLCONSTANT_16(name, v0, v1, v2, v3, v4, v5, v6, v7)		static const __m128i name = _mm_set_epi16(v7, v6, v5, v4, v3, v2, v1, v0);

#define FUNCTION_HEADER	\
	void j_rev_dct_sse2( short data[64] )\
	{\
		__m128i processedRows[8];\
		__m128i stored_temps[8];\
		char *bx = (char*)data;\
		int cx;\
		char *dx = (char*)processedRows;\
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;\

#define FUNCTION_FOOTER }

#ifndef SSE2_INLINE_SET1
	#define SSE2_DECL_SCALAR_CONSTANT_32(name, value)	SSE2_DECLCONSTANT_32(name, value, value, value, value)
	#define SSE2_DECL_SCALAR_CONSTANT_16(name, value)	SSE2_DECLCONSTANT_16(name, value, value, value, value, value, value, value, value)
	#define SSE2_SET1_32(reg, value)					SSE2_LOADCONSTANT(reg, value)
	#define SSE2_SET1_16(reg, value)					SSE2_LOADCONSTANT(reg, value)
#else
	#define SSE2_DECL_SCALAR_CONSTANT_32(name, value)	enum { name = value };
	#define SSE2_DECL_SCALAR_CONSTANT_16(name, value)	enum { name = value };
	#define SSE2_SET1_16(reg, value)				reg = _mm_set1_epi16(value);
	#define SSE2_SET1_32(reg, value)				reg = _mm_set1_epi32(value);
#endif


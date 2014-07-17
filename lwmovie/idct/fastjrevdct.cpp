#include "liteasm.h"

#define CONST_BITS		13
#define PASS1_BITS		2
#define CONST_SCALE		8192
#define DCTSIZE			8

// Pass order is hi-lo so packing can have the low data at the right time
#define PASS_LO		1
#define PASS_HI		0
#define STORED_TEMP_TMP10	0
#define STORED_TEMP_TMP11	2
#define STORED_TEMP_TMP12	4
#define STORED_TEMP_TMP13	6

#define STORED_TEMP_TMP10_LO	(STORED_TEMP_TMP10 + PASS_LO)
#define STORED_TEMP_TMP10_HI	(STORED_TEMP_TMP10 + PASS_HI)
#define STORED_TEMP_TMP11_LO	(STORED_TEMP_TMP11 + PASS_LO)
#define STORED_TEMP_TMP11_HI	(STORED_TEMP_TMP11 + PASS_HI)
#define STORED_TEMP_TMP12_LO	(STORED_TEMP_TMP12 + PASS_LO)
#define STORED_TEMP_TMP12_HI	(STORED_TEMP_TMP12 + PASS_HI)
#define STORED_TEMP_TMP13_LO	(STORED_TEMP_TMP13 + PASS_LO)
#define STORED_TEMP_TMP13_HI	(STORED_TEMP_TMP13 + PASS_HI)

#define FIX_0_211164243 1730
#define FIX_0_275899380 2260
#define FIX_0_298631336 2446
#define FIX_0_390180644 3196
#define FIX_0_509795579 4176
#define FIX_0_541196100 4433
#define FIX_0_601344887 4926
#define FIX_0_765366865 6270
#define FIX_0_785694958 6436
#define FIX_0_899976223 7373
#define FIX_1_061594337 8697
#define FIX_1_111140466 9102
#define FIX_1_175875602 9633
#define FIX_1_306562965 10703
#define FIX_1_387039845 11363
#define FIX_1_451774981 11893
#define FIX_1_501321110 12299
#define FIX_1_662939225 13623
#define FIX_1_847759065 15137
#define FIX_1_961570560 16069
#define FIX_2_053119869 16819
#define FIX_2_172734803 17799
#define FIX_2_562915447 20995
#define FIX_3_072711026 25172

#define FIX_1 (1 << (CONST_BITS))

SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_0_541196100, FIX_0_541196100)
SSE2_DECL_SCALAR_CONSTANT_16(S16_N_FIX_1_847759065, -FIX_1_847759065)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_0_765366865, FIX_0_765366865)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_1_961570560, FIX_1_961570560)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_0_390180644, FIX_0_390180644)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_0_899976223, FIX_0_899976223)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_0_298631336, FIX_0_298631336)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_1_501321110, FIX_1_501321110)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_2_562915447, FIX_2_562915447)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_2_053119869, FIX_2_053119869)
SSE2_DECL_SCALAR_CONSTANT_16(S16_FIX_3_072711026, FIX_3_072711026)

SSE2_DECLCONSTANT_16(S16_FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602, FIX_1_175875602)

/* Sign-extends low 4 units of 16x8 reg_inout to 32x4 */
/* Outputs to reg_inout */
#define m128i_sex16_lo(reg_inout)	\
	SSE2_PUNPCKLWD_RR(reg_inout, reg_inout)\
	SSE2_PSRAD_R(reg_inout, 16)
     
/* Sign-extends high 4 units of 16x8 reg_inout to 32x4 */
/* Outputs to reg_inout */
#define m128i_sex16_hi(reg_inout)	\
	SSE2_PUNPCKHWD_RR(reg_inout, reg_inout)\
	SSE2_PSRAD_R(reg_inout, 16)

/* Sign-extends 16x8 reg_inout_lo to 32x8 */
/* Outputs to reg_inout_lo:reg_out_hi */
#define m128i_sex16(reg_inout_lo, reg_out_hi)	\
	SSE2_MOVDQA_RR(reg_out_hi, reg_inout_lo)\
	m128i_sex16_hi(reg_out_hi)\
	m128i_sex16_lo(reg_inout_lo)

/* Shuffles high 4 units 16x8 reg_in */
/* Outputs to reg_out */
#define m128i_shuffle_hi(reg_out, reg_in, s0, s1, s2, s3)	\
	SSE2_PSHUFHW_RR(reg_out, reg_in, ((s0) | (s1 << 2) | (s2 << 4) | (s3 << 6)))

/* Shuffles low 4 units 16x8 reg_in */
/* Outputs to reg_out */
#define m128i_shuffle_lo(reg_out, reg_in, s0, s1, s2, s3)	\
	SSE2_PSHUFLW_RR(reg_out, reg_in, ((s0) | (s1 << 2) | (s2 << 4) | (s3 << 6)))

/* Shuffles high 4 units 16x8 reg_in */
/* Outputs to reg_out */
#define m128i_shuffle(reg_out, reg_in, s0, s1, s2, s3)	\
	SSE2_PSHUFD_RR(reg_out, reg_in, ((s0) | (s1 << 2) | (s2 << 4) | (s3 << 6)))

/* Multiplies 32x8 reg_inout_lo:reg_inout_hi by scalar constant 16x8 factor */
/* Outputs to reg_inout_lo:reg_inout_hi */
/* Clobbers reg_temp1 and reg_temp2 */
#define m32i8_mul_16_pos_constant(reg_inout_lo, reg_inout_hi, reg_temp1, reg_temp2, factor)	\
	SSE2_SET1_16(reg_temp1, factor)\
	SSE2_MOVDQA_RR(reg_temp2, reg_inout_lo)\
	SSE2_PMULHUW_RR(reg_temp2, reg_temp1)\
	SSE2_PSLLD_R(reg_temp2, 16)\
	SSE2_PMULLW_RR(reg_inout_lo, reg_temp1)\
	SSE2_PADDD_RR(reg_inout_lo, reg_temp2)\
	SSE2_MOVDQA_RR(reg_temp2, reg_inout_hi)\
	SSE2_PMULHUW_RR(reg_temp2, reg_temp1)\
	SSE2_PSLLD_R(reg_temp2, 16)\
	SSE2_PMULLW_RR(reg_inout_hi, reg_temp1)\
	SSE2_PADDD_RR(reg_inout_hi, reg_temp2)\

/* Multiplies 32x4 reg_inout by positive scalar constant 16x8 factor */
/* Outputs to reg_inout */
/* Clobbers reg_temp1 and reg_temp2 */
#define m32i4_mul_16_pos_constant(reg_inout, reg_temp1, reg_temp2, factor)	\
	SSE2_SET1_16(reg_temp1, factor)\
	SSE2_MOVDQA_RR(reg_temp2, reg_inout)\
	SSE2_PMULHUW_RR(reg_temp2, reg_temp1)\
	SSE2_PSLLD_R(reg_temp2, 16)\
	SSE2_PMULLW_RR(reg_inout, reg_temp1)\
	SSE2_PADDD_RR(reg_inout, reg_temp2)

/* Multiplies 32x4 reg_inout by positive scalar constant 16x8 factor (register-conserving version) */
/* Outputs to reg_inout */
/* Clobbers reg_temp1 */
#define m32i4_mul_16_pos_constant_rc(reg_inout, reg_temp1, factor, factor_scalar)	\
	SSE2_MOVDQA_RR(reg_temp1, reg_inout)\
	SSE2_PMULHUW_RCONST(reg_temp1, factor_scalar)\
	SSE2_PSLLD_R(reg_temp1, 16)\
	SSE2_PMULLW_RCONST(reg_inout, factor_scalar)\
	SSE2_PADDD_RR(reg_inout, reg_temp1)

/* Multiplies 32x4 reg_in_clobber by positive scalar constant 16x8 factor */
/* Outputs to reg_out */
/* Clobbers reg_in_clobber */
#define m32i4_mul_16_neg_constant(reg_inout, reg_temp1, reg_temp2, in16)	\
	SSE2_SETZERO(reg_temp1)\
	SSE2_PSUBD_RR(reg_temp1, reg_inout)\
	SSE2_SET1_16(reg_inout, in16)\
	SSE2_MOVDQA_RR(reg_temp2, reg_temp1)\
	SSE2_PMULHUW_RR(reg_temp1, reg_inout)\
	SSE2_PSLLD_R(reg_temp2, 16)\
	SSE2_PMULLW_RR(reg_inout, reg_temp1)\
	SSE2_PADDD_RR(reg_inout, reg_temp2)

/* Multiplies 32x4 reg_in_clobber by positive scalar constant 16x8 factor (register-conserving) */
/* Outputs to reg_out */
/* Clobbers reg_in_clobber */
#define m32i4_mul_16_neg_constant_rc(reg_in_clobber, reg_out, factor, factor_scalar)	\
	SSE2_SETZERO(reg_out)\
	SSE2_PSUBD_RR(reg_out, reg_in_clobber)\
	SSE2_MOVDQA_RR(reg_in_clobber, reg_out)\
	SSE2_PMULHUW_RCONST(reg_in_clobber, factor_scalar)\
	SSE2_PSLLD_R(reg_in_clobber, 16);\
	SSE2_PMULLW_RR(reg_out, factor_scalar)\
	SSE2_PADDD_RR(reg_out, reg_in_clobber)


#define m32i4_add(reg_inout_lo, reg_inout_hi, reg_rs_lo, reg_rs_hi)	\
	SSE2_PADDD_RR(reg_inout_lo, reg_rs_lo)\
	SSE2_PADDD_RR(reg_inout_hi, reg_rs_hi)\

// Register-conserving add-sub
// reg_in_ls_out_sub - reg_in_rs_out_add --> reg_in_ls_out_sub
// reg_in_ls_out_sub + reg_in_rs_out_add --> reg_in_rs_out_add
#define m32i4_addsub(reg_in_ls_out_sub, reg_in_rs_out_add) \
	SSE2_PSUBD_RR(reg_in_ls_out_sub, reg_in_rs_out_add)\
	SSE2_PSLLD_R(reg_in_rs_out_add, 1)\
	SSE2_PADDD_RR(reg_in_rs_out_add, reg_in_ls_out_sub)

#define m32i4_sub(reg_inout_lo, reg_inout_hi, reg_rs_lo, reg_rs_hi)	\
	SSE2_PSUBD_RR(reg_inout_lo, reg_rs_lo)\
	SSE2_PSUBD_RR(reg_inout_hi, reg_rs_hi)

// Creates a 32i4 from a 16i4 multiplied by a factor
// __m128i wideMulFactor = _mm_set1_epi16(factor);
// __m128i mulHi = _mm_mulhi_epi16(base128, wideMulFactor);
// __m128i mulLo = _mm_mullo_epi16(base128, wideMulFactor);
// result.hi = _mm_unpackhi_epi16(mulLo, mulHi);
// result.lo = _mm_unpacklo_epi16(mulLo, mulHi);
#define m32i4_from_16x4_mul_16i(reg_out_lo, reg_out_hi, reg_in_clobber, factor)	\
	SSE2_SET1_16(reg_out_hi, factor)\
	SSE2_MOVDQA_RR(reg_out_lo, reg_in_clobber)\
	SSE2_PMULHW_RR(reg_in_clobber, reg_out_hi)\
	SSE2_PMULLW_RR(reg_out_lo, reg_out_hi)\
	SSE2_MOVDQA_RR(reg_out_hi, reg_out_lo)\
	SSE2_PUNPCKHWD_RR(reg_out_hi, reg_in_clobber)\
	SSE2_PUNPCKLWD_RR(reg_out_lo, reg_in_clobber)

#define m32i4_descale_export_pass_1_rc(reg, descale_bit_add_mem, descale_bits, out_loc)	\
	SSE2_PADDD_RCONST(reg, descale_bit_add_mem)\
	SSE2_PSRAD_R(reg, descale_bits)\
	SSE2_MOVDQA_MEMR(out_loc, reg)


#define m32i4_descale_export_pass_2_rc(reg, descale_bit_add_mem, descale_bits, out_loc)	\
	SSE2_PADDD_RCONST(reg, descale_bit_add_mem)\
	SSE2_PSRAD_R(reg, descale_bits)\
	SSE2_PACKSSDW_RMEM(reg, out_loc)\
	SSE2_MOVDQA_MEMR(out_loc, reg)


#define m32i4_descale_export_pass_1(reg, descale_reg, descale_bits, out_loc)	\
	SSE2_PADDD_RR(reg, descale_reg)\
	SSE2_PSRAD_R(reg, descale_bits)\
	SSE2_MOVDQA_MEMR(out_loc, reg)


#define m32i4_descale_export_pass_2(reg, descale_reg, descale_bits, out_loc)	\
	SSE2_PADDD_RR(reg, descale_reg)\
	SSE2_PSRAD_R(reg, descale_bits)\
	SSE2_PACKSSDW_RMEM(reg, out_loc)\
	SSE2_MOVDQA_MEMR(out_loc, reg)

#define m128i_first_descale_init(reg_rounder, bits)	\
	SSE2_SET1_32(reg_rounder, S32_FIRST_DESCALE_BITS_ROUND)

// Descales reg_inout
#define m128i_descale_32(reg_inout, reg_rounder, bits)	\
	SSE2_PADDD_RR(reg_inout, reg_rounder)\
	SSE2_PSRAD_R(reg_inout, (bits))\

SSE2_DECLCONSTANT_16(sse_mul_1, FIX_1, (FIX_0_541196100+FIX_0_765366865),
		FIX_1, FIX_0_541196100,
		FIX_1, -FIX_0_541196100,
		FIX_1, (-FIX_0_765366865-FIX_0_541196100) )

SSE2_DECLCONSTANT_16(sse_mul_2, FIX_1, FIX_0_541196100,
		-FIX_1, (FIX_0_541196100-FIX_1_847759065),
		-FIX_1, (FIX_1_847759065-FIX_0_541196100),
		FIX_1, -FIX_0_541196100 )

SSE2_DECLCONSTANT_16(sse_mul_3, (FIX_1_501321110+FIX_1_175875602-FIX_0_390180644-FIX_0_899976223), FIX_1_175875602,
		FIX_1_175875602, (FIX_3_072711026+FIX_1_175875602-FIX_1_961570560-FIX_2_562915447),
		(FIX_1_175875602-FIX_0_390180644), (FIX_1_175875602-FIX_2_562915447),
		(FIX_1_175875602-FIX_0_899976223), (FIX_1_175875602-FIX_1_961570560) )

SSE2_DECLCONSTANT_16(sse_mul_4, (FIX_1_175875602-FIX_0_390180644), (FIX_1_175875602-FIX_0_899976223),
		(FIX_1_175875602-FIX_2_562915447), (FIX_1_175875602-FIX_1_961570560),
		(FIX_2_053119869+FIX_1_175875602-FIX_0_390180644-FIX_2_562915447), FIX_1_175875602,
		FIX_1_175875602, (FIX_0_298631336+FIX_1_175875602-FIX_1_961570560-FIX_0_899976223) );

#define FIRST_DESCALE_BITS_ROUND	(1<<((CONST_BITS-PASS1_BITS)-1))


#define FINAL_DESCALE_BITS	(CONST_BITS+PASS1_BITS+3)
#define FINAL_DESCALE_BITS_ROUND	(1<<((FINAL_DESCALE_BITS)-1))

SSE2_DECL_SCALAR_CONSTANT_32(S32_FIRST_DESCALE_BITS_ROUND, FIRST_DESCALE_BITS_ROUND)

SSE2_DECLCONSTANT_32(final_descale_bit_mem, FINAL_DESCALE_BITS_ROUND, FINAL_DESCALE_BITS_ROUND, FINAL_DESCALE_BITS_ROUND, FINAL_DESCALE_BITS_ROUND)

#define odd_part_pass_1_start	\
	SSE2_MOVDQA_RMEM(xmm0, LOCAL_M128I_OFFS(processedRows, 7))\
	m128i_sex16_hi(xmm0)\
	SSE2_MOVDQA_RMEM(xmm1, LOCAL_M128I_OFFS(processedRows, 5))\
	m128i_sex16_hi(xmm1)\
	SSE2_MOVDQA_RMEM(xmm2, LOCAL_M128I_OFFS(processedRows, 3))\
	m128i_sex16_hi(xmm2)\
	SSE2_MOVDQA_RMEM(xmm3, LOCAL_M128I_OFFS(processedRows, 1))\
	m128i_sex16_hi(xmm3)

#define odd_part_pass_2_start	\
	SSE2_MOVDQA_RMEM(xmm0, LOCAL_M128I_OFFS(processedRows, 7))\
	m128i_sex16_lo(xmm0)\
	SSE2_MOVDQA_RMEM(xmm1, LOCAL_M128I_OFFS(processedRows, 5))\
	m128i_sex16_lo(xmm1)\
	SSE2_MOVDQA_RMEM(xmm2, LOCAL_M128I_OFFS(processedRows, 3))\
	m128i_sex16_lo(xmm2)\
	SSE2_MOVDQA_RMEM(xmm3, LOCAL_M128I_OFFS(processedRows, 1))\
	m128i_sex16_lo(xmm3)

#define load_stored_temp(reg, index)	\
	SSE2_MOVDQA_RMEM(reg, LOCAL_M128I_OFFS(stored_temps, index))

#define odd_part_export_pass_1_rc(reg1, out1, reg2, out2, descale_bits)	\
	m32i4_descale_export_pass_1_rc(reg1, final_descale_bit_mem, descale_bits, XMMDATA_M128I_OFFS(out1))\
	m32i4_descale_export_pass_1_rc(reg2, final_descale_bit_mem, descale_bits, XMMDATA_M128I_OFFS(out2))

#define odd_part_export_pass_2_rc(reg1, out1, reg2, out2, descale_bits)	\
	m32i4_descale_export_pass_2_rc(reg1, final_descale_bit_mem, descale_bits, XMMDATA_M128I_OFFS(out1))\
	m32i4_descale_export_pass_2_rc(reg2, final_descale_bit_mem, descale_bits, XMMDATA_M128I_OFFS(out2))

#define export_macro_norc_init(temp_reg)	\
	SSE2_LOADCONSTANT(temp_reg, final_descale_bit_mem)

#define odd_part_export_pass_1(reg1, out1, reg2, out2, descale_reg, descale_bits)	\
	m32i4_descale_export_pass_1(reg1, descale_reg, descale_bits, XMMDATA_M128I_OFFS(out1))\
	m32i4_descale_export_pass_1(reg2, descale_reg, descale_bits, XMMDATA_M128I_OFFS(out2))

#define odd_part_export_pass_2(reg1, out1, reg2, out2, descale_reg, descale_bits)	\
	m32i4_descale_export_pass_2(reg1, descale_reg, descale_bits, XMMDATA_M128I_OFFS(out1));\
	m32i4_descale_export_pass_2(reg2, descale_reg, descale_bits, XMMDATA_M128I_OFFS(out2))

#define odd_part(pass, export_macro_rc, export_macro)	\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3 */\
	/*m256 z3 = tmp0.Add(tmp2); */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z3 */\
	SSE2_MOVDQA_RR(xmm4, xmm0)\
	SSE2_PADDD_RR(xmm4, xmm2)\
	/*m256 z4 = tmp1.Add(tmp3); */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z3  xmm5: z4 */\
	SSE2_MOVDQA_RR(xmm5, xmm1)\
	SSE2_PADDD_RR(xmm5, xmm3)\
	/*m256 z5 = z3.Add(z4).Mul16Constant(FIX(1.175875602)); // sqrt(2) * c3 */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z3  xmm5: z4  xmm6: z5 */\
	SSE2_MOVDQA_RR(xmm6, xmm4)\
	SSE2_PADDD_RR(xmm6, xmm5)\
	m32i4_mul_16_pos_constant_rc(xmm6, xmm7, FIX_1_175875602, S16_FIX_1_175875602);\
	/* z3 = z3.Mul16Constant(- FIX(1.961570560)); // sqrt(2) * (-c3-c5) */\
	/* z3 = z3.Add(z5); */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm5: z4  xmm6: z5  xmm7: z3*/\
	m32i4_mul_16_neg_constant_rc(xmm4, xmm7, FIX_1_961570560, S16_FIX_1_961570560);\
	SSE2_PADDD_RR(xmm7, xmm6)\
	/* z4 = z4.Mul16Constant(- FIX(0.390180644)); // sqrt(2) * (c5-c3) */\
	/* z4 = z4.Add(z5);	// EOL for z5 */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm7: z3*/\
	m32i4_mul_16_neg_constant_rc(xmm5, xmm4, FIX_0_390180644, S16_FIX_0_390180644);\
	SSE2_PADDD_RR(xmm4, xmm6)\
	/* z1 = tmp0_1.Add(tmp3_1); */\
	/* z1 = z1.Mul16Constant(- FIX(0.899976223)); // sqrt(2) * (c7-c3) */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm6: z1  xmm7: z3*/\
	SSE2_MOVDQA_RR(xmm5, xmm0)\
	SSE2_PADDD_RR(xmm5, xmm3)\
	m32i4_mul_16_neg_constant_rc(xmm5, xmm6, FIX_0_899976223, S16_FIX_0_899976223);\
	/* m256 tmp0 = tmp0.Mul16Constant(FIX(0.298631336)); // sqrt(2) * (-c1+c3+c5-c7) */\
	/* m256 tmp3 = tmp3.Mul16Constant(FIX(1.501321110)); // sqrt(2) * ( c1+c3-c5-c7) */\
	/* xmm0: tmp0  xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm6: z1  xmm7: z3*/\
	m32i4_mul_16_pos_constant_rc(xmm0, xmm5, FIX_0_298631336, S16_FIX_0_298631336);\
	m32i4_mul_16_pos_constant_rc(xmm3, xmm5, FIX_1_501321110, S16_FIX_1_501321110);\
	/* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */\
	/* tmp0 = tmp0.Add(z1).Add(z3); */\
	/* descale tmp13.Add(tmp0) --> 3 */\
	/* descale tmp13.Sub(tmp0) --> 4 */\
	/* xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm6: z1  xmm7: z3*/\
	SSE2_PADDD_RR(xmm0, xmm6)\
	SSE2_PADDD_RR(xmm0, xmm7)\
	load_stored_temp(xmm5, STORED_TEMP_TMP13 + pass);\
	m32i4_addsub(xmm5, xmm0);\
	export_macro_rc(xmm0, 3, xmm5, 4, (CONST_BITS+PASS1_BITS+3));\
	/* m256 z2 = tmp1.Add(tmp2); */\
	/* z2 = z2.Mul16Constant(- FIX(2.562915447)); // sqrt(2) * (-c1-c3) */\
	/* xmm1: tmp1  xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm5: z2  xmm6: z1  xmm7: z3*/\
	SSE2_MOVDQA_RR(xmm0, xmm1)\
	SSE2_PADDD_RR(xmm0, xmm2)\
	m32i4_mul_16_neg_constant_rc(xmm0, xmm5, FIX_2_562915447, S16_FIX_2_562915447);\
	/* tmp1 = tmp1.Mul16Constant(FIX(2.053119869)); // sqrt(2) * ( c1+c3-c5+c7) */\
	/* tmp1 = tmp1.Add(z2).Add(z4); */\
	/* descale tmp12.Add(tmp1) --> 2 */\
	/* descale tmp12.Sub(tmp1) --> 5 */\
	/* xmm2: tmp2  xmm3: tmp3  xmm4: z4  xmm5: z2  xmm6: z1  xmm7: z3*/\
	m32i4_mul_16_pos_constant_rc(xmm1, xmm0, FIX_2_053119869, S16_FIX_2_053119869);\
	SSE2_PADDD_RR(xmm1, xmm5)\
	SSE2_PADDD_RR(xmm1, xmm4)\
	load_stored_temp(xmm0, STORED_TEMP_TMP12 + pass);\
	m32i4_addsub(xmm0, xmm1);\
	export_macro_rc(xmm1, 2, xmm0, 5, (CONST_BITS+PASS1_BITS+3));\
	/* tmp2 = tmp2.Mul16Constant(FIX(3.072711026)); // sqrt(2) * ( c1+c3+c5-c7) */\
	/* tmp2 = tmp2.Add(z2).Add(z3);	// EOL for z2 and z3 */\
	/* descale tmp11.Add(tmp2) --> 1 */\
	/* descale tmp11.Sub(tmp2) --> 6 */\
	/* Less register pressure, don't have to use _rc or addsub */\
	/* xmm3: tmp3  xmm4: z4  xmm6: z1*/\
	m32i4_mul_16_pos_constant(xmm2, xmm0, xmm1, S16_FIX_3_072711026);\
	SSE2_PADDD_RR(xmm2, xmm5)\
	SSE2_PADDD_RR(xmm2, xmm7)\
	load_stored_temp(xmm0, STORED_TEMP_TMP11 + pass);\
	SSE2_MOVDQA_RR(xmm1, xmm0)\
	SSE2_PSUBD_RR(xmm1, xmm2)\
	SSE2_PADDD_RR(xmm0, xmm2)\
	export_macro_norc_init(xmm2);\
	export_macro(xmm0, 1, xmm1, 6, xmm2, (CONST_BITS+PASS1_BITS+3));\
	/* tmp3 = tmp3.Add(z1).Add(z4); */\
	/* descale tmp10.Add(tmp3) --> 0 */\
	/* descale tmp10.Sub(tmp3) --> 7 */\
	SSE2_PADDD_RR(xmm3, xmm6)\
	SSE2_PADDD_RR(xmm3, xmm4)\
	load_stored_temp(xmm0, STORED_TEMP_TMP10 + pass);\
	SSE2_MOVDQA_RR(xmm1, xmm0)\
	SSE2_PSUBD_RR(xmm1, xmm3)\
	SSE2_PADDD_RR(xmm0, xmm3)\
	export_macro(xmm0, 0, xmm1, 7, xmm2, (CONST_BITS+PASS1_BITS+3))



FUNCTION_HEADER
	/* Prologue:
		bx = data
		cx = scratch
		dx = processedRows
		Also require 128 bytes for stored_temps
		*/

	/* Pass 1: process rows. */
	/* Note results are scaled up by sqrt(8) compared to a true IDCT; */
	/* furthermore, we scale the results by 2**PASS1_BITS. */

	SSE2_LOADCONSTANT(xmm4, sse_mul_1)
	SSE2_LOADCONSTANT(xmm5, sse_mul_2)
	SSE2_LOADCONSTANT(xmm6, sse_mul_3)
	SSE2_LOADCONSTANT(xmm7, sse_mul_4)

	SET_REG(cx, 8)

FCLABEL(pass1)
		SSE2_MOVDQA_RXREG(xmm0, bx)

		/* Due to quantization, we will usually find that many of the input
		 * coefficients are zero, especially the AC terms.  We can exploit this
		 * by short-circuiting the IDCT calculation for any row in which all
		 * the AC terms are zero.  In that case each output is equal to the
		 * DC coefficient (with scale factor as needed).
		 * With typical images and quantization tables, half or more of the
		 * row DCT calculations can be simplified this way.
		 */
		
		/* Even part: reverse the even part of the forward DCT. */
		/* The rotator is sqrt(2)*c(-6). */

		/*
			tmp10 = r0*FIX_C + r2*(FIX_0_541196100+FIX_0_765366865) + r4*FIX_C + r6*FIX_0_541196100;
			tmp11 = r0*FIX_C + r2*FIX_0_541196100 + r4*-FIX_C + r6*(FIX_0_541196100-FIX_1_847759065);
			tmp12 = r0*FIX_C + r2*-FIX_0_541196100 + r4*-FIX_C + r6*(FIX_1_847759065-FIX_0_541196100);
			tmp13 = r0*FIX_C + r2*(-FIX_0_765366865-FIX_0_541196100) + r4*FIX_C + r6*-FIX_0_541196100;
     
			tmp0 = r1*(FIX_1_501321110+FIX_1_175875602-FIX_0_390180644-FIX_0_899976223) + r3*FIX_1_175875602 + r5*(FIX_1_175875602-FIX_0_390180644) + r7*(FIX_1_175875602-FIX_0_899976223);
			tmp1 = r1*FIX_1_175875602 + r3*(FIX_3_072711026+FIX_1_175875602-FIX_1_961570560-FIX_2_562915447) + r5*(FIX_1_175875602-FIX_2_562915447) + r7*(FIX_1_175875602-FIX_1_961570560);
			tmp2 = r1*(FIX_1_175875602-FIX_0_390180644) + r3*(FIX_1_175875602-FIX_2_562915447) + r5*(FIX_2_053119869+FIX_1_175875602-FIX_0_390180644-FIX_2_562915447) + r7*FIX_1_175875602;
			tmp3 = r1*(FIX_1_175875602-FIX_0_899976223) + r3*(FIX_1_175875602-FIX_1_961570560) + r5*FIX_1_175875602 + r7*(FIX_0_298631336+FIX_1_175875602-FIX_1_961570560-FIX_0_899976223);
		*/

		//__m128i r00004444 = m128i_shuffle_hi(m128i_shuffle_lo(dctRow, 0, 0, 0, 0), 0, 0, 0, 0);
		//__m128i r22226666 = m128i_shuffle_hi(m128i_shuffle_lo(dctRow, 2, 2, 2, 2), 2, 2, 2, 2);
		//__m128i r02020202 = _mm_unpacklo_epi16(r00004444, r22226666);
		//__m128i r46464646 = _mm_unpackhi_epi16(r00004444, r22226666);
		// xmm0: dctRow  xmm1: r46464646  xmm2: CLOBBER   xmm3: r02020202
		m128i_shuffle_lo(xmm1, xmm0, 0, 0, 0, 0)
		m128i_shuffle_hi(xmm1, xmm1, 0, 0, 0, 0)
		m128i_shuffle_lo(xmm2, xmm0, 2, 2, 2, 2)
		m128i_shuffle_hi(xmm2, xmm2, 2, 2, 2, 2)
		SSE2_MOVDQA_RR(xmm3, xmm1)
		SSE2_PUNPCKLWD_RR(xmm3, xmm2)
		SSE2_PUNPCKHWD_RR(xmm1, xmm2)

		//__m128i tmp1 = _mm_add_epi32(_mm_madd_epi16(r02020202, sse_mul_1_preload), _mm_madd_epi16(r46464646, sse_mul_2_preload));
		// xmm0: dctRow  xmm1: CLOBBER  xmm2: CLOBBER   xmm3: tmp1
		SSE2_PMADDWD_RR(xmm3, xmm4)
		SSE2_PMADDWD_RR(xmm1, xmm5)
		SSE2_PADDD_RR(xmm3, xmm1)

		/* Odd part per figure 8; the matrix is unitary and hence its
		 * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
		 */

		/*
			tmp0 = r1*(FIX_1_501321110+FIX_1_175875602-FIX_0_390180644-FIX_0_899976223) + r3*FIX_1_175875602 + r5*(FIX_1_175875602-FIX_0_390180644) + r7*(FIX_1_175875602-FIX_0_899976223);
			tmp1 = r1*FIX_1_175875602 + r3*(FIX_3_072711026+FIX_1_175875602-FIX_1_961570560-FIX_2_562915447) + r5*(FIX_1_175875602-FIX_2_562915447) + r7*(FIX_1_175875602-FIX_1_961570560);
			tmp2 = r1*(FIX_1_175875602-FIX_0_390180644) + r3*(FIX_1_175875602-FIX_2_562915447) + r5*(FIX_2_053119869+FIX_1_175875602-FIX_0_390180644-FIX_2_562915447) + r7*FIX_1_175875602;
			tmp3 = r1*(FIX_1_175875602-FIX_0_899976223) + r3*(FIX_1_175875602-FIX_1_961570560) + r5*FIX_1_175875602 + r7*(FIX_0_298631336+FIX_1_175875602-FIX_1_961570560-FIX_0_899976223);
		*/
		//__m128i r11115555 = M128_ShuffleHi(M128_ShuffleLo(dctRow, 1, 1, 1, 1), 1, 1, 1, 1);
		//__m128i r33337777 = M128_ShuffleHi(M128_ShuffleLo(dctRow, 3, 3, 3, 3), 3, 3, 3, 3);
		//__m128i r13131313 = _mm_unpacklo_epi16(r11115555, r33337777);
		//__m128i r57575757 = _mm_unpackhi_epi16(r11115555, r33337777);
		// xmm0: r13131313  xmm1: r57575757  xmm2: CLOBBER   xmm3: tmp1
		m128i_shuffle_lo(xmm1, xmm0, 1, 1, 1, 1);
		m128i_shuffle_hi(xmm1, xmm1, 1, 1, 1, 1);	// xmm1 = r11115555
		m128i_shuffle_lo(xmm2, xmm0, 3, 3, 3, 3);
		m128i_shuffle_hi(xmm2, xmm2, 3, 3, 3, 3);	// xmm2 = r33337777
		SSE2_MOVDQA_RR(xmm0, xmm1)
		SSE2_PUNPCKLWD_RR(xmm0, xmm2)	// xmm0 = r13131313
		SSE2_PUNPCKHWD_RR(xmm1, xmm2)	// xmm1 = r57575757

		// __m128i tmp = _mm_add_epi32(_mm_madd_epi16(r13131313, sse_mul_3_preload), _mm_madd_epi16(r57575757, sse_mul_4_preload));
		SSE2_PMADDWD_RR(xmm0, xmm6)
		SSE2_PMADDWD_RR(xmm1, xmm7)
		SSE2_PADDD_RR(xmm0, xmm1)
		// xmm0: tmp    xmm1: CLOBBER  xmm2: CLOBBER   xmm3: tmp1

		/* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */
		/* dataptr[0..3] = tmp1 + tmp0 */
		/* dataptr[4..7] = reverse(tmp1 - tmp) */

		//__m128i out0to3 = DescaleSSE2(_mm_add_epi32(tmp1, tmp), CONST_BITS-PASS1_BITS);
		//__m128i out4to7 = DescaleSSE2(M128_Shuffle(_mm_sub_epi32(tmp1, tmp), 3, 2, 1, 0), CONST_BITS-PASS1_BITS);
		// xmm0: CLOBBER    xmm1: CLOBBER    xmm2: out0to3   xmm3: out4to7
		m128i_first_descale_init(xmm1, CONST_BITS-PASS1_BITS)
		SSE2_MOVDQA_RR(xmm2, xmm3)
		SSE2_PADDD_RR(xmm2, xmm0)
		m128i_descale_32(xmm2, xmm1, CONST_BITS-PASS1_BITS)

		SSE2_PSUBD_RR(xmm3, xmm0)
		m128i_shuffle(xmm3, xmm3, 3, 2, 1, 0);
		m128i_descale_32(xmm3, xmm1, CONST_BITS-PASS1_BITS)

		//processedRows[rowctr] = _mm_packs_epi32(out0to3, out4to7);
		// xmm0: CLOBBER    xmm1: CLOBBER    xmm2: out0to3   xmm3: out4to7
		SSE2_PACKSSDW_RR(xmm2, xmm3)
		SSE2_MOVDQA_MEMR(REG_AS_ADDRESS(dx), xmm2)

		ADD_REG(dx, 16)
		ADD_REG(bx, 16)
		DEC_REG_JNZ(cx, pass1)

	SUB_REG(dx, 128)
	SUB_REG(bx, 128)

	/* Pass 2: process columns. */
	/* Note that we must descale the results by a factor of 8 == 2**3, */
	/* and also undo the PASS1_BITS scaling. */

	/* Columns of zeroes can be exploited in the same way as we did with rows.
	 * However, the row calculation has created many nonzero AC terms, so the
	 * simplification applies less often (typically 5% to 10% of the time).
	 * On machines with very fast multiplication, it's possible that the
	 * test takes more time than it's worth.  In that case this section
	 * may be commented out.
	 */

	/* Even part: reverse the even part of the forward DCT. */
	/* The rotator is sqrt(2)*c(-6). */

	//__m128i z2short = processedRows[2];
	//__m128i z3short = processedRows[6];
	// xmm0: z2short   xmm2: z3short
	SSE2_MOVDQA_RMEM(xmm0, LOCAL_M128I_OFFS(processedRows, 2));
	SSE2_MOVDQA_RMEM(xmm2, LOCAL_M128I_OFFS(processedRows, 6));

	//m256 z2addz3 = m256::FromPacked16(z2short).Add(m256::FromPacked16(z3short));
	// xmm0: z2short   xmm2: z3short   xmm4: z2addz3_lo   xmm5: z2addz3_hi
	SSE2_MOVDQA_RR(xmm1, xmm0)
	m128i_sex16(xmm1, xmm3);
	SSE2_MOVDQA_RR(xmm4, xmm2)
	m128i_sex16(xmm4, xmm5);
	m32i4_add(xmm4, xmm5, xmm1, xmm3);

	// m256 z1 = z2addz3.Mul16Constant(FIX(0.541196100));
	// xmm0: z2short  xmm2: z3short  xmm4: z1_lo  xmm5: z1_hi   xmm6: CLOBBER   zmm7: CLOBBER
	m32i8_mul_16_pos_constant(xmm4, xmm5, xmm6, xmm7, S16_FIX_0_541196100);

	//m256 tmp2 = z1.Add(m256::FromM16Mul16(z3short, - FIX(1.847759065)));
	// xmm0: z2short  xmm2: CLOBBER  xmm4: z1_lo  xmm5: z1_hi   xmm6: tmp2_lo   xmm7: tmp2_hi
	m32i4_from_16x4_mul_16i(xmm6, xmm7, xmm2, S16_N_FIX_1_847759065);
	m32i4_add(xmm6, xmm7, xmm4, xmm5);

	//m256 tmp3 = z1.Add(m256::FromM16Mul16(z2short, FIX(0.765366865)));
	// xmm4:xmm5: tmp3  xmm6:xmm7: tmp2
	m32i4_from_16x4_mul_16i(xmm1, xmm2, xmm0, S16_FIX_0_765366865);
	m32i4_add(xmm4, xmm5, xmm1, xmm2);

	//m256 expanded0 = m256::FromPacked16(processedRows[0]);
	// xmm0:xmm1: expanded0  xmm4:xmm5: tmp3  xmm6:xmm7: tmp2
	SSE2_MOVDQA_RMEM(xmm0, LOCAL_M128I_OFFS(processedRows, 0));
	m128i_sex16(xmm0, xmm1)

	//m256 expanded4 = m256::FromPacked16(processedRows[4]);
	// xmm0:xmm1: expanded0  xmm2:xmm3: expanded4  xmm4:xmm5: tmp3  xmm6:xmm7: tmp2
	SSE2_MOVDQA_RMEM(xmm2, LOCAL_M128I_OFFS(processedRows, 4));
	m128i_sex16(xmm2, xmm3)

	//m256 tmp0 = expanded0.Add(expanded4).LeftShift(CONST_BITS);
	//m256 tmp1 = expanded0.Sub(expanded4).LeftShift(CONST_BITS);
	// xmm0:xmm1: tmp1  xmm2:xmm3: tmp0  xmm4:xmm5: tmp3  xmm6:xmm7: tmp2
	m32i4_addsub(xmm0, xmm2)
	SSE2_PSLLD_R(xmm0, CONST_BITS)
	SSE2_PSLLD_R(xmm2, CONST_BITS)
	m32i4_addsub(xmm1, xmm3)
	SSE2_PSLLD_R(xmm1, CONST_BITS)
	SSE2_PSLLD_R(xmm3, CONST_BITS)

	//m256 tmp10 = tmp0.Add(tmp3);
	//m256 tmp13 = tmp0.Sub(tmp3);
	// xmm0:xmm1: tmp1  xmm2:xmm3: tmp13  xmm4:xmm5: tmp10  xmm6:xmm7: tmp2
	m32i4_addsub(xmm2, xmm4);
	m32i4_addsub(xmm3, xmm5);

	// xmm0:xmm1: tmp1  xmm6:xmm7: tmp2
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP10_LO), xmm4)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP10_HI), xmm5)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP13_LO), xmm2)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP13_HI), xmm3)

	//m256 tmp11 = tmp1.Add(tmp2);
	//m256 tmp12 = tmp1.Sub(tmp2);
	// xmm0:xmm1: tmp11  xmm2:xmm3: tmp12
	SSE2_MOVDQA_RR(xmm2, xmm0)
	SSE2_MOVDQA_RR(xmm3, xmm1)
	m32i4_sub(xmm2, xmm3, xmm6, xmm7);
	m32i4_add(xmm0, xmm1, xmm6, xmm7);

	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP11_LO), xmm0)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP11_HI), xmm1)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP12_LO), xmm2)
	SSE2_MOVDQA_MEMR(LOCAL_M128I_OFFS(stored_temps, STORED_TEMP_TMP12_HI), xmm3)

	odd_part_pass_1_start
	odd_part(0, odd_part_export_pass_1_rc, odd_part_export_pass_1)
	odd_part_pass_2_start
	odd_part(1, odd_part_export_pass_2_rc, odd_part_export_pass_2)

FUNCTION_FOOTER

// Define one of these to specify an SIMD option
// So far, the fastest MP2 decoding is achieved using LWMOVIE_SSE41 with LWMOVIE_FIXEDPOINT.
// If SSE 4.1 is not available, the fastest is LWMOVIE_SSE2 without LWMOVIE_FIXEDPOINT.
// You must enable at least one option.

// Enable to support SSE 2 instructions
#define LWMOVIE_SSE2

// Define to support SSE 4.1 instructions
#define LWMOVIE_SSE41

// Define this to use fixed-point arithmetic
#define LWMOVIE_FIXEDPOINT

// Define this to not use any SIMD instruction set
//#define LWMOVIE_NOSIMD

// Define this to compile lwmovie as a DLL
#define LWMOVIE_DLL


#ifdef LWMOVIE_SSE42
	#define LWMOVIE_SSE2
#endif

#ifndef __LWMOVIE_ATTRIBS_H__
#define __LWMOVIE_ATTRIBS_H__

#ifdef _MSC_VER
#define LWMOVIE_ATTRIB_ALIGN(n) __declspec(align(n))
#define LWMOVIE_ATTRIB_ARCH(arch)
#define LWMOVIE_FORCEINLINE __forceinline
#endif

#ifdef __GNUC__
#define LWMOVIE_ATTRIB_ALIGN(n) __attribute__ ((aligned(n)))
#define LWMOVIE_ATTRIB_ARCH(arch) __attribute__ ((__target__ (arch)))
#define LWMOVIE_FORCEINLINE inline __attribute__((always_inline))
#endif

#endif

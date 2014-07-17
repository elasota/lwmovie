#ifndef __LWMOVIE_LAYER2_XMATH_HPP__
#define __LWMOVIE_LAYER2_XMATH_HPP__

#include "lwmovie_layer2.hpp"

namespace lwmovie
{
	namespace xmath
	{
		lwmSInt64 EMul(lwmSInt32 ls, lwmSInt32 rs);
	}
}

#ifdef _WIN32
#include <intrin.h>

#pragma intrinsic(__emul)
#pragma warning(disable:4293)   // Negative shifts

inline lwmSInt64 lwmovie::xmath::EMul(lwmSInt32 ls, lwmSInt32 rs)
{
	return __emul(ls, rs);
}

#endif



#endif

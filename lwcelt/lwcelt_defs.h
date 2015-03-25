#include "../common/lwmovie_config.h"

#ifndef HAVE_CONFIG_H
	#define HAVE_CONFIG_H
#endif

#if defined(LWMOVIE_SSE) && !defined(__SSE__)
	#define __SSE__
#endif

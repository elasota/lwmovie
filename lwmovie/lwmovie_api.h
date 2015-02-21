#ifndef __LWMOVIE_API_HPP__
#define __LWMOVIE_API_HPP__

#include "../common/lwmovie_config.h"

#ifdef LWMOVIE_DLL
	#ifdef LWMOVIE_DLL_EXPORT
		#define LWMOVIE_API_DLL __declspec(dllexport)
	#else
		#define LWMOVIE_API_DLL __declspec(dllimport)
	#endif
#else
	#define LWMOVIE_API_DLL
#endif

#ifdef __cplusplus

#define LWMOVIE_API_CLASS	class
#define LWMOVIE_API_LINK	extern "C" LWMOVIE_API_DLL

#else // !__cplusplus

#define LWMOVIE_API_CLASS	struct
#define LWMOVIE_API_LINK	LWMOVIE_API_DLL

#endif // !__cplusplus

#endif

#ifndef __LWMOVIE_API_HPP__
#define __LWMOVIE_API_HPP__

#include "../common/lwmovie_config.h"

#ifdef __GNUC__
	#ifdef lwmovie_EXPORTS
		#ifdef __CYGWIN__
			#define LWMOVIE_API_DLL __declspec(dllexport)
		#else
			#define LWMOVIE_API_DLL __attribute__ ((visibility ("default")))
		#endif
	#else
		#ifdef __CYGWIN__
			#define LWMOVIE_API_DLL __declspec(dllimport)
		#else
			#define LWMOVIE_API_DLL
		#endif
	#endif
#endif

#ifdef _MSC_VER
	#ifdef LWMOVIE_DLL
		#ifdef LWMOVIE_DLL_EXPORT
			#define LWMOVIE_API_DLL __declspec(dllexport)
		#else
			#define LWMOVIE_API_DLL __declspec(dllimport)
		#endif
	#else
		#define LWMOVIE_API_DLL
	#endif
#endif

#ifdef __cplusplus

#define LWMOVIE_API_CLASS	class
#define LWMOVIE_API_LINK	extern "C" LWMOVIE_API_DLL

#else // !__cplusplus

#define LWMOVIE_API_CLASS	struct
#define LWMOVIE_API_LINK	LWMOVIE_API_DLL

#endif // !__cplusplus

#endif

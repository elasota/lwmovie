#ifndef __LWMOVIE_API_HPP__
#define __LWMOVIE_API_HPP__

#ifdef __cplusplus

#define LWMOVIE_API_CLASS	class
#define LWMOVIE_API_LINK	extern "C"

#else // __cplusplus

#define LWMOVIE_API_CLASS	struct
#define LWMOVIE_API_LINK

#endif // __cplusplus

#endif

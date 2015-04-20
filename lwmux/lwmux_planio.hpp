#ifndef __LWMUX_PLANIO_HPP__
#define __LWMUX_PLANIO_HPP__

#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"

#include <string.h>

template<class T>
inline void lwmWritePlanToFile(const T &input, lwmOSFile *osFile)
{
	lwmUInt8 buffer[lwmPlanHandler<T>::SIZE];
	lwmPlanHandler<T>::Write(input, buffer);
	osFile->WriteBytes(buffer, lwmPlanHandler<T>::SIZE);
}

template<class T>
inline bool lwmReadPlanFromFile(T &input, lwmOSFile *osFile)
{
	lwmUInt8 buffer[lwmPlanHandler<T>::SIZE];
	if(osFile->ReadBytes(buffer, lwmPlanHandler<T>::SIZE) != lwmPlanHandler<T>::SIZE)
		return false;
	return lwmPlanHandler<T>::Read(input, buffer);
}

inline void EncodeMetaID(const char *metaID, lwmUInt32 metaIDChunks[2])
{
	size_t len = strlen(metaID);
	if(len > 8)
		len = 8;
	metaIDChunks[0] = metaIDChunks[1] = 0;
	for (size_t i = 0; i < len; i++)
		metaIDChunks[i / 4] |= static_cast<lwmUInt8>(metaID[i]) << ((i % 4) * 8);
}

#endif

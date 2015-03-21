#ifndef __LWMUX_PLANIO_HPP__
#define __LWMUX_PLANIO_HPP__

#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"

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

#endif

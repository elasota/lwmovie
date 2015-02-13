#ifndef __LWMOVIE_SIMDINT_HPP__
#define __LWMOVIE_SIMDINT_HPP__

#include "lwmovie_layer2.hpp"

template<class TSource>
class lwmSimdInt16
{
};

template<>
class lwmSimdInt16<lwmSInt32>
{
public:
	lwmSimdInt16(lwmSInt32 a, lwmSInt32 b);
	lwmSInt16 GetSimdSub(int index) const;
private:
	lwmSInt16 m_data[2];
};

inline lwmSimdInt16<lwmSInt32>::lwmSimdInt16(lwmSInt32 a, lwmSInt32 b)
{
	m_data[0] = a;
	m_data[1] = b;
}

inline lwmSInt16 lwmSimdInt16<lwmSInt32>::GetSimdSub(int index) const
{
	return m_data[index];
}

#endif

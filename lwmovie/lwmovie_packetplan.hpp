/*
 * Copyright (c) 2014 Eric Lasota
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __LWMOVIE_PACKETPLAN_HPP__
#define __LWMOVIE_PACKETPLAN_HPP__

#include "../common/lwmovie_coretypes.h"

template<class T> struct lwmPlanHandler { };
template<class T> struct lwmPlanMemberHandler { };

#define LWM_DECLARE_SIMPLE_PLAN_HANDLER(type)	\
	template<>\
	struct lwmPlanHandler<type>\
	{\
		enum\
		{\
			SIZE = sizeof(type),\
		};\
		static bool Read(type &output, const void *buffer);\
		static void Write(const type &input, void *buffer);\
		static bool ReadNonZero(type &output, const void *buffer);\
		static void WriteNonZero(const type &input, void *buffer);\
	}

LWM_DECLARE_SIMPLE_PLAN_HANDLER(lwmUInt8);
LWM_DECLARE_SIMPLE_PLAN_HANDLER(lwmUInt16);
LWM_DECLARE_SIMPLE_PLAN_HANDLER(lwmUInt32);
LWM_DECLARE_SIMPLE_PLAN_HANDLER(lwmUInt64);

// This handles out-of-range members
template<class TPlanType, int TIndex>
class lwmComplexPlanMember
{
public:
	enum
	{
		SIZE = 0,
		IS_PLAN_OCCUPIED = 0,\
	};

	static inline bool Read(TPlanType &output, const void *buffer)
	{
		return true;
	}

	static inline void Write(const TPlanType &input, void *buffer)
	{
	}
};

#define LWM_DECLARE_PLAN_MEMBER_NZENUM(index, TPlanType, TMemberType, member, memberSizeEnum, readFunc, writeFunc)	\
template<>\
class lwmComplexPlanMember<TPlanType, index>\
{\
private:\
	enum\
	{\
		MEMBER_SIZE = lwmPlanHandler<TMemberType>::memberSizeEnum\
	};\
public:\
	enum\
	{\
		SIZE = MEMBER_SIZE + lwmComplexPlanMember<TPlanType, (index-1)>::SIZE,\
		IS_PLAN_OCCUPIED = 1,\
	};\
	inline static bool Read(TPlanType &output, const void *buffer)\
	{\
		if(!lwmComplexPlanMember<TPlanType, (index-1)>::Read(output, buffer))\
			return false;\
		return lwmPlanHandler<TMemberType>::readFunc(output.member, reinterpret_cast<const lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE);\
	}\
	inline static void Write(const TPlanType &input, void *buffer)\
	{\
		lwmComplexPlanMember<TPlanType, (index-1)>::Write(input, buffer);\
		lwmPlanHandler<TMemberType>::writeFunc(input.member, reinterpret_cast<lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE);\
	}\
};


#define LWM_DECLARE_PLAN_ENUM_MEMBER(index, TPlanType, TMemberType, TMaxExclusive, TBackingType, member)	\
template<>\
class lwmComplexPlanMember<TPlanType, index>\
{\
private:\
	enum\
	{\
		MEMBER_SIZE = lwmPlanHandler<TBackingType>::SIZE\
	};\
public:\
	enum\
	{\
		SIZE = MEMBER_SIZE + lwmComplexPlanMember<TPlanType, (index-1)>::SIZE,\
		IS_PLAN_OCCUPIED = 1,\
	};\
	inline static bool Read(TPlanType &output, const void *buffer)\
	{\
		if(!lwmComplexPlanMember<TPlanType, (index-1)>::Read(output, buffer))\
			return false;\
		TBackingType value;\
		if(!lwmPlanHandler<TBackingType>::ReadNonZero(value, reinterpret_cast<const lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE))\
			return false;\
		if(value >= TMaxExclusive)\
			return false;\
		TMemberType &outRef = output.member;\
		outRef = static_cast<TMemberType>(value);\
		return true;\
	}\
	inline static void Write(const TPlanType &input, void *buffer)\
	{\
		lwmComplexPlanMember<TPlanType, (index-1)>::Write(input, buffer);\
		TBackingType value = static_cast<TBackingType>(input.member);\
		lwmPlanHandler<TBackingType>::WriteNonZero(value, reinterpret_cast<lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE);\
	}\
};


#define LWM_DECLARE_PLAN_MEMBER(index, TPlanType, TMemberType, member)	\
	LWM_DECLARE_PLAN_MEMBER_NZENUM(index, TPlanType, TMemberType, member, SIZE, Read, Write)

#define LWM_DECLARE_PLAN_MEMBER_NONZERO(index, TPlanType, TMemberType, member)	\
	LWM_DECLARE_PLAN_MEMBER_NZENUM(index, TPlanType, TMemberType, member, SIZE, ReadNonZero, WriteNonZero)

template<class TPlanType, int TIndex, bool TClimbHigher>
class lwmPlanMemberClimber
{
public:
	enum
	{
		HIGH_INDEX = lwmPlanMemberClimber<TPlanType, TIndex+1, (lwmComplexPlanMember<TPlanType, TIndex+1>::IS_PLAN_OCCUPIED == 1)>::HIGH_INDEX
	};
};

template<class TPlanType, int TIndex>
class lwmPlanMemberClimber<TPlanType, TIndex, false>
{
public:
	enum
	{
		HIGH_INDEX = TIndex
	};
};

#define LWM_DECLARE_PLAN_SENTINEL_NZENUM(index, TPlanType, TSentinelType, expectedValue, memberSizeEnum, readFunc, writeFunc)	\
template<>\
class lwmComplexPlanMember<TPlanType, index>\
{\
private:\
	enum\
	{\
		MEMBER_SIZE = lwmPlanHandler<TSentinelType>::memberSizeEnum\
	};\
public:\
	enum\
	{\
		SIZE = MEMBER_SIZE + lwmComplexPlanMember<TPlanType, (index-1)>::SIZE,\
		IS_PLAN_OCCUPIED = 1,\
	};\
	inline static bool Read(TPlanType &output, const void *buffer)\
	{\
		if(!lwmComplexPlanMember<TPlanType, (index-1)>::Read(output, buffer))\
			return false;\
		TSentinelType v;\
		if(!lwmPlanHandler<TSentinelType>::readFunc(v, reinterpret_cast<const lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE))\
			return false;\
		return (v == expectedValue);\
	}\
	inline static void Write(const TPlanType &input, void *buffer)\
	{\
		lwmComplexPlanMember<TPlanType, (index-1)>::Write(input, buffer);\
		lwmPlanHandler<TSentinelType>::writeFunc(expectedValue, reinterpret_cast<lwmUInt8*>(buffer) + SIZE - MEMBER_SIZE);\
	}\
};

#define LWM_DECLARE_PLAN_SENTINEL(index, TPlanType, TSentinelType, expectedValue)	\
	LWM_DECLARE_PLAN_SENTINEL_NZENUM(index, TPlanType, TSentinelType, expectedValue, SIZE, Read, Write)

#define LWM_DECLARE_PLAN_SENTINEL_NONZERO(index, TPlanType, TSentinelType, expectedValue)	\
	LWM_DECLARE_PLAN_SENTINEL_NZENUM(index, TPlanType, TSentinelType, expectedValue, SIZE, ReadNonZero, WriteNonZero)

#define LWM_DECLARE_PLAN(TPlanType)	\
	template<>\
	struct lwmPlanHandler<TPlanType>\
	{\
	private:\
		enum\
		{\
			HIGH_MEMBER_INDEX = lwmPlanMemberClimber<TPlanType, 0, true>::HIGH_INDEX - 1\
		};\
	public:\
		enum\
		{\
			SIZE = lwmComplexPlanMember<TPlanType, HIGH_MEMBER_INDEX>::SIZE\
		};\
		inline static bool Read(TPlanType &output, const void *buffer)\
		{\
			return lwmComplexPlanMember<TPlanType, HIGH_MEMBER_INDEX>::Read(output, buffer);\
		}\
		inline static void Write(const TPlanType &input, void *buffer)\
		{\
			lwmComplexPlanMember<TPlanType, HIGH_MEMBER_INDEX>::Write(input, buffer);\
		}\
	}

// Implementations
template<class T>
inline bool lwmReadUInt(T &output, const void *buffer, bool allowZero)
{
	T result;
	result = 0;
	for(lwmLargeUInt i=0;i<sizeof(T);i++)
		result |= static_cast<T>(static_cast<T>(static_cast<const lwmUInt8 *>(buffer)[i]) << ((sizeof(T) - 1 - i) * 8));
	if(!allowZero && result == 0)
		return false;
	output = result;
	return true;
}

template<class T>
inline void lwmWriteUInt(const T &input, void *buffer)
{
	T v = input;
	for(lwmLargeUInt i=0;i<sizeof(T);i++)
		static_cast<lwmUInt8 *>(buffer)[i] = static_cast<lwmUInt8>((v >> ((sizeof(T) - 1 - i) * 8)) & 0xff);
}


#define LWM_DECLARE_HANDLER_UINT(type)	\
inline bool lwmPlanHandler<type>::Read(type &output, const void *buffer)\
{\
	return lwmReadUInt(output, buffer, true);\
}\
inline bool lwmPlanHandler<type>::ReadNonZero(type &output, const void *buffer)\
{\
	return lwmReadUInt(output, buffer, false);\
}\
inline void lwmPlanHandler<type>::Write(const type &input, void *buffer)\
{\
	lwmWriteUInt(input, buffer);\
}\
inline void lwmPlanHandler<type>::WriteNonZero(const type &input, void *buffer)\
{\
	lwmWriteUInt(input, buffer);\
}\

LWM_DECLARE_HANDLER_UINT(lwmUInt8)
LWM_DECLARE_HANDLER_UINT(lwmUInt16)
LWM_DECLARE_HANDLER_UINT(lwmUInt32)
LWM_DECLARE_HANDLER_UINT(lwmUInt64)

#endif

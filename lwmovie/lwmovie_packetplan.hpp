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
			SIZE_NON_ZERO = sizeof(type) + 1\
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
		MEMBER_SIZE = lwmPlanHandler<TBackingType>::SIZE_NON_ZERO\
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
	LWM_DECLARE_PLAN_MEMBER_NZENUM(index, TPlanType, TMemberType, member, SIZE_NON_ZERO, ReadNonZero, WriteNonZero)

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
	LWM_DECLARE_PLAN_SENTINEL_NZENUM(index, TPlanType, TSentinelType, expectedValue, SIZE_NON_ZERO, ReadNonZero, WriteNonZero)

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
inline bool lwmReadUIntTrivial(T &output, const void *buffer, bool allowZero)
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
inline void lwmWriteUIntTrivial(const T &input, void *buffer)
{
	T v = input;
	for(lwmLargeUInt i=0;i<sizeof(T);i++)
		static_cast<lwmUInt8 *>(buffer)[i] = static_cast<lwmUInt8>((v >> ((sizeof(T) - 1 - i) * 8)) & 0xff);
}

template<class T>
inline bool lwmReadUIntFragmented(T &output, const void *buffer, bool allowZero)
{
	const lwmUInt8 *splitter = static_cast<const lwmUInt8*>(buffer);
	if((splitter[0] & 0xc0) != 0xc0 ||
		(splitter[sizeof(T)/2] & 0x3c) != 0x3c ||
		(splitter[sizeof(T)] & 0x3) != 0x3)
		return false;
	lwmUInt32 highFragment = static_cast<lwmUInt32>(splitter[0]) << (sizeof(T)*4 - 6);
	for(int i=1;i<sizeof(T)/2;i++)
		highFragment |= static_cast<lwmUInt32>(splitter[i]) << (sizeof(T)*4 - 6 - i*8);
	highFragment |= static_cast<lwmUInt32>(splitter[sizeof(T)/2] & 0xc0) >> 6;

	lwmUInt32 lowFragment = static_cast<lwmUInt32>(splitter[sizeof(T)/2] & 0x3) << (sizeof(T)*4 - 2);
	for(int i=sizeof(T)/2+1;i<sizeof(T);i++)
		lowFragment |= static_cast<lwmUInt32>(splitter[i]) << ((sizeof(T) - i)*8 - 2);
	lowFragment |= static_cast<lwmUInt32>(splitter[sizeof(T)]) >> 2;

	T outBuilder;
	outBuilder = static_cast<T>(highFragment) << sizeof(T)*4;
	outBuilder |= static_cast<T>(lowFragment);

	if(!allowZero && outBuilder == 0)
		return false;

	output = outBuilder;

	return true;
}

template<>
inline bool lwmReadUIntFragmented<lwmUInt8>(lwmUInt8 &output, const void *buffer, bool allowZero)
{
	const lwmUInt8 *splitter = static_cast<const lwmUInt8*>(buffer);
	if((splitter[0] & 0xc3) != 0xc3 ||
		(splitter[1] & 0xc3) != 0xc3)
		return false;
	lwmUInt8 outBuilder = static_cast<lwmUInt8>(((splitter[1] >> 2) & 0xf) | ((splitter[0] << 2) & 0xf0));
	if(!allowZero && outBuilder == 0)
		return false;

	output = outBuilder;

	return true;
}

template<class T>
void lwmWriteUIntFragmented(const T &input, void *buffer)
{
	lwmUInt8 splitTemp[sizeof(T)+1];
	for(int i=0;i<sizeof(T)+1;i++)
		splitTemp[i] = 0;
	lwmUInt32 highFragment = static_cast<lwmUInt32>(input >> sizeof(T)*4);
	lwmUInt32 mask = (((static_cast<lwmUInt32>(1) << (sizeof(T)*4 - 1)) - 1) * 2) + 1;
	lwmUInt32 lowFragment = static_cast<lwmUInt32>(input) & mask;

	for(int i=0;i<sizeof(T)/2;i++)
		splitTemp[i] |= highFragment >> (sizeof(T)*4 - i*8 - 6);
	splitTemp[sizeof(T)/2] |= (highFragment << 6);
	for(int i=sizeof(T)/2;i<sizeof(T);i++)
		splitTemp[i] |= lowFragment >> ((sizeof(T) - i)*8 - 2);
	splitTemp[sizeof(T)] |= lowFragment << 2;
	splitTemp[0] |= 0xc0;
	splitTemp[sizeof(T)/2] |= 0x3c;
	splitTemp[sizeof(T)] |= 0x3;

	for(int i=0;i<sizeof(T)+1;i++)
		static_cast<lwmUInt8*>(buffer)[i] = splitTemp[i];
}

template<>
inline void lwmWriteUIntFragmented<lwmUInt8>(const lwmUInt8 &input, void *buffer)
{
	lwmUInt8 *out = static_cast<lwmUInt8*>(buffer);
	out[0] = static_cast<lwmUInt8>(0xc3 | (input >> 2));
	out[1] = static_cast<lwmUInt8>(0xc3 | (input << 2));
}


#define LWM_DECLARE_HANDLER_UINT_TRIVIAL(type)	\
inline bool lwmPlanHandler<type>::Read(type &output, const void *buffer)\
{\
	return lwmReadUIntTrivial(output, buffer, true);\
}\
inline bool lwmPlanHandler<type>::ReadNonZero(type &output, const void *buffer)\
{\
	return lwmReadUIntFragmented(output, buffer, false);\
}\
inline void lwmPlanHandler<type>::Write(const type &input, void *buffer)\
{\
	lwmWriteUIntTrivial(input, buffer);\
}\
inline void lwmPlanHandler<type>::WriteNonZero(const type &input, void *buffer)\
{\
	lwmWriteUIntTrivial(input, buffer);\
}\


#define LWM_DECLARE_HANDLER_UINT_FRAGMENTED(type)	\
inline bool lwmPlanHandler<type>::Read(type &output, const void *buffer)\
{\
	return lwmReadUIntTrivial(output, buffer, true);\
}\
inline bool lwmPlanHandler<type>::ReadNonZero(type &output, const void *buffer)\
{\
	return lwmReadUIntFragmented(output, buffer, false);\
}\
inline void lwmPlanHandler<type>::Write(const type &input, void *buffer)\
{\
	lwmWriteUIntTrivial(input, buffer);\
}\
inline void lwmPlanHandler<type>::WriteNonZero(const type &input, void *buffer)\
{\
	lwmWriteUIntFragmented(input, buffer);\
}\

LWM_DECLARE_HANDLER_UINT_FRAGMENTED(lwmUInt8)
LWM_DECLARE_HANDLER_UINT_FRAGMENTED(lwmUInt16)
LWM_DECLARE_HANDLER_UINT_FRAGMENTED(lwmUInt32)
LWM_DECLARE_HANDLER_UINT_FRAGMENTED(lwmUInt64)

#endif

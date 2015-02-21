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
#ifndef __LWMOVIE_PROFILE_HPP__
#define __LWMOVIE_PROFILE_HPP__

#include "../common/lwmovie_coretypes.h"

class lwmCProfileTagSet;

enum lwmEProfileTag
{
	lwmEPROFILETAG_Deslice,
	lwmEPROFILETAG_ReconRow,
	lwmEPROFILETAG_Motion,
	lwmEPROFILETAG_IDCTSparse,
	lwmEPROFILETAG_IDCTFull,
	lwmEPROFILETAG_ParseBlock,
	lwmEPROFILETAG_ParseCoeffs,
	lwmEPROFILETAG_ParseCoeffsTest,
	lwmEPROFILETAG_ParseCoeffsIntra,
	lwmEPROFILETAG_ParseCoeffsInter,
	lwmEPROFILETAG_ParseCoeffsCommit,

	lwmEPROFILETAG_Count,
};

#ifdef LWMOVIE_PROFILE


class lwmCProfileTag
{
public:
	lwmCProfileTag();

	void AddTime(lwmUInt32 t);
	double GetTotalTime() const;
	lwmUInt32 GetRawTime() const;
	void Reset();
private:
	lwmUInt32 m_time;
};


class lwmCAutoProfile
{
public:
	lwmCAutoProfile(lwmCProfileTagSet *tagSet, lwmEProfileTag profileTagIndex);
	~lwmCAutoProfile();

private:
	lwmCProfileTag *m_tag;
	lwmUInt32 m_baseTime;
};

class lwmCProfileTagSet
{
private:
	lwmCProfileTag m_profileTags[lwmEPROFILETAG_Count];

public:
	inline lwmCProfileTag *GetTag(lwmEProfileTag tag)
	{
		return m_profileTags + tag;
	}

	void FlushTo(lwmCProfileTagSet *otherTagSet);
};

#include "lwmovie_profile_win32.inl"

#else	// LWMOVIE_PROFILE

class lwmCAutoProfile
{
public:
	inline lwmCAutoProfile(lwmCProfileTagSet *tagSet, lwmEProfileTag profileTagIndex) { }
};

#endif	// LWMOVIE_PROFILE

#endif

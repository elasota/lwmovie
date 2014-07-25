#ifndef __LWMOVIE_PROFILE_HPP__
#define __LWMOVIE_PROFILE_HPP__

#include "lwmovie_types.hpp"

#define LWMOVIE_DEEP_PROFILE

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

#endif


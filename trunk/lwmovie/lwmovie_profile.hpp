#ifndef __LWMOVIE_PROFILE_HPP__
#define __LWMOVIE_PROFILE_HPP__

#include "lwmovie_types.hpp"

class lwmCProfileTag
{
public:
	lwmCProfileTag();

	void AddTime(lwmUInt32 t);
	double GetTotalTime() const;
private:
	lwmUInt32 m_time;
};

class lwmCAutoProfile
{
public:
	explicit lwmCAutoProfile(lwmCProfileTag &tag);
	~lwmCAutoProfile();

private:
	lwmCProfileTag *m_tag;
	lwmUInt32 m_baseTime;
};

extern lwmCProfileTag g_ptDeslice;
extern lwmCProfileTag g_ptReconRow;
extern lwmCProfileTag g_ptMotion;
extern lwmCProfileTag g_ptIDCTSparse;
extern lwmCProfileTag g_ptIDCTFull;
extern lwmCProfileTag g_ptParseBlock;
extern lwmCProfileTag g_ptParseCoeffs;
extern lwmCProfileTag g_ptParseCoeffsTest;
extern lwmCProfileTag g_ptParseCoeffsIntra;
extern lwmCProfileTag g_ptParseCoeffsInter;
extern lwmCProfileTag g_ptParseCoeffsCommit;

#endif


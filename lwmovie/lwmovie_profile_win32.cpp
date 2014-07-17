//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "lwmovie_profile.hpp"

lwmCProfileTag::lwmCProfileTag()
{
	m_time = 0;
}


void lwmCProfileTag::AddTime(lwmUInt32 t)
{
	m_time += t;
}

double lwmCProfileTag::GetTotalTime() const
{
	//LARGE_INTEGER li;
	//QueryPerformanceFrequency(&li);
	//return static_cast<double>(m_time) / static_cast<double>(li.QuadPart);
	return static_cast<double>(m_time) / 1000.0;
}

lwmCAutoProfile::lwmCAutoProfile(lwmCProfileTag &tag)
{
	m_tag = &tag;
	//LARGE_INTEGER li;
	//QueryPerformanceCounter(&li);
	//m_baseTime = static_cast<lwmUInt64>(li.QuadPart);
	m_baseTime = timeGetTime();
}

lwmCAutoProfile::~lwmCAutoProfile()
{
	//LARGE_INTEGER li;
	//QueryPerformanceCounter(&li);
	//timeGetTime(
	//m_tag->AddTime(static_cast<lwmUInt64>(li.QuadPart) - m_baseTime);
	m_tag->AddTime(timeGetTime() - m_baseTime);
}


lwmCProfileTag g_ptDeslice;
lwmCProfileTag g_ptMotion;
lwmCProfileTag g_ptIDCTSparse;
lwmCProfileTag g_ptIDCTFull;
lwmCProfileTag g_ptReconRow;
lwmCProfileTag g_ptParseBlock;
lwmCProfileTag g_ptParseCoeffs;
lwmCProfileTag g_ptParseCoeffsTest;
lwmCProfileTag g_ptParseCoeffsIntra;
lwmCProfileTag g_ptParseCoeffsInter;
lwmCProfileTag g_ptParseCoeffsCommit;

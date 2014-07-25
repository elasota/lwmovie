#include <Windows.h>

__forceinline lwmCProfileTag::lwmCProfileTag()
{
	m_time = 0;
}

__forceinline void lwmCProfileTag::AddTime(lwmUInt32 t)
{
	m_time += t;
}

__forceinline double lwmCProfileTag::GetTotalTime() const
{
	return static_cast<double>(m_time) / 1000.0;
}

__forceinline lwmUInt32 lwmCProfileTag::GetRawTime() const
{
	return m_time;
}

__forceinline void lwmCProfileTag::Reset()
{
	m_time = 0;
}

__declspec(dllimport) unsigned long __stdcall GetTickCount(void);

__forceinline lwmCAutoProfile::lwmCAutoProfile(lwmCProfileTagSet *tagSet, lwmEProfileTag profileTagIndex)
{
	m_tag = tagSet->GetTag(profileTagIndex);
	m_baseTime = GetTickCount();
}

__forceinline lwmCAutoProfile::~lwmCAutoProfile()
{
	m_tag->AddTime(GetTickCount() - m_baseTime);
}


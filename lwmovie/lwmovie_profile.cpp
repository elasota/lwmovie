#include "lwmovie_profile.hpp"

void lwmCProfileTagSet::FlushTo(lwmCProfileTagSet *otherTagSet)
{
	for(int i=0;i<lwmEPROFILETAG_Count;i++)
	{
		otherTagSet->GetTag(static_cast<lwmEProfileTag>(i))->AddTime(m_profileTags[i].GetRawTime());
		m_profileTags[i].Reset();
	}
}

#ifndef __LWPLAY_THREADPOOL_HPP__
#define __LWPLAY_THREADPOOL_HPP__

#include "../lwmovie/lwmovie_cake.h"
#include "../lwmovie/lwmovie_cake_cppshims.hpp"

namespace lwplay
{
	class CWorkNotifierFactory : public lwmICakeWorkNotifierFactory
	{
	public:
		lwmSWorkNotifier *CreateWorkNotifier(void *obj, lwmCakeParticipateCallback participationCallback);
		void DestroyWorkNotifier(struct lwmSWorkNotifier *notifier);
	};
}


#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "threadpool.hpp"
#include "../lwmovie/lwmovie_cpp_shims.hpp"

namespace lwplay
{
	class CWin32WorkNotifier : public lwmIWorkNotifier
	{
	public:
		CWin32WorkNotifier(void *obj, lwmCakeParticipateCallback callback);
		void NotifyAvailable();
		void Join();

	private:
		void Work();
		static VOID NTAPI StaticWorkCB(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);

		void *m_obj;
		lwmCakeParticipateCallback m_callback;
		PTP_WORK m_work;
	};
};

lwplay::CWin32WorkNotifier::CWin32WorkNotifier(void *obj, lwmCakeParticipateCallback callback)
	: m_obj(obj)
	, m_callback(callback)
{
	m_work = CreateThreadpoolWork(StaticWorkCB, this, NULL);
}

void lwplay::CWin32WorkNotifier::NotifyAvailable()
{
	if(m_work)
		SubmitThreadpoolWork(m_work);
	else
		m_callback(m_obj);
}

void lwplay::CWin32WorkNotifier::Join()
{
	if(m_work)
		WaitForThreadpoolWorkCallbacks(m_work, FALSE);
}

void lwplay::CWin32WorkNotifier::Work()
{
	m_callback(m_obj);
}

VOID NTAPI lwplay::CWin32WorkNotifier::StaticWorkCB(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	static_cast<CWin32WorkNotifier*>(Context)->Work();
}

lwmSWorkNotifier *lwplay::CWorkNotifierFactory::CreateWorkNotifier(void *obj, lwmCakeParticipateCallback participationCallback)
{
	return new CWin32WorkNotifier(obj, participationCallback);
}

void lwplay::CWorkNotifierFactory::DestroyWorkNotifier(struct lwmSWorkNotifier *notifier)
{
	delete static_cast<CWin32WorkNotifier*>(notifier);
}

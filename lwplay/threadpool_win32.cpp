/*
* Copyright (c) 2015 Eric Lasota
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

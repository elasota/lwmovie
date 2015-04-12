#ifndef __LWMOVIE_CAKE_CPPSHIMS_HPP__
#define __LWMOVIE_CAKE_CPPSHIMS_HPP__

#include "lwmovie_cake.h"

struct lwmICakeAudioDevice : public lwmCakeAudioDevice
{
	virtual lwmLargeUInt QueueSamples(lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples) = 0;
	virtual bool UnderrunOccurred() const = 0;
	virtual void Reset() = 0;

private:
	static lwmLargeUInt StaticQueueSamples(lwmCakeAudioDevice *audioDevice, class lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples)
	{
		return static_cast<lwmICakeAudioDevice*>(audioDevice)->QueueSamples(cake, sources, numSources, numSamples);
	}

	static int StaticUnderrunOccurred(lwmCakeAudioDevice *audioDevice)
	{
		return static_cast<lwmICakeAudioDevice*>(audioDevice)->UnderrunOccurred();
	}

	static void StaticReset(lwmCakeAudioDevice *audioDevice)
	{
		static_cast<lwmICakeAudioDevice*>(audioDevice)->Reset();
	}

public:
	lwmICakeAudioDevice()
	{
		queueSamplesFunc = StaticQueueSamples;
		resetFunc = StaticReset;
		underrunOccurredFunc = StaticUnderrunOccurred;
	}
};


struct lwmICakeWorkNotifierFactory : public lwmCakeWorkNotifierFactory
{
public:
	virtual lwmSWorkNotifier *CreateWorkNotifier(void *obj, lwmCakeParticipateCallback participationCallback) = 0;
	virtual void DestroyWorkNotifier(struct lwmSWorkNotifier *notifier) = 0;
private:
	static lwmSWorkNotifier *StaticCreateWorkNotifier(struct lwmCakeWorkNotifierFactory *workNotifierFactory, void *obj, lwmCakeParticipateCallback participationCallback)
	{
		return static_cast<lwmICakeWorkNotifierFactory*>(workNotifierFactory)->CreateWorkNotifier(obj, participationCallback);
	}

	static void StaticDestroyWorkNotifier(struct lwmCakeWorkNotifierFactory *workNotifierFactory, struct lwmSWorkNotifier *notifier)
	{
		static_cast<lwmICakeWorkNotifierFactory*>(workNotifierFactory)->DestroyWorkNotifier(notifier);
	}

public:
	lwmICakeWorkNotifierFactory()
	{
		this->createWorkNotifierFunc = StaticCreateWorkNotifier;
		this->destroyWorkNotifierFunc = StaticDestroyWorkNotifier;
	}
};

#endif

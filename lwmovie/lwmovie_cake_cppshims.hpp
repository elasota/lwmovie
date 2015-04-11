#ifndef __LWMOVIE_CAKE_CPPSHIMS_HPP__
#define __LWMOVIE_CAKE_CPPSHIMS_HPP__

#include "lwmovie_cake.h"

struct lwmICakeAudioDevice : public lwmCakeAudioDevice
{
	virtual lwmLargeUInt QueueSamples(lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples) = 0;
	virtual bool UnderrunOccurred() const = 0;
	virtual void Reset() = 0;

private:
	static lwmLargeUInt StaticQueueSamples(lwmCakeAudioDevice *soundDevice, class lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples)
	{
		return static_cast<lwmICakeAudioDevice*>(soundDevice)->QueueSamples(cake, sources, numSources, numSamples);
	}

	static int StaticUnderrunOccurred(lwmCakeAudioDevice *soundDevice)
	{
		return static_cast<lwmICakeAudioDevice*>(soundDevice)->UnderrunOccurred();
	}

	static void StaticReset(lwmCakeAudioDevice *soundDevice)
	{
		static_cast<lwmICakeAudioDevice*>(soundDevice)->Reset();
	}

public:
	lwmICakeAudioDevice()
	{
		queueSamplesFunc = StaticQueueSamples;
		resetFunc = StaticReset;
		underrunOccurredFunc = StaticUnderrunOccurred;
	}
};

#endif

/*
* Copyright (c) 2014 Eric Lasota
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
#ifndef __LWMOVIE_CAKE_CPPSHIMS_HPP__
#define __LWMOVIE_CAKE_CPPSHIMS_HPP__

#include "lwmovie_cake.h"

struct lwmICakeFileReader : public lwmCakeFileReader
{
	virtual bool IsEOF() = 0;
	virtual lwmLargeUInt ReadBytes(void *dest, lwmLargeUInt numBytes) = 0;

private:
	static int StaticIsEOF(lwmCakeFileReader *fileReader)
	{
		return static_cast<lwmICakeFileReader*>(fileReader)->IsEOF() ? 1 : 0;
	}

	static lwmLargeUInt StaticReadBytes(lwmCakeFileReader *fileReader, void *dest, lwmLargeUInt numBytes)
	{
		return static_cast<lwmICakeFileReader*>(fileReader)->ReadBytes(dest, numBytes);
	}

public:
	lwmICakeFileReader()
	{
		this->isEOFFunc = StaticIsEOF;
		this->readBytesFunc = StaticReadBytes;
	}
};

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

struct lwmICakeTimeReader : public lwmCakeTimeReader
{
public:
	virtual lwmUInt64 GetTimeMilliseconds() = 0;
	virtual lwmUInt32 GetResolutionMilliseconds() = 0;

private:
	static lwmUInt64 StaticGetTimeMilliseconds(struct lwmCakeTimeReader *timeReader)
	{
		return static_cast<lwmICakeTimeReader*>(timeReader)->GetTimeMilliseconds();
	}

	static lwmUInt32 StaticGetResolutionMilliseconds(struct lwmCakeTimeReader *timeReader)
	{
		return static_cast<lwmICakeTimeReader*>(timeReader)->GetResolutionMilliseconds();
	}

public:
	lwmICakeTimeReader()
	{
		this->getTimeMillisecondsFunc = StaticGetTimeMilliseconds;
		this->getResolutionMillisecondsFunc = StaticGetResolutionMilliseconds;
	}
};

#endif

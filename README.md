# lwmovie

lwmovie is an audio/video decoding library intended for use by games, embedded systems, and other multimedia applications.

The primary goal of the project is easy implementation, with a simple API, patent-free codecs, and a very permissive license, while also being flexible enough to accomodate for a variety of use cases.

Some of its advantages:

###Flexible:###
- Frame provider interface for direct processing to texture memory.
- Reconstructor interface for hardware-accelerated DSP.
- Integrates with host application thread pooling.
- Uses a Zlib-like streaming IO model with readahead hints.
- Notification-based sound synchronization.

###Powerful:###
- SSE, SSE2, and SSE 4.1 optimized DSP.
- Heavily threadable, almost all of the decode process can be executed in parallel and scales very well with thread count.
- Uses established codecs with existing high-quality encoders (MPEG video, CELT audio, and MPEG Layer II audio).

###Permissive:###
- All components are under very permissive MIT/BSD-like licenses.
- Uses only codecs that are free of any known patent restrictions or are under royalty-free licenses.

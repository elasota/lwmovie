A portable, speed-oriented, embeddable audio/video playback library.  Intended for use by games, embedded systems, and other applications that demand low implementation complexity.



High-performance:
  * SIMD-optimized DSP.
  * Heavily threadable, nearly the entire decode process scales very well with CPU count.

Powerful:
  * Standardized input formats allow established, high-quality encoders to be used.
  * Supports MPEG-1 video, MPEG-1 Layer II audio, CELT audio, and 4-bit ADPCM audio.

Permissive:
  * All components are under very permissive MIT/BSD-like licenses.
  * Uses only codecs that are free of any patent restrictions.

Modular:
  * Frame provider interface for integrating direct processing to texture memory.
  * Reconstructor interface for hardware-accelerated DSP.
  * Integrates with host application thread pooling.
  * No I/O callbacks, uses a Zlib-like "stream" API with readahead hints.
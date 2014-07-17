/*
Copyright (c) 2012 Eric Lasota

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __MP2DEC_H__
#define __MP2DEC_H__

#include "config.h"
#include "fixedpoint.h"

#ifdef __cplusplus
#       define MP2DEC_LINKAGE MP2DEC_CPP_LINKAGE
#else
#       define MP2DEC_LINKAGE MP2DEC_C_LINKAGE
#endif

#ifndef MP2DEC_FLOATINGPOINT
        typedef struct MP2Dec_Fixed32Base AbstractReal;
#else
        typedef float AbstractReal;
#endif

#define MP2DEC_MAX_FRAME_SIZE_BYTES     4800            // Maximum size of a frame in bytes, including header
#define MP2DEC_HEADER_SIZE_BYTES        4                       // Size of a frame header in bytes
#define MP2DEC_FRAME_NUM_SAMPLES        1152

#define MP2DEC_TRUE             1
#define MP2DEC_FALSE    0

typedef struct mp2dec_decode_properties
{
        char calcBuffersUnaligned[sizeof(AbstractReal)*2*2*16*32 + MP2DEC_SIMD_SIZE];
        unsigned int cbAlignmentOffset;

        mp2dec_uint     frameBytes;                             // Number of bytes requested for frame data
        mp2dec_uint     tableIndex;
        mp2dec_uint numSubBands;
        mp2dec_uint stereoBound;
        mp2dec_bool inputStereo;
        mp2dec_uint sampleRate;

        int currentcalcbuffers[2];
        int calcbufferoffsets[2];
} mp2dec_decode_properties;

typedef struct mp2dec_header
{
        mp2dec_uint16   syncword;                               // 12 bits
        mp2dec_uint8    version;                                // 1 bit
        mp2dec_uint8    layer;                                  // 2 bits
        mp2dec_uint8    disableCRCFlag;                 // 1 bit
        mp2dec_uint8    bitrateIndex;                   // 4 bits
        mp2dec_uint8    samplerateIndex;                // 2 bits
        mp2dec_uint8    padding;                                // 1 bit
        mp2dec_uint8    privateFlag;                    // 1 bit
        mp2dec_uint8    mode;                                   // 2 bits
        mp2dec_uint8    modeExt;                                // 2 bits
        mp2dec_uint8    copyrightFlag;                  // 1 bit
        mp2dec_uint8    originalFlag;                   // 1 bit
        mp2dec_uint8    emphasis;                               // 2 bits
} mp2dec_header;


MP2DEC_LINKAGE void mp2dec_init_decode_state(mp2dec_decode_properties *decodeProps);
MP2DEC_LINKAGE mp2dec_bool mp2dec_parse_header(const void *inBytes, mp2dec_header *outHeader, mp2dec_decode_properties *outDecodeProps);
MP2DEC_LINKAGE void mp2dec_decode_frame(const mp2dec_header *header, mp2dec_decode_properties *decodeProps,
        const void *frameData, mp2dec_sint16 output[2][MP2DEC_FRAME_NUM_SAMPLES]);
MP2DEC_LINKAGE void mp2dec_mix(const mp2dec_sint16 input[2][MP2DEC_FRAME_NUM_SAMPLES], mp2dec_sint16 mixed[MP2DEC_FRAME_NUM_SAMPLES*2]);

#endif


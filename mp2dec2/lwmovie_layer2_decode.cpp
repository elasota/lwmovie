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
#include <stdio.h>

#include "lwmovie_layer2_decodestate.hpp"
#include "lwmovie_layer2_bitstream.hpp"

FILE *debugFile2;

namespace layerii
{
}

lwmovie::layerii::lwmCMP2DecodeState::lwmCMP2DecodeState()
{
	for(int i=0;i<MAX_CHANNELS;i++)
		for(int j=0;j<FILTER_SIZE;j++)
			for(int k=0;k<NUM_SUBBANDS;k++)
				m_accumulators[i][j][k] = lwmFixedReal22(0.0);
	m_sbsRotator = 0;
}

lwmUInt16 lwmovie::layerii::lwmCMP2DecodeState::GetFrameSizeBytes() const
{
	// 1152 * 1000 / 8
	// totalBitrate * 1000 * 1152 / sampleRate
	lwmUInt32 frameBytes = (static_cast<lwmUInt32>(m_bitrateKbps) * (1000*1152/8) / m_sampleRate) - 4;
	if(m_hasPadding)
		frameBytes++;
	if(m_hasChecksum)
		frameBytes += 2;
	return frameBytes;
}

bool lwmovie::layerii::lwmCMP2DecodeState::ParseHeader(const void *data)
{
	const lwmUInt8 *bytes = static_cast<const lwmUInt8 *>(data);
	lwmUInt32 header = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]);

	lwmFastUInt16 syncword = (header & 0xffe00000) >> 21;
	lwmFastUInt8 version = (header & 0x180000) >> 19;
	lwmFastUInt8 layer = (header & 0x60000) >> 17;
	lwmFastUInt8 protection = (header & 0x10000) >> 16;
	lwmFastUInt8 bitrateIndex = (header & 0xf000) >> 12;
	lwmFastUInt8 samplerateIndex = (header & 0xc00) >> 10;
	lwmFastUInt8 padding = (header & 0x200) >> 9;
	lwmFastUInt8 privateFlag = (header & 0x100) >> 8;
	lwmFastUInt8 mode = (header & 0xc0) >> 6;
	lwmFastUInt8 modeExt = (header & 0x30) >> 4;
	lwmFastUInt8 copyrightFlag = (header & 0x8) >> 3;
	lwmFastUInt8 originalFlag = (header & 0x4) >> 2;
	lwmFastUInt8 emphasis = (header & 0x3);

	if(syncword != 0x7ff		// Bad syncword
		|| version == 0			// MPEG 2.5, not supported
		|| version == 1			// Reserved
		|| layer != 2
		|| bitrateIndex == 0	// Free format, not supported
		|| bitrateIndex == 15	// Bad
		|| samplerateIndex == 3	// Reserved
		)
		return false;

	if(version == 2)
		m_mpegVersion = EMPEGVersion_MPEG2;
	else if(version == 3)
		m_mpegVersion = EMPEGVersion_MPEG1;

	m_bitrateKbps = MP2_BITRATE_KBPS[m_mpegVersion][bitrateIndex];
	m_bitrateIndex = bitrateIndex;
	if(mode != 3)
		m_numChannels = 2;
	else
		m_numChannels = 1;

	m_hasPadding = (padding != 0);
	m_hasChecksum = (protection == 0);
	m_isJointStereo = (mode == 1);
	m_modeExt = modeExt;
	m_samplerateIndex = samplerateIndex;
	m_sampleRate = MP2_SAMPLERATE[m_mpegVersion][samplerateIndex];

	return true;
}


template<lwmFastUInt8 numChannels>
bool lwmovie::layerii::lwmCMP2DecodeState::DecodeFrameCH(const void *bytes, lwmSInt16 *output)
{
	lwmCMemBitstream bitstream(bytes);

	lwmFastUInt8 tableSet = MP2_TABLESET[m_mpegVersion][numChannels-1][m_bitrateIndex];

	lwmFastUInt8 sbLimit;
	
	if(m_mpegVersion == EMPEGVersion_MPEG1)
		sbLimit = MP2_SBLIMITS_MPEG1[numChannels-1][m_samplerateIndex][m_bitrateIndex];
	else
		sbLimit = 30;

	lwmFastUInt8 jsLimit;
	if(numChannels == 1)
		jsLimit = 0;
	else if(m_isJointStereo)
		jsLimit = (m_modeExt + 1) * 4;
	else
		jsLimit = sbLimit;

	if(jsLimit > sbLimit)
		return false;	// Bad JS limit

	// Parse off CRC
	if(m_hasChecksum)
		bitstream.SkipBytes(2);

	// Read bitalloc
	lwmFastUInt8 bitAlloc[MAX_CHANNELS][NUM_SUBBANDS];
	for(lwmFastUInt8 i=0;i<sbLimit;i++)
	{
		lwmFastUInt8 bitAllocSize = MP2_BITALLOC_SIZE[tableSet][i];
		for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
		{
			if(ch == 0 || i < jsLimit)
				bitAlloc[ch][i] = static_cast<lwmFastUInt8>(bitstream.Read(bitAllocSize));
			else
				bitAlloc[ch][i] = bitAlloc[0][i];
		}
	}

	// Read scale selectors
	lwmFastUInt8 scaleSelector[MAX_CHANNELS][NUM_SUBBANDS];
	for(lwmFastUInt8 i=0;i<sbLimit;i++)
	{
		for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
		{
			if(bitAlloc[ch][i])
				scaleSelector[ch][i] = bitstream.Read(2);
		}
	}

	// Read scale factors
	lwmCompressedSF scaleFactors[MAX_CHANNELS][NUM_SUBBANDS][3];
	for(lwmFastUInt8 i=0;i<sbLimit;i++)
	{
		for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
		{
			if(bitAlloc[ch][i])
			{
				lwmUInt8 ss = scaleSelector[ch][i];

				switch(ss)
				{
				case 0:		// 0, 1, 2
					{
						lwmUInt32 bits = bitstream.Read(18);
						scaleFactors[ch][i][0] = MP2_SCALEFACTORS[(bits >> 12) & 0x3f];
						scaleFactors[ch][i][1] = MP2_SCALEFACTORS[(bits >> 6) & 0x3f];
						scaleFactors[ch][i][2] = MP2_SCALEFACTORS[bits & 0x3f];
					}
					break;
				case 1:		// 0+1, 2
					{
						lwmUInt32 bits = bitstream.Read(12);
						scaleFactors[ch][i][0] =
						scaleFactors[ch][i][1] = MP2_SCALEFACTORS[(bits >> 6) & 0x3f];
						scaleFactors[ch][i][2] = MP2_SCALEFACTORS[bits & 0x3f];
					}
					break;
				case 2:		// 0+1+2
					{
						lwmUInt32 bits = bitstream.Read(6);
						scaleFactors[ch][i][0] =
						scaleFactors[ch][i][1] =
						scaleFactors[ch][i][2] = MP2_SCALEFACTORS[bits];
					}
					break;
				case 3:		// 0, 1+2
					{
						lwmUInt32 bits = bitstream.Read(12);
						scaleFactors[ch][i][0] = MP2_SCALEFACTORS[(bits >> 6) & 0x3f];
						scaleFactors[ch][i][1] =
						scaleFactors[ch][i][2] = MP2_SCALEFACTORS[bits & 0x3f];
					}
					break;
				};
			}
		}
	}

	// Read sample
	// Zero unused coeffs
	lwmFixedReal14 coeffs[MAX_CHANNELS][3][NUM_SUBBANDS];
	for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
		for(lwmFastUInt8 vc=0;vc<3;vc++)
			for(lwmFastUInt8 i=sbLimit;i<NUM_SUBBANDS;i++)
				coeffs[ch][vc][i] = lwmFixedReal14(0);

	for(lwmFastUInt8 cluster=0;cluster<3;cluster++)
	{
		for(lwmFastUInt8 block=0;block<4;block++)
		{
			for(lwmFastUInt8 i=0;i<sbLimit;i++)
			{
				for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
				{
					if(ch == 0 || i < jsLimit)
					{
						if(bitAlloc[ch][i])
						{
							lwmFastUInt8 qindexTable = MP2_QINDEX_TABLE_SELECT[tableSet][i];
							lwmFastUInt8 qindex = MP2_QINDEX[qindexTable][bitAlloc[ch][i]];

							const MP2QuantInfo *quantInfo = MP2_QUANT_INFO + qindex;
							lwmUInt8 group = quantInfo->modGroup;
							lwmFastUInt8 codeLength = quantInfo->codeLength;

							lwmFixedReal29 rcp = quantInfo->rcp;
						
							lwmFastUInt16 baseValues[3];
							if(group)
							{
								lwmUInt16 groupedValue = static_cast<lwmUInt16>(bitstream.Read(codeLength));
								baseValues[0] = static_cast<lwmFastUInt16>(groupedValue % group);
								groupedValue /= group;
								baseValues[1] = static_cast<lwmFastUInt16>(groupedValue % group);
								groupedValue /= group;
								baseValues[2] = static_cast<lwmFastUInt16>(groupedValue);
							}
							else
							{
								for(int vc=0;vc<3;vc++)
									baseValues[vc] = static_cast<lwmFastUInt16>(bitstream.Read(codeLength));
							}

							// Base values can be up to 16 bits, shift+add = 18 bits (14 left over).  After RCP, always less than 2, after the sub it can be from -1..1
							// After scalefactor mult, can be 2..-2
							for(int vc=0;vc<3;vc++)
							{
								lwmUInt32 a = baseValues[vc]*2 + 1;	// Max = 131071
								lwmFixedReal29 b = lwmFixedReal0(a) * rcp;
								lwmFixedReal29 c = b - lwmFixedReal29(1.0);
								lwmFixedReal14 d = scaleFactors[ch][i][cluster] LWMOVIE_FIXEDREAL_CSF_MUL (c);
								coeffs[ch][vc][i] = d;
							}
						}
						else
						{
							for(int vc=0;vc<3;vc++)
								coeffs[ch][vc][i] = lwmFixedReal14(0);
						}
					}
					else
					{
						for(int vc=0;vc<3;vc++)
							coeffs[ch][vc][i] = coeffs[0][vc][i];
					}
				}
			}

			for(int vc=0;vc<3;vc++)
			{
				for(lwmFastUInt8 ch=0;ch<numChannels;ch++)
					SubBandSynthesis(output + ch, m_accumulators[ch], coeffs[ch][vc], m_sbsRotator, numChannels);
				if(++m_sbsRotator == FILTER_SIZE)
					m_sbsRotator = 0;
				output += NUM_SUBBANDS * numChannels;
			}
		}
	}

	return true;
}

bool lwmovie::layerii::lwmCMP2DecodeState::DecodeFrame(const void *bytes, lwmSInt16 *output)
{
	switch(m_numChannels)
	{
	case 1:
		return DecodeFrameCH<1>(bytes, output);
	case 2:
		return DecodeFrameCH<2>(bytes, output);
	}
	return false;
}

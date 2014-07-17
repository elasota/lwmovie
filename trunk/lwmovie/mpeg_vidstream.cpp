/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*
 * Portions of this software Copyright (c) 1995 Brown University.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement
 * is hereby granted, provided that the above copyright notice and the
 * following two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*
Portions copyright (c) 2014 Eric Lasota

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stddef.h>
#include <stdio.h>
#include <new>

#include "lwmovie_types.hpp"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_constants.hpp"
#include "lwmovie_demux.hpp"
#include "lwmovie_recon_m1v.hpp"

lwmovie::constants::lwmEParseState lwmovie::lwmVidStream::ParseSeqHead_MPEG(lwmCBitstream *bitstream)
{
	lwmUInt32 data;

	/* Get horizontal size of image space. */
	data = bitstream->get_bits12();
	if(data != m_h_size)
		return constants::PARSE_FATAL;

	/* Get vertical size of image space. */
	data = bitstream->get_bits12();
	if(data != m_v_size)
		return constants::PARSE_FATAL;

	/* Parse of aspect ratio code. */
	data = bitstream->get_bits4();
	m_aspect_ratio_code = static_cast<lwmUInt8>(data);

	/* Parse off picture rate code. */
	data = bitstream->get_bits4();
	m_picture_rate_code = static_cast<lwmUInt8>(data);

	/* Parse off bit rate. */
	data = bitstream->get_bits18();
	m_bit_rate = data;

	/* Flush marker bit. */
	bitstream->flush_bits(1);

	/* Parse off vbv buffer size. */
	data = bitstream->get_bits10();
	m_vbv_buffer_size = data;

	/* Parse off contrained parameter flag. */
	data = bitstream->get_bits1();
	m_const_param_flag = (data != 0);

	/*
	 * If intra_quant_matrix_flag set, parse off intra quant matrix values.
	 */
	data = bitstream->get_bits1();
	if (data != 0)
	{
		for (int i = 0; i < 64; i++)
			m_sequence.m_intra_quant_matrix[constants::ZIGZAG[i]] = static_cast<lwmUInt8>(bitstream->get_bits8());
	}
	/*
	 * If non intra quant matrix flag set, parse off non intra quant matrix
	 * values.
	 */
	data = bitstream->get_bits1();
	if (data != 0)
	{
		for (int i = 0; i < 64; i++)
			m_sequence.m_non_intra_quant_matrix[constants::ZIGZAG[i]] = static_cast<lwmUInt8>(bitstream->get_bits8());
	}

	return constants::PARSE_OK;
}

lwmovie::constants::lwmEParseState lwmovie::lwmVidStream::ParsePicture(lwmCBitstream *bitstream)
{
	lwmUInt32 data;

	/* Flush header start code. */
	bitstream->flush_bits(8);

	/* This happens if there is a picture code before a sequence start */
	// TODO: Detect missing seq start

	/* Parse off temporal reference. */
	data = bitstream->get_bits10();
	m_picture.temp_ref = data;

	/* Parse of picture type. */
	data = bitstream->get_bits3();
	m_picture.code_type = data;

	if(m_picture.code_type != constants::MPEG_B_TYPE &&
		m_picture.code_type != constants::MPEG_I_TYPE &&
		m_picture.code_type != constants::MPEG_P_TYPE)
		return constants::PARSE_SKIP_PICTURE;	/* garbage code */

	if ((m_picture.code_type == constants::MPEG_B_TYPE) &&
		((m_future == lwmRECONSLOT_Unassigned) ||
		 (m_past == lwmRECONSLOT_Unassigned)))
		/* According to 2-D.5.1 (p D-18) this is ok, if the refereneces are OK */
		return constants::PARSE_SKIP_PICTURE;

	if ((m_picture.code_type == constants::MPEG_P_TYPE) && (m_future == lwmRECONSLOT_Unassigned))
		return constants::PARSE_SKIP_PICTURE;

	/* Parse off vbv buffer delay value. */
	data = bitstream->get_bits16();
	m_picture.vbv_delay = data;

	/* If P or B type frame... */
	if ((m_picture.code_type == constants::MPEG_P_TYPE) || 
		(m_picture.code_type == constants::MPEG_B_TYPE))
	{
		/* Parse off forward vector full pixel flag. */
		data = bitstream->get_bits1();
		m_picture.full_pel_forw_vector = (data != 0);

		/* Parse of forw_r_code. */
		data = bitstream->get_bits3();

		/* Decode forw_r_code into forw_r_size and forw_f. */
		m_picture.forw_r_size = (data != 0) ? (data - 1) : 0;
		m_picture.forw_f = 1 << m_picture.forw_r_size;
	}

	/* If B type frame... */
	if(m_picture.code_type == constants::MPEG_B_TYPE)
	{
		/* Parse off back vector full pixel flag. */
		data = bitstream->get_bits1();
		m_picture.full_pel_back_vector = (data != 0);

		/* Parse off back_r_code. */
		data = bitstream->get_bits3();

		/* Decode back_r_code into back_r_size and back_f. */
		m_picture.back_r_size = (data != 0) ? (data - 1) : 0;
		m_picture.back_f = 1 << m_picture.back_r_size;
	}

	if(m_picture.code_type == constants::MPEG_B_TYPE)
		m_current = lwmRECONSLOT_B;
	else if(m_picture.code_type == constants::MPEG_I_TYPE)
	{
		if(m_future == lwmRECONSLOT_Unassigned)
			m_current = m_future = lwmRECONSLOT_IP1;
		else if(m_past == lwmRECONSLOT_Unassigned)
		{
			lwmEReconSlot slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = slot;
		}
		else
		{
			m_future = m_past;
			lwmEReconSlot slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = slot;
		}
	}
	else if(m_picture.code_type == constants::MPEG_P_TYPE)
	{
		if(m_future == lwmRECONSLOT_Unassigned)
			return constants::PARSE_SKIP_PICTURE;
		else if(m_past == lwmRECONSLOT_Unassigned)
		{
			lwmEReconSlot slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = slot;
		}
		else
		{
			m_future = m_past;
			lwmEReconSlot slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = slot;
		}
	}
	else
		return constants::PARSE_SKIP_PICTURE;

	return constants::PARSE_OK;
}

lwmovie::lwmVidStream::lwmVidStream(const lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, bool useThreadedDeslicer)
	: m_stDeslicerJob((width + 15) / 16, (height + 15) / 16)
{
	m_alloc = *alloc;

	/* Copy default intra matrix. */
	for(int i = 0; i < 64; i++)
		m_sequence.m_intra_quant_matrix[i] = constants::DEFAULT_INTRA_MATRIX[i];

	/* Initialize non intra quantization matrix. */
	for(int i = 0; i < 64; i++)
		m_sequence.m_non_intra_quant_matrix[i] = constants::DEFAULT_NON_INTRA_MATRIX[i];

	m_h_size = width;
	m_v_size = height;

	m_mb_width = (m_h_size + 15) / 16;
	m_mb_height = (m_v_size + 15) / 16;

	/* Initialize pointers to image spaces. */
	m_current = m_past = m_future = lwmRECONSLOT_Unassigned;

	m_useThreadedDeslicer = useThreadedDeslicer;
}

void lwmovie::lwmVidStream::SkipExtraBitInfo(lwmCBitstream *bitstream)
{
	lwmUInt32 data;
	lwmUInt32 safetyLimit = constants::MPEG_EXTRA_SAFETY_LIMIT;

	/* Get first flag bit. */
	data = bitstream->get_bits1();

	/* If flag is false, return NULL pointer (i.e. no extra bit info). */
	if(data == 0)
		return;

	/* While flag bit is true. */
	while (data != 0 && safetyLimit != 0)
	{
		/* Get next 8 bits of data. */
		data = bitstream->get_bits8();

		/* Get next flag bit. */
		data = bitstream->get_bits1();

		safetyLimit--;
	}
}

bool lwmovie::lwmVidStream::DigestStreamParameters(const void *bytes, lwmUInt32 packetSize, lwmIM1VReconstructor *recon)
{
	recon->WaitForFinish();

	lwmCBitstream bitstream;
	bitstream.Initialize(bytes, packetSize);
	if(ParseSeqHead_MPEG(&bitstream) != constants::PARSE_OK)
		return false;

	return true;
}

bool lwmovie::lwmVidStream::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, lwmIM1VReconstructor *recon, lwmUInt32 *outResult)
{
	if(packetSize < 1)
		return false;

	lwmUInt32 packetTypeCode = (0x00000100 | static_cast<const lwmUInt8 *>(bytes)[0]);

	if (packetTypeCode == constants::MPEG_PICTURE_START_CODE)
	{
		recon->WaitForFinish();

		lwmCBitstream bitstream;
		bitstream.Initialize(bytes, packetSize);

		if (ParsePicture(&bitstream) != constants::PARSE_OK)
			return false;

		recon->StartNewFrame(m_current - lwmRECONSLOT_FirstListed, m_future - lwmRECONSLOT_FirstListed, m_past - lwmRECONSLOT_FirstListed);
		*outResult = lwmDIGEST_Nothing;
		return true;
	}

	if (packetTypeCode >= constants::MPEG_SLICE_MIN_START_CODE && packetTypeCode <= constants::MPEG_SLICE_MAX_START_CODE)
	{
		// TODO: Thread deslicer
		m_stDeslicerJob.Digest(&m_sequence, &m_picture, bytes, packetSize, recon);
		return true;
	}

	return false;
}

void lwmovie::lwmVidStream::SignalIOFailure()
{
}

void lwmovie::lwmVidStream::ReportError(const char *errorStr)
{
}

void lwmovie::lwmVidStream::OutputFinishedFrame()
{
}

/*
        void ExecuteDisplay()
        {
            if( _skipFrame != 0 )
            {
                _skipFrame--;
                return;
            }

            uint frameRateNum = 1;
            uint frameRateDenom = 1;

            frameRateDenom = 1;
            switch (picture_rate)
            {
                case 1:
                    frameRateNum = 23976;
                    frameRateDenom = 1000;
                    break;
                case 2:
                    frameRateNum = 24;
                    break;
                case 3:
                    frameRateNum = 25;
                    break;
                case 4:
                    frameRateNum = 2997;
                    frameRateDenom = 100;
                    break;
                case 5:
                    frameRateNum = 30;
                    break;
                case 6:
                    frameRateNum = 50;
                    break;
                case 7:
                    frameRateNum = 5994;
                    frameRateDenom = 100;
                    break;
                case 8:
                    frameRateNum = 60;
                    break;
                default:
                    return; // Forget it
            }
            
            _smpeg.ExecuteDisplay(new ImageInfo(h_size, v_size, mb_width * 16, mb_width * 8, mb_width * 8,
                current.luminance, current.Cb, current.Cr, frameRateNum, frameRateDenom));
        }


        internal void SetFrameSkip(int numFrames)
        {
            _skipFrame = numFrames;
        }
    }


    public struct ImageInfo
    {
        public readonly uint width;
        public readonly uint height;

        public readonly uint yPitch;
        public readonly uint cbPitch;
        public readonly uint crPitch;

        public readonly byte[] yPlane;
        public readonly byte[] cbPlane;
        public readonly byte[] crPlane;

        public readonly uint frameRateNum;
        public readonly uint frameRateDenom;

        public ImageInfo(uint width, uint height, uint yPitch, uint cbPitch, uint crPitch, byte[] yPlane, byte[] cbPlane, byte[] crPlane, uint frameRateNum, uint frameRateDenom)
        {
            this.width = width;
            this.height = height;

            this.yPitch = yPitch;
            this.cbPitch = cbPitch;
            this.crPitch = crPitch;

            this.yPlane = yPlane;
            this.cbPlane = cbPlane;
            this.crPlane = crPlane;

            this.frameRateNum = frameRateNum;
            this.frameRateDenom = frameRateDenom;
        }
    }
}

	*/
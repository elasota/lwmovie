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
#include <string.h>

#include "../common/lwmovie_coretypes.h"
#include "../common/lwmovie_atomicint_type.hpp"
#include "../common/lwmovie_atomicint_funcs.hpp"
#include "lwmovie.h"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_constants.hpp"
#include "lwmovie_recon_m1v.hpp"

lwmovie::m1v::constants::lwmEParseState lwmovie::m1v::CVidStream::ParseSeqHead_MPEG(CBitstream *bitstream)
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

	lwmUInt32 wasteBits = bitstream->get_bits16();
	lwmUInt32 wasteBits2 = bitstream->get_bits16();

	return constants::PARSE_OK;
}

lwmovie::m1v::constants::lwmEParseState lwmovie::m1v::CVidStream::ParsePicture(CBitstream *bitstream)
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

	if ((m_picture.code_type == lwmovie::m1v::constants::MPEG_B_TYPE) &&
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
		m_picture.forw_h_size = (data != 0) ? (data - 1) : 0;
		m_picture.forw_v_size = m_picture.forw_h_size;
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
		m_picture.back_h_size = (data != 0) ? (data - 1) : 0;
		m_picture.back_v_size = m_picture.back_h_size;
	}

	if(m_picture.code_type == constants::MPEG_B_TYPE)
	{
		if(m_dropAggressiveness < lwmDROPAGGRESSIVENESS_B && m_allowBFrames)
			m_current = m_outputSlot = lwmRECONSLOT_B;
		else
			m_current = m_outputSlot = lwmRECONSLOT_Dropped_B;
	}
	else if(m_picture.code_type == constants::MPEG_I_TYPE)
	{
		if(m_future == lwmRECONSLOT_Unassigned)
		{
			lwmEReconSlot slot;
			if(m_dropAggressiveness >= lwmDROPAGGRESSIVENESS_BPI)
				slot = lwmRECONSLOT_Dropped_IP;
			else
				slot = lwmRECONSLOT_IP1;
			m_current = m_future = m_outputSlot = slot;
		}
		else if(m_past == lwmRECONSLOT_Unassigned)
		{
			lwmEReconSlot slot;

			if(m_dropAggressiveness >= lwmDROPAGGRESSIVENESS_BPI)
				slot = lwmRECONSLOT_Dropped_IP;
			else
				slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = m_outputSlot = slot;
		}
		else
		{
			m_future = m_past;
			lwmEReconSlot slot;

			if(m_dropAggressiveness >= lwmDROPAGGRESSIVENESS_BPI)
				slot = lwmRECONSLOT_Dropped_IP;
			else
				slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = m_outputSlot = slot;
		}
	}
	else if(m_picture.code_type == constants::MPEG_P_TYPE)
	{
		if(m_future == lwmRECONSLOT_Unassigned || m_future == lwmRECONSLOT_Dropped_IP)
		{
			m_current = m_past = m_future = lwmRECONSLOT_Dropped_IP;
			return constants::PARSE_OK;
		}
		else if(m_past == lwmRECONSLOT_Unassigned)
		{
			// Have a forward prediction frame, don't have a back prediction, this should be it
			lwmEReconSlot slot;
			if(m_dropAggressiveness >= lwmDROPAGGRESSIVENESS_BP)
				slot = lwmRECONSLOT_Dropped_IP;
			else
				slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = m_outputSlot = slot;
		}
		else
		{
			// Have a past and future, OR the past prediction is a dropped frame
			m_future = m_past;
			lwmEReconSlot slot;
			if(m_future == lwmRECONSLOT_Dropped_IP || m_dropAggressiveness >= lwmDROPAGGRESSIVENESS_BP)
				slot = lwmRECONSLOT_Dropped_IP;
			else
				slot = (m_future == lwmRECONSLOT_IP1) ? lwmRECONSLOT_IP2 : lwmRECONSLOT_IP1;
			m_current = m_past = m_outputSlot = slot;
		}
	}
	else
		return constants::PARSE_SKIP_PICTURE;

	return constants::PARSE_OK;
}

lwmovie::m1v::constants::lwmEParseState lwmovie::m1v::CVidStream::ParseExtension(CBitstream *bitstream)
{
	bitstream->flush_bits(8);

	const lwmUInt32 pceFirstDWORD = bitstream->show_bits32();

	const lwmUInt32 extType = (pceFirstDWORD >> 28) & 0xf;

	if (extType == constants::MPEG_EXT_TYPE_PICTURE_CODING_EXTENSION)
	{
		if (!m_canAcceptPCE)
			return constants::PARSE_SKIP_PICTURE;

		lwmUInt8 fCode[2][2];
		for (int s = 0; s < 2; s++)
		{
			for (int t = 0; t < 2; t++)
			{
				fCode[s][t] = static_cast<lwmUInt8>((pceFirstDWORD >> (24 - s * 8 - t * 4)) & 0xf);
				if (fCode[s][t] > 0)
					fCode[s][t]--;
			}
		}

		const lwmUInt8 intraDCPrecision = static_cast<lwmUInt8>((pceFirstDWORD >> 10) & 0x3);
		const lwmUInt8 pictureStructure = static_cast<lwmUInt8>((pceFirstDWORD >> 8) & 0x3);

		const bool topFieldFirst = ((pceFirstDWORD >> 7) & 1) != 0;
		const bool framePredFrameDCT = ((pceFirstDWORD >> 6) & 1) != 0;
		const bool concealmentMotionVectors = ((pceFirstDWORD >> 5) & 1) != 0;
		const bool qScaleType = ((pceFirstDWORD >> 4) & 1) != 0;
		const bool intraVLCFormat = ((pceFirstDWORD >> 3) & 1) != 0;
		const bool alternateScan = ((pceFirstDWORD >> 2) & 1) != 0;
		const bool repeatFirstField = ((pceFirstDWORD >> 1) & 1) != 0;
		const bool chroma420Type = ((pceFirstDWORD >> 0) & 1) != 0;

		// We ignore the next 3 bytes since this should always be progressive scan and we don't care about composite info.
		m_picture.forw_h_size = fCode[0][0];
		m_picture.forw_v_size = fCode[0][1];
		m_picture.back_h_size = fCode[1][0];
		m_picture.back_v_size = fCode[1][1];

		m_canAcceptPCE = false;

		return constants::PARSE_OK;
	}
	else
		return constants::PARSE_SKIP_PICTURE;
}

lwmovie::m1v::CVidStream::CVidStream(lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, bool allowBFrames, lwmMovieState *movieState, lwmSWorkNotifier *workNotifier, bool useThreadedDeslicer, bool isMPEG2)
	: m_stDeslicerJob((width + 15) / 16, (height + 15) / 16)
	, m_deslicerMemPool(alloc, useThreadedDeslicer ? 150000 : 0)
	, m_dropAggressiveness(lwmDROPAGGRESSIVENESS_None)
	, m_allowBFrames(allowBFrames)
	, m_isMPEG2(isMPEG2)
	, m_canAcceptPCE(false)
{
	m_alloc = alloc;

	m_stBlockCursor = NULL;
	m_deslicerJobStack = NULL;
	m_movieState = movieState;
	m_workNotifier = workNotifier;

	/* Copy default intra matrix. */
	for(int i = 0; i < 64; i++)
		m_sequence.m_intra_quant_matrix[i] = constants::DEFAULT_INTRA_MATRIX[i];

	/* Initialize non intra quantization matrix. */
	for(int i = 0; i < 64; i++)
		m_sequence.m_non_intra_quant_matrix[i] = constants::DEFAULT_NON_INTRA_MATRIX[i];

	m_sequence.m_isMPEG2 = isMPEG2;

	m_h_size = width;
	m_v_size = height;

	m_mb_width = (m_h_size + 15) / 16;
	m_mb_height = (m_v_size + 15) / 16;

	/* Initialize pointers to image spaces. */
	m_current = m_past = m_future = m_outputSlot = lwmRECONSLOT_Unassigned;

	m_useThreadedDeslicer = useThreadedDeslicer;
}

lwmovie::m1v::CVidStream::~CVidStream()
{
	WaitForDigestFinish();
	m_deslicerMemPool.Destroy(m_alloc);
	if(m_stBlockCursor)
	{
		m_stBlockCursor->~IM1VBlockCursor();
		m_alloc->Free(m_stBlockCursor);
	}
}

void lwmovie::m1v::CVidStream::SkipExtraBitInfo(CBitstream *bitstream)
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

bool lwmovie::m1v::CVidStream::DigestStreamParameters(const void *bytes, lwmUInt32 packetSize)
{
	CBitstream bitstream;
	bitstream.Initialize(bytes, packetSize);
	if (ParseSeqHead_MPEG(&bitstream) != lwmovie::m1v::constants::PARSE_OK)
		return false;

	return true;
}

void lwmovie::m1v::CVidStream::DispatchDeslicerJob(const void *bytes, lwmUInt32 packetSize, IM1VReconstructor *recon)
{
	IM1VBlockCursor *blockCursor = recon->CreateBlockCursor();

	if(!blockCursor)
		return;		// TODO: Fix this...

	// Check for integer overflow
	if (UINT32_MAX - sizeof(SDeslicerJobStackNode) < packetSize)
		return;

	bool pooled = true;
	lwmUInt32 stackNodeSize = sizeof(SDeslicerJobStackNode) + packetSize - 1;
	// TODO: SIMD align
	stackNodeSize += 15;
	stackNodeSize &= ~(static_cast<lwmUInt32>(15));

	void *memBuf = m_deslicerMemPool.Alloc(stackNodeSize);
	if(!memBuf)
	{
		pooled = false;
		memBuf = m_alloc->NAlloc<lwmUInt8>(stackNodeSize);
	}
	
	if(memBuf == NULL)
	{
		m_stDeslicerJob.Digest(&m_sequence, blockCursor, &m_picture, bytes, packetSize, recon);
		blockCursor->~IM1VBlockCursor();
		m_alloc->Free(blockCursor);
		return;
	}

	SDeslicerJobStackNode *deslicerStackNode = static_cast<SDeslicerJobStackNode*>(memBuf);

	new (deslicerStackNode) SDeslicerJobStackNode(m_alloc, m_mb_width, m_mb_height);
	deslicerStackNode->m_accessFlag = 0;
	memcpy(deslicerStackNode->m_dataBytes, bytes, packetSize);
	deslicerStackNode->m_dataSize = packetSize;
	deslicerStackNode->m_recon = recon;
	deslicerStackNode->m_memPooled = pooled;
	deslicerStackNode->m_blockCursor = blockCursor;

	// Insert into the job stack
	while(true)
	{
		SDeslicerJobStackNode *stackHead = m_deslicerJobStack;
		deslicerStackNode->m_next = stackHead;
		if(lwmAtomicCompareAndSwap(reinterpret_cast<void *volatile *>(&m_deslicerJobStack), deslicerStackNode, stackHead) == stackHead)
			break;
	}

	if(m_workNotifier)
		m_workNotifier->notifyAvailableFunc(m_workNotifier);
}

void lwmovie::m1v::CVidStream::DestroyDeslicerJobs()
{
	SDeslicerJobStackNode *jobNode = m_deslicerJobStack;
	m_deslicerJobStack = NULL;
	while(jobNode)
	{
		SDeslicerJobStackNode *nextNode = jobNode->m_next;
#ifdef LWMOVIE_PROFILE
		jobNode->m_deslicerJob.GetProfileTags()->FlushTo(m_stDeslicerJob.GetProfileTags());
#endif
		if(!jobNode->m_memPooled)
		{
			jobNode->~SDeslicerJobStackNode();
			m_alloc->Free(jobNode);
		}
		else
			jobNode->~SDeslicerJobStackNode();

		jobNode = nextNode;
	}

	m_deslicerMemPool.Reset(m_alloc);
}

bool lwmovie::m1v::CVidStream::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, lwmUInt32 *outResult)
{
	if(packetSize < 1)
		return false;

	lwmUInt32 packetTypeCode = (0x00000100 | static_cast<const lwmUInt8 *>(bytes)[0]);

	if (packetTypeCode == constants::MPEG_PICTURE_START_CODE)
	{
		WaitForDigestFinish();
		m_recon->WaitForFinish();

		m_canAcceptPCE = false;

		CBitstream bitstream;
		bitstream.Initialize(bytes, packetSize);

		if (ParsePicture(&bitstream) != constants::PARSE_OK)
			return false;

		m_recon->StartNewFrame(m_current, m_future, m_past, m_picture.code_type == constants::MPEG_B_TYPE);
		*outResult = lwmDIGEST_Worked;
		m_canAcceptPCE = m_isMPEG2;
		return true;
	}

	if (packetTypeCode >= constants::MPEG_SLICE_MIN_START_CODE && packetTypeCode <= constants::MPEG_SLICE_MAX_START_CODE)
	{
		if(m_current == lwmRECONSLOT_Dropped_IP || m_current == lwmRECONSLOT_Dropped_B)
			return true;

		if(m_workNotifier)
		{
			DispatchDeslicerJob(bytes, packetSize, m_recon);
		}
		else
		{
			if(!m_stBlockCursor)
			{
				m_stBlockCursor = m_recon->CreateBlockCursor();
				if(!m_stBlockCursor)
					return false;
			}

			m_stDeslicerJob.Digest(&m_sequence, m_stBlockCursor, &m_picture, bytes, packetSize, m_recon);
		}
		return true;
	}

	if (packetTypeCode == constants::MPEG_EXT_START_CODE)
	{
		CBitstream bitstream;
		bitstream.Initialize(bytes, packetSize);

		if (ParseExtension(&bitstream) != constants::PARSE_OK)
			return false;

		return true;
	}

	return false;
}

void lwmovie::m1v::CVidStream::SetReconstructor(lwmIVideoReconstructor *recon)
{
	m_recon = static_cast<IM1VReconstructor*>(recon);
}

void lwmovie::m1v::CVidStream::Participate()
{
	// Priority 1 is to consume a deslicer job
	SDeslicerJobStackNode *dsJob = m_deslicerJobStack;
	while(dsJob)
	{
		if(lwmAtomicIncrement(&dsJob->m_accessFlag) == 1)
		{
			// Can consume this job
			dsJob->m_deslicerJob.Digest(&m_sequence, dsJob->m_blockCursor, &m_picture, dsJob->m_dataBytes, dsJob->m_dataSize, m_recon);
			return;
		}
		dsJob = dsJob->m_next;
	}

	// If that fails, then the job must have come from the reconstructor
	m_recon->Participate();
}

void lwmovie::m1v::CVidStream::WaitForDigestFinish()
{
	if(m_workNotifier)
		m_workNotifier->joinFunc(m_workNotifier);
	DestroyDeslicerJobs();
}

bool lwmovie::m1v::CVidStream::EmitFrame()
{
	if(m_current != lwmRECONSLOT_Dropped_IP && m_current != lwmRECONSLOT_Dropped_B)
	{
		// TODO: Handle bad syncs
		m_recon->PresentFrame(m_outputSlot);
	}

	// TODO: Possibly mark
	if(m_outputSlot == lwmRECONSLOT_B || m_outputSlot == lwmRECONSLOT_Dropped_B)
		m_outputSlot = m_past;

	return true;
}

void lwmovie::m1v::CVidStream::OutputFinishedFrame()
{
}

lwmovie::m1v::CVidStream::SDeslicerMemoryPool::SDeslicerMemoryPool(lwmSAllocator *alloc, lwmUInt32 initialCapacity)
{
	memBytes = NULL;
	memCapacity = 0;
	memRemaining = 0;
	nextCapacity = initialCapacity;

	Reset(alloc);
}

void *lwmovie::m1v::CVidStream::SDeslicerMemoryPool::Alloc(lwmUInt32 size)
{
	nextCapacity += size;
	if(memRemaining < size)
		return NULL;
	void *outBuf = static_cast<lwmUInt8*>(memBytes) + (memCapacity - memRemaining);
	memRemaining -= size;
	return outBuf;
}

void lwmovie::m1v::CVidStream::SDeslicerMemoryPool::Reset(lwmSAllocator *alloc)
{
	if(nextCapacity > memCapacity)
	{
		if(memBytes)
			alloc->Free(memBytes);
		memBytes = alloc->NAlloc<lwmUInt8>(nextCapacity);
		if(!memBytes)
			memCapacity = 0;
		else
			memCapacity = nextCapacity;
	}

	memRemaining = memCapacity;
	nextCapacity = 0;
}

void lwmovie::m1v::CVidStream::SDeslicerMemoryPool::Destroy(lwmSAllocator *alloc)
{
	if(memBytes)
		alloc->Free(memBytes);
}

void lwmovie::m1v::CVidStream::SetDropAggressiveness(lwmEDropAggressiveness dropAggressiveness)
{
	this->m_dropAggressiveness = dropAggressiveness;
}

void lwmovie::m1v::CVidStream::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
#ifdef LWMOVIE_PROFILE
	m_stDeslicerJob.GetProfileTags()->FlushTo(tagSet);
#endif
}

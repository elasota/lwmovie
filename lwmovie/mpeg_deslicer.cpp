#include "lwmovie_videotypes.hpp"
#include "lwmovie_vlc.hpp"
#include "lwmovie_recon_m1v.hpp"
#include "lwmovie_profile.hpp"

lwmovie::lwmDeslicerJob::lwmDeslicerJob(lwmUInt32 mbWidth, lwmUInt32 mbHeight)
{
	m_mb_width = mbWidth;
	m_mb_height = mbHeight;
}

bool lwmovie::lwmDeslicerJob::Digest(const mpegSequence *sequenceData, const mpegPict *pictData, const void *sliceData, lwmUInt32 sliceSize, lwmIM1VReconstructor *recon)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(&m_profileTags, lwmEPROFILETAG_Deslice);
#endif

	lwmCBitstream bitstream;
	bitstream.Initialize(sliceData, sliceSize);
	m_sequence = sequenceData;
	m_picture = pictData;

	if(ParseSliceHeader(&bitstream) != constants::PARSE_OK)
		return false;

	/* Parse slice macroblocks */
		
	lwmSInt32 max_mb_addr = static_cast<lwmSInt32>(m_mb_width * m_mb_height - 1);

	while(true)
	{
		/* Parse Macroblock. */
		constants::lwmEParseState parseState = ParseMacroBlock(&bitstream, max_mb_addr, recon, &m_profileTags);
		if(parseState == constants::PARSE_BREAK)
			break;
		if(parseState != constants::PARSE_OK)
			return false;
	}

	return true;
}

bool lwmovie::lwmDeslicerJob::ParseSliceHeader(lwmCBitstream *bitstream)
{
	lwmUInt32 basePos = bitstream->get_bits8();

	/* Parse off slice vertical position. */
	lwmUInt32 vertPos;
	if(this->m_mb_height > 175)
	{
		/* slice vertical extension */
		if(basePos == 0 || basePos > 127)
			return false;
		vertPos = basePos | (bitstream->get_bits3() << 7);
	}
	else
	{
		if(basePos < (constants::MPEG_SLICE_MIN_START_CODE & 0xff) || basePos > (constants::MPEG_SLICE_MAX_START_CODE & 0xff))
			return false;
		vertPos = basePos;
	}

	if(vertPos > m_mb_height)
		return false;
	
	m_slice.vert_pos = vertPos;

	lwmUInt32 data;

	/* Parse off quantization scale. */
	data = bitstream->get_bits5();
	m_slice.quant_scale = data;

	/* Parse off extra bit slice info. */
	lwmVidStream::SkipExtraBitInfo(bitstream);

	/* Reset past intrablock address. */
	m_mblock.past_intra_addr = -2;

	/* Reset previous recon motion vectors. */
	m_mblock.recon_right_for_prev = 0;
	m_mblock.recon_down_for_prev = 0;
	m_mblock.recon_right_back_prev = 0;
	m_mblock.recon_down_back_prev = 0;

	/* Reset macroblock address. */
	m_mblock.mb_address = m_mblock.past_mb_addr = static_cast<lwmSInt32>(((m_slice.vert_pos - 1) * m_mb_width)) - 1;
	
	/* Reset past dct dc y, cr, and cb values. */
	m_block.dct_dc_y_past = 1024 << 3;
	m_block.dct_dc_cr_past = 1024 << 3;
	m_block.dct_dc_cb_past = 1024 << 3;

	return true;
}


/*
 *--------------------------------------------------------------
 *
 * ParseMacroBlock --
 *
 *      Parseoff macroblock. Reconstructs DCT values. Applies
 *      inverse DCT, reconstructs motion vectors, calculates and
 *      set pixel values for macroblock in current pict image
 *      structure.
 *
 * Results:
 *      Here's where everything really happens. Welcome to the
 *      heart of darkness.
 *
 * Side effects:
 *      Bit stream irreversibly parsed off.
 *
 *--------------------------------------------------------------
 */
lwmovie::constants::lwmEParseState lwmovie::lwmDeslicerJob::ParseMacroBlock( lwmCBitstream *bitstream, lwmSInt32 max_mb_addr, lwmIM1VReconstructor *recon, lwmCProfileTagSet *profileTags )
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ParseBlock);
#endif

	constants::lwmEMBQuantType mb_quant = constants::MB_QUANT_TYPE_FALSE;
	/* Multiple ops necessary to deal with mb -1... */
	lwmSInt32 row_end_mb = m_mblock.mb_address + 1;
	row_end_mb -= row_end_mb % static_cast<lwmSInt32>(m_mb_width);
	row_end_mb += static_cast<lwmSInt32>(m_mb_width - 1);

	/*
	 * Parse off macroblock address increment and add to macroblock address.
	 */
	lwmUInt8 addr_incr;
	do
	{
		lwmUInt32 ind = bitstream->show_bits11();
		if(ind == 0)
			return constants::PARSE_BREAK;

		addr_incr = DecodeMBAddrInc(bitstream);

		if (addr_incr == lwmovie::vlc::UERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;

		if (lwmovie::vlc::mb_addr_inc[ind].num_bits == 0)
			addr_incr = 1;
		else if (addr_incr == lwmovie::constants::MPEG_MB_ESCAPE)
		{
			m_mblock.mb_address += 33;
			addr_incr = lwmovie::constants::MPEG_MB_STUFFING;
			if(m_mblock.mb_address > max_mb_addr)
				return constants::PARSE_SKIP_TO_START_CODE;
		}
	} while (addr_incr == lwmovie::constants::MPEG_MB_STUFFING);

	m_mblock.mb_address += addr_incr;
	if(m_mblock.mb_address > max_mb_addr)
		return lwmovie::constants::PARSE_SKIP_TO_START_CODE;

	if( m_mblock.mb_address < 0 )
		return lwmovie::constants::PARSE_SKIP_TO_START_CODE;

	/*
	 * If macroblocks have been skipped, process skipped macroblocks.
	 */
	if (m_mblock.mb_address - m_mblock.past_mb_addr > 1)
	{
		/* Copy this to all recons */
		for (lwmSInt32 i = m_mblock.past_mb_addr + 1; i < m_mblock.mb_address; i++)
		{
			if(m_picture->code_type == constants::MPEG_P_TYPE)
				recon->SetMBlockInfo(i, true, true, false, 0, 0, false, 0, 0, false);
			else if(m_picture->code_type == constants::MPEG_B_TYPE)
				recon->SetMBlockInfo(i, true, m_mblock.bpict_past_forw, m_mblock.bpict_past_back, m_mblock.recon_right_for_prev, m_mblock.recon_down_for_prev, m_picture->full_pel_forw_vector, m_mblock.recon_right_back_prev, m_mblock.recon_down_back_prev, m_picture->full_pel_back_vector);
			else
				return constants::PARSE_SKIP_TO_START_CODE;

			if(i == row_end_mb)
			{
				recon->MarkRowFinished(static_cast<lwmUInt32>(row_end_mb - (m_mb_width - 1)));
				row_end_mb += static_cast<lwmSInt32>(m_mb_width);
			}
		}

		if (m_picture->code_type == constants::MPEG_P_TYPE)
		{
			// Reset the predictor
			m_mblock.recon_right_for_prev = 0;
			m_mblock.recon_down_for_prev = 0;
		}
	}
	/* Set past macroblock address to current macroblock address. */
	m_mblock.past_mb_addr = m_mblock.mb_address;

	bool mb_motion_forw, mb_motion_back;

	bool mb_pattern;

	/* Based on picture type decode macroblock type. */
	switch (m_picture->code_type)
	{
	case constants::MPEG_I_TYPE:
		DecodeMBTypeI(bitstream, &mb_quant, &mb_motion_forw, &mb_motion_back, &mb_pattern, &m_mblock.mb_intra);
		break;

	case constants::MPEG_P_TYPE:
		DecodeMBTypeP(bitstream, &mb_quant, &mb_motion_forw, &mb_motion_back, &mb_pattern, &m_mblock.mb_intra);
		break;

	case constants::MPEG_B_TYPE:
		DecodeMBTypeB(bitstream, &mb_quant, &mb_motion_forw, &mb_motion_back, &mb_pattern, &m_mblock.mb_intra);
		break;
	default:
		return constants::PARSE_SKIP_TO_START_CODE;
	}

	/* If quantization flag set, parse off new quantization scale. */
	if (mb_quant == constants::MB_QUANT_TYPE_TRUE)
	{
		lwmUInt32 data = bitstream->get_bits5();
		m_slice.quant_scale = data;
	}
	/* If forward motion vectors exist... */
	if (mb_motion_forw)
	{
		/* Parse off and decode horizontal forward motion vector. */
		m_mblock.motion_h_forw_code = DecodeMotionVectors(bitstream);

		if(m_mblock.motion_h_forw_code == lwmovie::vlc::ERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;

		/* If horiz. forward r data exists, parse off. */
		if ((m_picture->forw_f != 1) &&
			(m_mblock.motion_h_forw_code != 0))
		{
			lwmUInt32 data = bitstream->get_bitsn(m_picture->forw_r_size);
			m_mblock.motion_h_forw_r = data;
		}
		/* Parse off and decode vertical forward motion vector. */
		m_mblock.motion_v_forw_code = DecodeMotionVectors(bitstream);

		if(m_mblock.motion_v_forw_code == lwmovie::vlc::ERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;

		/* If vert. forw. r data exists, parse off. */
		if ((m_picture->forw_f != 1) &&
			(m_mblock.motion_v_forw_code != 0))
		{
			lwmUInt32 data = bitstream->get_bitsn(m_picture->forw_r_size);
			m_mblock.motion_v_forw_r = data;
		}
	}
	/* If back motion vectors exist... */
	if (mb_motion_back)
	{
		/* Parse off and decode horiz. back motion vector. */
		m_mblock.motion_h_back_code = DecodeMotionVectors(bitstream);
		
		if(m_mblock.motion_h_back_code == lwmovie::vlc::ERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;

		/* If horiz. back r data exists, parse off. */
		if ((m_picture->back_f != 1) &&
			(m_mblock.motion_h_back_code != 0))
		{
			lwmUInt32 data = bitstream->get_bitsn(m_picture->back_r_size);
			m_mblock.motion_h_back_r = data;
		}
		/* Parse off and decode vert. back motion vector. */
		m_mblock.motion_v_back_code = DecodeMotionVectors(bitstream);

		if(m_mblock.motion_v_back_code == lwmovie::vlc::ERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;

		/* If vert. back r data exists, parse off. */
		if ((m_picture->back_f != 1) &&
			(m_mblock.motion_v_back_code != 0))
		{
			lwmUInt32 data = bitstream->get_bitsn(m_picture->back_r_size);
			m_mblock.motion_v_back_r = data;
		}
	}

	/* If mblock pattern flag set, parse and decode CBP (code block pattern). */
	if (mb_pattern)
	{
		m_mblock.cbp = DecodeCBP(bitstream);

		if(m_mblock.cbp == lwmovie::vlc::UERROR8)
			return constants::PARSE_SKIP_TO_START_CODE;
	}
	/* Otherwise, set CBP to zero. */
	else
		m_mblock.cbp = 0;

	lwmSInt32 recon_right_for, recon_down_for;
	lwmSInt32 recon_right_back, recon_down_back;

	/* Reconstruct motion vectors depending on picture type. */
	if (m_picture->code_type == lwmovie::constants::MPEG_P_TYPE)
	{
		/*
		 * If no forw motion vectors, reset previous and current vectors to 0.
		 */
		if (!mb_motion_forw)
		{
			recon_right_for = 0;
			recon_down_for = 0;
			m_mblock.recon_right_for_prev = 0;
			m_mblock.recon_down_for_prev = 0;
		}
		/*
		 * Otherwise, compute new forw motion vectors. Reset previous vectors to
		 * current vectors.
		 */
		else
			ComputeForwVector(&recon_right_for, &recon_down_for);

		/* Turn motion flag on, P frame MBs always have motion */
		mb_motion_forw = !m_mblock.mb_intra;
		recon_right_back = recon_down_back = 0;
	}
	else if (m_picture->code_type == lwmovie::constants::MPEG_B_TYPE)
	{
		/* Reset prev. and current vectors to zero if mblock is intracoded. */

		if (m_mblock.mb_intra)
		{
			m_mblock.recon_right_for_prev = 0;
			m_mblock.recon_down_for_prev = 0;
			m_mblock.recon_right_back_prev = 0;
			m_mblock.recon_down_back_prev = 0;

			recon_right_for = recon_down_for = recon_right_back = recon_down_back = 0;
		}
		else
		{
			/* If no forw vectors, current vectors equal prev. vectors. */
			if (!mb_motion_forw)
			{
				recon_right_for = m_mblock.recon_right_for_prev;
				recon_down_for = m_mblock.recon_down_for_prev;
			}
			/*
			 * Otherwise compute forw. vectors. Reset prev vectors to new values.
			 */
			else
				ComputeForwVector(&recon_right_for, &recon_down_for);

			/* If no back vectors, set back vectors to prev back vectors. */
			if (!mb_motion_back)
			{
				recon_right_back = m_mblock.recon_right_back_prev;
				recon_down_back = m_mblock.recon_down_back_prev;
			}
			/* Otherwise compute new vectors and reset prev. back vectors. */
			else
				ComputeBackVector(&recon_right_back, &recon_down_back);

			/*
			 * Store vector existence flags in structure for possible skipped
			 * macroblocks to follow.
			 */
			m_mblock.bpict_past_forw = mb_motion_forw;
			m_mblock.bpict_past_back = mb_motion_back;
		}
	}
	else
		recon_right_for = recon_down_for = recon_right_back = recon_down_back = 0;

	recon->SetMBlockInfo(m_mblock.mb_address, false, mb_motion_forw, mb_motion_back, recon_right_for, recon_down_for, m_picture->full_pel_forw_vector, recon_right_back, recon_down_back, m_picture->full_pel_back_vector);


	for (lwmUInt8 mask = 32, i = 0; i < 6; mask >>= 1, i++)
	{
		bool zero_block_flag = false;
		/* If block exists... */
		if ((m_mblock.mb_intra) || ((m_mblock.cbp & mask) != 0))
		{
			if (!ParseReconBlock(bitstream, i, recon, profileTags))
				return lwmovie::constants::PARSE_SKIP_TO_START_CODE;
		}
		else
			zero_block_flag = true;

		recon->SetBlockInfo(m_mblock.mb_address * 6 + i, zero_block_flag);
	}

	/* If D Type picture, flush marker bit. */
	if (m_picture->code_type == 4)
		bitstream->flush_bits(1);

	/* If macroblock was intracoded, set macroblock past intra address. */
	if (m_mblock.mb_intra)
		m_mblock.past_intra_addr = m_mblock.mb_address;

	if(m_mblock.mb_address == row_end_mb)
		recon->MarkRowFinished(static_cast<lwmUInt32>(m_mblock.mb_address - (m_mb_width - 1)));

	return lwmovie::constants::PARSE_OK;
}

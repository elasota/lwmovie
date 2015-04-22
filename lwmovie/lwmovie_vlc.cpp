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
#include "lwmovie_vlc.hpp"
#include "lwmovie_videotypes.hpp"


lwmUInt8 lwmovie::m1v::CDeslicerJob::DecodeMBAddrInc(CBitstream *bitstream)
{
	lwmUInt16 index;
	index = bitstream->show_bits11();
	lwmUInt8 value = lwmovie::m1v::vlc::mb_addr_inc[index].value;
	bitstream->flush_bits(lwmovie::m1v::vlc::mb_addr_inc[index].num_bits);
	return value;
}


lwmSInt32 lwmovie::m1v::CDeslicerJob::DecodeMotionVectors(CBitstream *bitstream)
{
	lwmUInt16 index;
	index = bitstream->show_bits11();
	lwmSInt8 value = lwmovie::m1v::vlc::motion_vectors[index].value;
	bitstream->flush_bits(lwmovie::m1v::vlc::motion_vectors[index].num_bits);
	return value;
}

void lwmovie::m1v::CDeslicerJob::DecodeMBTypeB(CBitstream *bitstream, lwmovie::m1v::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra)
{
	lwmUInt8 index = bitstream->show_bits6();

	const lwmovie::m1v::vlc::lwmVlcValue8 *typeEntry = lwmovie::m1v::vlc::mb_type_B + index;
	lwmUInt8 flags = typeEntry->value;

	*mb_quant = ((index == 0) ? lwmovie::m1v::constants::MB_QUANT_TYPE_ERROR : (((flags & lwmovie::m1v::vlc::MB_FLAG_QUANT) != 0) ? lwmovie::m1v::constants::MB_QUANT_TYPE_TRUE : lwmovie::m1v::constants::MB_QUANT_TYPE_FALSE));
	*mb_motion_forw = ((flags & lwmovie::m1v::vlc::MB_FLAG_MOTION_FORWARD) != 0);
	*mb_motion_back = ((flags & lwmovie::m1v::vlc::MB_FLAG_MOTION_BACKWARD) != 0);
	*mb_pattern = ((flags & lwmovie::m1v::vlc::MB_FLAG_PATTERN) != 0);
	*mb_intra = ((flags & lwmovie::m1v::vlc::MB_FLAG_INTRA) != 0);
	bitstream->flush_bits(typeEntry->num_bits);
}

void lwmovie::m1v::CDeslicerJob::DecodeMBTypeI(CBitstream *bitstream, lwmovie::m1v::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra)
{
	lwmUInt8 index = bitstream->show_bits2();

	*mb_motion_forw = false;
	*mb_motion_back = false;
	*mb_pattern = false;
	*mb_intra = true;

	if(index == 0)
		*mb_quant = lwmovie::m1v::constants::MB_QUANT_TYPE_ERROR;
	else if(index == 1)
	{
		*mb_quant = lwmovie::m1v::constants::MB_QUANT_TYPE_TRUE;
		bitstream->flush_bits(2);
	}
	else
	{
		*mb_quant = lwmovie::m1v::constants::MB_QUANT_TYPE_FALSE;
		bitstream->flush_bits(1);
	}
}

void lwmovie::m1v::CDeslicerJob::DecodeMBTypeP(CBitstream *bitstream, lwmovie::m1v::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra)
{
	lwmUInt8 index = bitstream->show_bits6();
	
	const lwmovie::m1v::vlc::lwmVlcValue8 *typeEntry = lwmovie::m1v::vlc::mb_type_P + index;
	lwmUInt8 flags = typeEntry->value;

	*mb_quant = ((index == 0) ? lwmovie::m1v::constants::MB_QUANT_TYPE_ERROR : (((flags & lwmovie::m1v::vlc::MB_FLAG_QUANT) != 0) ? lwmovie::m1v::constants::MB_QUANT_TYPE_TRUE : lwmovie::m1v::constants::MB_QUANT_TYPE_FALSE));
	*mb_motion_forw = ((flags & lwmovie::m1v::vlc::MB_FLAG_MOTION_FORWARD) != 0);
	*mb_motion_back = ((flags & lwmovie::m1v::vlc::MB_FLAG_MOTION_BACKWARD) != 0);
	*mb_pattern = ((flags & lwmovie::m1v::vlc::MB_FLAG_PATTERN) != 0);
	*mb_intra = ((flags & lwmovie::m1v::vlc::MB_FLAG_INTRA) != 0);
	bitstream->flush_bits(typeEntry->num_bits);
}

lwmUInt8 lwmovie::m1v::CDeslicerJob::DecodeCBP(CBitstream *bitstream)
{
	lwmUInt16 index = bitstream->show_bits9();
	const lwmovie::m1v::vlc::lwmVlcValue8 *vlcv = lwmovie::m1v::vlc::coded_block_pattern + index;
	lwmUInt8 coded_bp = vlcv->value;
	bitstream->flush_bits(vlcv->num_bits);
	return coded_bp;
}

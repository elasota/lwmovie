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
#include <stdlib.h>
#include "lwmovie_videotypes.hpp"
#include "lwmovie_recon_m1v.hpp"

inline void ComputeVector(lwmSInt32 *recon_right_ptr, lwmSInt32 *recon_down_ptr, lwmSInt32 *recon_right_prev, lwmSInt32 *recon_down_prev,
			lwmUInt8 f, bool full_pel_vector, lwmSInt32 motion_h_code, lwmSInt32 motion_v_code, lwmUInt8 motion_h_r, lwmUInt8 motion_v_r)
{
	/* The following procedure for the reconstruction of motion vectors        
	   is a direct and simple implementation of the instructions given  
	   in the mpeg December 1991 standard draft.                                
	*/

	lwmSInt32 comp_h_r;
	if (f == 1 || motion_h_code == 0)
		comp_h_r = 0;
	else
		comp_h_r = static_cast<lwmSInt32>(f) - 1 - motion_h_r;

	lwmSInt32 comp_v_r;
	if (f == 1 || motion_v_code == 0)
		comp_v_r = 0;
	else
		comp_v_r = f - 1 - motion_v_r;

	lwmSInt32 right_little = motion_h_code * f;
	lwmSInt32 right_big;
	if (right_little == 0)
		right_big = 0;
	else
	{
		if (right_little > 0)
		{
			right_little = right_little - comp_h_r;
			right_big = right_little - 32 * f;
		}
		else
		{
			right_little = right_little + comp_h_r;
			right_big = right_little + 32 * f;
		}
	}

	lwmSInt32 down_little = motion_v_code * f;
	lwmSInt32 down_big;
	if (down_little == 0)
		down_big = 0;
	else
	{
		if (down_little > 0)
		{
			down_little = down_little - comp_v_r;
			down_big = down_little - 32 * f;
		}
		else
		{
			down_little = down_little + comp_v_r;
			down_big = down_little + 32 * f;
		}
	}

	lwmSInt32 max = 16 * f - 1;
	lwmSInt32 min = -16 * f;

	lwmSInt32 new_vector = (*recon_right_prev) + right_little;

	if (new_vector <= max && new_vector >= min)
		*recon_right_ptr = (*recon_right_prev) + right_little;
	/* just new_vector */
	else
		*recon_right_ptr = (*recon_right_prev) + right_big;
	*recon_right_prev = *recon_right_ptr;
	if (full_pel_vector)
		*recon_right_ptr = (*recon_right_ptr) << 1;

	new_vector = (*recon_down_prev) + down_little;
	if (new_vector <= max && new_vector >= min)
		*recon_down_ptr = (*recon_down_prev) + down_little;
	/* just new_vector */
	else
		*recon_down_ptr = (*recon_down_prev) + down_big;
	*recon_down_prev = *recon_down_ptr;
	if (full_pel_vector)
		*recon_down_ptr = (*recon_down_ptr) << 1;
}

void lwmovie::m1v::CDeslicerJob::ComputeForwVector( lwmSInt32 *recon_right_for_ptr, lwmSInt32 *recon_down_for_ptr )
{
	ComputeVector(recon_right_for_ptr, recon_down_for_ptr,
		&m_mblock.recon_right_for_prev,
		&m_mblock.recon_down_for_prev,
		m_picture->forw_f,
		m_picture->full_pel_forw_vector,
		m_mblock.motion_h_forw_code, m_mblock.motion_v_forw_code,
		m_mblock.motion_h_forw_r, m_mblock.motion_v_forw_r);
}

void lwmovie::m1v::CDeslicerJob::ComputeBackVector(lwmSInt32 *recon_right_back_ptr, lwmSInt32 *recon_down_back_ptr)
{
	ComputeVector(recon_right_back_ptr, recon_down_back_ptr,
				&m_mblock.recon_right_back_prev,
				&m_mblock.recon_down_back_prev,
				m_picture->back_f,
				m_picture->full_pel_back_vector,
				m_mblock.motion_h_back_code, m_mblock.motion_v_back_code,
				m_mblock.motion_h_back_r, m_mblock.motion_v_back_r);
}

lwmovie::m1v::CVidStream::SDeslicerJobStackNode::SDeslicerJobStackNode(lwmSAllocator *alloc, lwmUInt32 mbWidth, lwmUInt32 mbHeight)
	: m_deslicerJob(mbWidth, mbHeight)
	, m_blockCursor(NULL)
	, m_alloc(alloc)
{
}

lwmovie::m1v::CVidStream::SDeslicerJobStackNode::~SDeslicerJobStackNode()
{
	if(m_blockCursor)
	{
		m_blockCursor->~IM1VBlockCursor();
		m_alloc->Free(m_blockCursor);
	}
}

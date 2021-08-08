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

inline void ComputeVector(lwmSInt32 *recon_ptr, lwmSInt32 *recon_prev, lwmUInt8 f_pow, bool full_pel_vector, lwmSInt32 motion_code, lwmUInt8 motion_r)
{
	lwmSInt32 delta;
	if (f_pow == 0 || motion_code == 0)
		delta = motion_code;
	else
	{
		if (motion_code < 0)
			delta = -((((-motion_code) - 1) * (1 << f_pow)) + static_cast<lwmSInt32>(motion_r) + 1);
		else
			delta = ((motion_code - 1) * (1 << f_pow)) + static_cast<lwmSInt32>(motion_r) + 1;
	}

	lwmSInt32 high = (16 << f_pow) - 1;
	lwmSInt32 low = -16 * (1 << f_pow);
	lwmSInt32 range = 32 << f_pow;

	lwmSInt32 new_vector = (*recon_prev) + delta;

	if (new_vector < low)
		new_vector += range;
	if (new_vector > high)
		new_vector -= range;

	(*recon_ptr) = (*recon_prev) = new_vector;
}

void lwmovie::m1v::CDeslicerJob::ComputeForwVector( lwmSInt32 *recon_right_for_ptr, lwmSInt32 *recon_down_for_ptr )
{
	ComputeVector(recon_right_for_ptr, &m_mblock.recon_right_for_prev, m_picture->forw_h_size, m_picture->full_pel_forw_vector, m_mblock.motion_h_forw_code, m_mblock.motion_h_forw_r);
	ComputeVector(recon_down_for_ptr, &m_mblock.recon_down_for_prev, m_picture->forw_v_size, m_picture->full_pel_forw_vector, m_mblock.motion_v_forw_code, m_mblock.motion_v_forw_r);
}

void lwmovie::m1v::CDeslicerJob::ComputeBackVector(lwmSInt32 *recon_right_back_ptr, lwmSInt32 *recon_down_back_ptr)
{
	ComputeVector(recon_right_back_ptr, &m_mblock.recon_right_back_prev, m_picture->back_h_size, m_picture->full_pel_back_vector, m_mblock.motion_h_back_code, m_mblock.motion_h_back_r);
	ComputeVector(recon_down_back_ptr, &m_mblock.recon_down_back_prev, m_picture->back_v_size, m_picture->full_pel_back_vector, m_mblock.motion_v_back_code, m_mblock.motion_v_back_r);
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

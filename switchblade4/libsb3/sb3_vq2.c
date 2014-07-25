/*
** Copyright (C) 2003 Eric Lasota/Orbiter Productions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <string.h>

#include "sb3_internal.h"
#include "sb3_vq_shared.h"


//extern yuvBlock2_t yuvCodebook2[256];


rc_inline void YUVCentroid2(sb3_yuvcluster2_t *inputs, uint count, sb3_yuvcluster2_t *out, uint cbSize, uint *map)
{
	uint numEntries;
	uint totals[6];
	uint i;
	uint j;

	// For each entry in the codebook...
	for(i=0;i<cbSize;i++)
	{
		// Initialize the average
		totals[0] = totals[1] = totals[2] = totals[3] = totals[4] = totals[5] = 0;
		numEntries = 0;
		for(j=0;j<count;j++)
		{
			// If an input element is mapped to this entry...
			if(map[j] == i)
			{
				// Increase the number of entries
				numEntries++;
				// And add the values to the total
				totals[0] += inputs[j].y[0];
				totals[1] += inputs[j].y[1];
				totals[2] += inputs[j].y[2];
				totals[3] += inputs[j].y[3];
				totals[4] += inputs[j].u;
				totals[5] += inputs[j].v;
			}
		}

		// Average the results to create a final component
		if(numEntries)
		{
			out[i].y[0] = (u8)(totals[0] / numEntries);
			out[i].y[1] = (u8)(totals[1] / numEntries);
			out[i].y[2] = (u8)(totals[2] / numEntries);
			out[i].y[3] = (u8)(totals[3] / numEntries);
			out[i].u = (u8)(totals[4] / numEntries);
			out[i].v = (u8)(totals[5] / numEntries);
		}
	}
}



#define GLA_FUNCTION_SCOPE
#define GLA_UNIT              sb3_yuvcluster2_t
#define GLA_NULL              NULL
#define GLA_DIFFERENCE        YUVDifference2
#define GLA_CENTROID          YUVCentroid2
#define GLA_PRINTF(a,b)
#define GLA_COPY(in,out)      memcpy((out), (in), sizeof(sb3_yuvcluster2_t))
#define GLA_PERTURB           YUVPerturb2
#define GLA2_MAX_PASSES       handle->config.refinementPasses
#define GLA_HANDLE		      sb3_encoder_t
#define GLA_EXPORT            YUVGenerateCodebooks2
#define GLA_MALLOC            malloc
#define GLA_FREE              free



// YUVGenerateCodebooks2 is DYNAMICALLY CREATED using the following include
#include "stdvq.i"

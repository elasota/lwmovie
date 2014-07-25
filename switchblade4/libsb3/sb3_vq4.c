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


void YUVPerturb4(sb3_yuvcluster4_t *in, sb3_yuvcluster4_t *out, uint step)
{
	YUVPerturb2(in->block, out->block, step);
	YUVPerturb2(in->block + 1, out->block + 1, step);
	YUVPerturb2(in->block + 2, out->block + 2, step);
	YUVPerturb2(in->block + 3, out->block + 3, step);
}

uint YUVDifference4(sb3_yuvcluster4_t *in, sb3_yuvcluster4_t *out)
{
	return YUVDifference2(in->block, out->block) + \
		YUVDifference2(in->block + 1, out->block + 1) + \
		YUVDifference2(in->block + 2, out->block + 2) + \
		YUVDifference2(in->block + 3, out->block + 3);
}

void YUVCentroid4(sb3_yuvcluster4_t *blocks, uint count, sb3_yuvcluster4_t *out, uint cbSize, uint *map)
{
	uint numEntries;
	u32 totals[24];
	uint i;
	uint j;
	uint k;
	uint l;
	uint n;
	u8 *yuv;

	// For each entry in the codebook...
	for(i=0;i<cbSize;i++)
	{
		// Initialize the average
		for(j=0;j<24;j++)
			totals[j] = 0;

		numEntries = 0;
		for(j=0;j<count;j++)
		{
			// If an input element is mapped to this entry...
			if(map[j] == i)
			{
				// Increase the number of entries
				numEntries++;

				// And add the values to the total
				n = 0;
				for(k=0;k<4;k++)
				{
					totals[n++] += blocks[j].block[k].y[0];
					totals[n++] += blocks[j].block[k].y[1];
					totals[n++] += blocks[j].block[k].y[2];
					totals[n++] += blocks[j].block[k].y[3];
					totals[n++] += blocks[j].block[k].u;
					totals[n++] += blocks[j].block[k].v;
				}
			}
		}

		// Average the results to create a final component
		if(numEntries)
		{
			n = 0;
			for(k=0;k<4;k++)
			{
				out[i].block[k].y[0] = (u8)(totals[n++] / numEntries);
				out[i].block[k].y[1] = (u8)(totals[n++] / numEntries);
				out[i].block[k].y[2] = (u8)(totals[n++] / numEntries);
				out[i].block[k].y[3] = (u8)(totals[n++] / numEntries);
				out[i].block[k].u = (u8)(totals[n++] / numEntries);
				out[i].block[k].v = (u8)(totals[n++] / numEntries);
			}
		}
	}
}



#define GLA_FUNCTION_SCOPE
#define GLA_UNIT              sb3_yuvcluster4_t
#define GLA_NULL              NULL
#define GLA_DIFFERENCE        YUVDifference4
#define GLA_CENTROID          YUVCentroid4
#define GLA_PRINTF(a,b)
#define GLA_COPY(in,out)      memcpy((out), (in), sizeof(sb3_yuvcluster4_t))
#define GLA_PERTURB           YUVPerturb4
#define GLA2_MAX_PASSES       handle->config.refinementPasses
#define GLA_EXPORT            YUVGenerateCodebooks4
#define GLA_HANDLE            sb3_encoder_t
#define GLA_MALLOC            malloc
#define GLA_FREE              free

// YUVGenerateCodebooks4 is DYNAMICALLY CREATED using the following include
#include "stdvq.i"

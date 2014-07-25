/*
** Copyright (C) 2004 Eric Lasota/Orbiter Productions
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

// NeuQuant-based vector quantizer

#include <stdlib.h>
#include <string.h>

#include "sb3_internal.h"
#include "sb3_vq_shared.h"
#include "sb3_vq.h"

#define NQ_NUM_COLORS	24
#define NQ_CHROMA_BIAS	1

#include "neuquant.i"

int YUVGenerateCodebooks4_NQ(sb3_encoder_t *handle, sb3_yuvcluster4_t *input, u32 inputCount, u32 goalCells, u32 *resultCount, sb3_yuvcluster4_t **resultElements)
{
	unsigned char *prep;
	sb3_yuvcluster4_t *cluster;
	sb3_yuvcluster4_t *result;
	unsigned char resultCodebook[256 * 24];
	unsigned char *c;
	u32 i,j;
	int numResults;

	neuquant_instance_t nqi;

	prep = malloc(24 * inputCount);
	if(!prep)
		return 0;

	c = prep;
	cluster = input;
	for(i=0;i<inputCount;i++)
	{
		for(j=0;j<4;j++)
		{
			c[0] = cluster->block[j].y[0];
			c[1] = cluster->block[j].y[1];
			c[2] = cluster->block[j].y[2];
			c[3] = cluster->block[j].y[3];
			c[4] = cluster->block[j].u;
			c[5] = cluster->block[j].v;
			c += 6;
		}
		cluster++;
	}

	numResults = 256;

	i = BasicQuant(prep, inputCount, 24, resultCodebook);
	if(i)
		numResults = i;
	else
	{
		initnet(&nqi, prep, inputCount*24, 1);
		learn(&nqi);
		unbiasnet(&nqi);
		dumpcolormap(&nqi, resultCodebook);
	}

	free(prep);

	result = malloc(sizeof(sb3_yuvcluster4_t) * 256);
	if(!result)
		return 0;

	c = resultCodebook;
	cluster = result;
	for(i=0;i<numResults;i++)
	{
		for(j=0;j<4;j++)
		{
			cluster->block[j].y[0] = c[0];
			cluster->block[j].y[1] = c[1];
			cluster->block[j].y[2] = c[2];
			cluster->block[j].y[3] = c[3];
			cluster->block[j].u = c[4];
			cluster->block[j].v = c[5];
			c += 6;
		}
		cluster++;
	}

	*resultElements = result;
	*resultCount = numResults;

	return 1;
}

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

/*
  Adaptive GLA variant 3: Heiarchial VQ (aka stdvq)

  Uses a split-then-refine method that produces codebooks of comparable quality
  to the Trilobyte encoder.

  This is a template file, how functions are defined depends on what
  is set when it is included.
*/


rc_inline void GLA_ITERATE(GLA_UNIT *input, u32 inputCount, GLA_UNIT *book, u32 bookSize, u32 *mapping, u32 *hits, u32 *diameters, u32 *diamvectors)
{
	u32 i;
	u32 j;
	u32 misses;
	u32 diff;
	u32 bestDiff;
	u32 bestMatch;
	int lhits[256];

	// Remap the diameter list
	for(i=0;i<bookSize;i++)
		diameters[i] = diamvectors[i] = hits[i] = 0;

	// First map each input to the closest codebook entry
	for(i=0;i<inputCount;i++)
	{
		bestDiff = GLA_DIFFERENCE(input + i, book);
		bestMatch = 0;
		for(j=1;j<bookSize;j++)
		{
			diff = GLA_DIFFERENCE(input + i, book + j);
			if(diff < bestDiff)
			{
				bestDiff = diff;
				bestMatch = j;
				if(!diff)
					break;
			}
		}

		// Mark this the most deviant if needed
		if(bestDiff > diameters[bestMatch])
		{
			diameters[bestMatch] = bestDiff;
			diamvectors[bestMatch] = i;
		}

		hits[bestMatch]++;

		// Assign this entry to the centroid
		mapping[i] = bestMatch;
	}

	misses = 0;
	for(i=0;i<bookSize;i++)
		lhits[i] = 0;
	for(i=0;i<inputCount;i++)
		lhits[mapping[i]]++;
	for(i=0;i<bookSize;i++)
		if(!lhits[i]) misses++;

	GLA_PRINTF("%i misses ", misses);

	// Then re-map the centroids
	GLA_CENTROID(input, inputCount, book, bookSize, mapping);
}


GLA_FUNCTION_SCOPE int GLA_EXPORT(GLA_HANDLE *handle, GLA_UNIT *input, u32 inputCount, u32 goalCells, u32 *resultCount, GLA_UNIT **resultElements)
{
	GLA_UNIT *output=GLA_NULL;
	GLA_UNIT *output2=GLA_NULL;
	u32 *diameters;
	u32 *diamVectors;
	u32 *hits;
	GLA_UNIT *tempOutput;

	u32 *mapping = GLA_NULL;

	u32 i;
	u32 j;
	u32 splitTotal;

	u32 bestDiff;
	u32 bestMatch;

	u32 step;

	u32 cellGoal;

	mapping = GLA_MALLOC(inputCount * sizeof(u32));
	output = GLA_MALLOC(goalCells * sizeof(GLA_UNIT));
	output2 = GLA_MALLOC(goalCells * sizeof(GLA_UNIT));
	diameters = GLA_MALLOC(goalCells * sizeof(u32));
	diamVectors = GLA_MALLOC(goalCells * sizeof(u32));
	hits = GLA_MALLOC(goalCells * sizeof(u32));

	if(!mapping || !output || !output2 || !diameters || !diamVectors || !hits)
	{
		if(mapping)
			GLA_FREE(mapping);
		if(output)
			GLA_FREE(output);
		if(output2)
			GLA_FREE(output2);
		if(diameters)
			GLA_FREE(diameters);
		if(diamVectors)
			GLA_FREE(diamVectors);
		if(hits)
			GLA_FREE(hits);

		return 0;
	}

	step = 0;

	// Map all input units to the same cell to create
	// a centroid for the entire input set
	for(i=0;i<inputCount;i++)
		mapping[i] = 0;

	GLA_CENTROID(input, inputCount, output, 1, mapping);

	for(cellGoal=1;cellGoal<goalCells;cellGoal*=2)
	{
		GLA_PRINTF("stdvq: Building codebook size %i...", cellGoal*2);
		// Split each cell into 2 new cells, then perturb them.
		// Copy the original in so it can be perturbed as well if needed
		for(i=0;i<cellGoal;i++)
		{
			GLA_COPY(output + i, output2 + (i*2));
			GLA_PERTURB(output2 + (i*2), output2 + (i*2+1), step);
		}

		// Swap output buffers
		tempOutput = output2;
		output2 = output;
		output = tempOutput;

		// Run GLA passes on the new output
		GLA_PRINTF("Refining... ", 0);
		for(i=0;i<GLA2_MAX_PASSES;i++)
			GLA_ITERATE(input, inputCount, output, cellGoal*2, mapping, hits, diameters, diamVectors);
		GLA_PRINTF("Done\n", 0);

		step++;
	}

	// Split high-diameter cells to minimize waste
	splitTotal = 0;
	for(i=0;i<cellGoal;i++)
	{
		if(hits[i] == 0)
		{
			// This cell is dead.  Replace it with the highest-diameter input
			bestDiff = diameters[0];
			bestMatch = 0;

			for(j=0;j<cellGoal;j++)
			{
				if(diameters[j] > bestDiff && diameters[j] != 0)
				{
					bestDiff = diameters[j];
					bestMatch = j;
				}
			}

			// If no split could be made, ignore...
			if(bestDiff == 0)
				break;

			splitTotal++;

			diameters[bestMatch] = 0;
			GLA_COPY(input + diamVectors[bestMatch], output + i);
		}
	}

	GLA_PRINTF("%i splits made.\n", splitTotal);

	GLA_FREE(diameters);
	GLA_FREE(diamVectors);
	GLA_FREE(hits);
	GLA_FREE(output2);
	GLA_FREE(mapping);

	*resultCount = cellGoal;
	*resultElements = output;

	return 1;
}


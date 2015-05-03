#include <stdlib.h>
#include "SB4Encoder.hpp"
#include "SB4ELBG2.hpp"

static int compareInt6(const void *a, const void *b)
{
	const int *ia = static_cast<const int *>(a);
	const int *ib = static_cast<const int *>(b);

	int at = 0;
	int bt = 0;

	for (lwmUInt32 i = 0; i<4; i++)
	{
		at += *ia++;
		bt += *ib++;
	}

	return at - bt;
}

static int compareInt24(const void *a, const void *b)
{
	const int *ia = (const int *)a;
	const int *ib = (const int *)b;

	int at = 0;
	int bt = 0;

	for (lwmUInt32 i = 0; i<24; i += 6)
	{
		for (lwmUInt32 j = 0; j<4; j++)
		{
			at += *ia++;
			bt += *ib++;
		}
		ia += 2;
		ib += 2;
	}

	return at - bt;
}

static bool ShouldScrub4x4(const lwmUInt32 *subcelDists, lwmUInt32 threshold)
{
	lwmUInt32 combinedDist = 0;
	for (lwmLargeUInt i = 0; i < 24; i++)
		combinedDist += subcelDists[i];
	return combinedDist < threshold;
}

static bool ShouldScrub2x2(const lwmUInt32 *subcelDists, lwmUInt32 threshold)
{
	lwmUInt32 combinedDist = 0;
	for (lwmLargeUInt i = 0; i < 6; i++)
		combinedDist += subcelDists[i];
	return combinedDist < threshold;
}

// Culls any VQ input where a different coding option has lower distortion than
// the target threshold.  Because subcels are 4x4, this creates a point set
// for both 4x4 and 2x2 codebooks.
static void ScrubPoints(int *scrubbedPoints, const int *points, const lwmUInt32 *subcelDists, lwmUInt32 *pNumClusters, lwmUInt32 threshold)
{
	unsigned int numSubcels = (*pNumClusters) / 4;
	unsigned int unscrubbedClusters = 0;

	while (numSubcels--)
	{
		if ((*subcelDists) > threshold)
		{
			for (lwmUInt32 i = 0; i<6 * 4; i++)
				*scrubbedPoints++ = *points++;
			unscrubbedClusters += 4;
		}
	}
	*pNumClusters = unscrubbedClusters;
}



void SB4RoQEncoder::MakeCodebooks(lwmUInt8 *cb2, lwmUInt8 *cb4, SB4RandomState *rnd, const SB4Image *prevImg, const SB4Image *img, const lwmUInt32 *subcelPredDists, lwmUInt32 threshold)
{
	lwmUInt32 w = img->Width();
	lwmUInt32 h = img->Height();

	lwmUInt32 numClusters = (w / 2) * (h / 2);

	std::vector<int> cb2int;
	std::vector<int> cb4int;
	cb4int.resize(256 * 24);
	cb2int.resize(256 * 6);

	std::vector<int> points;
	std::vector<int> scrubbedPoints;
	points.resize(numClusters * 6);
	scrubbedPoints.resize(numClusters * 6);

	int *ptr;
	ptr = &points[0];

	// Map in CB4 cluster order to allow the same inputs to be reused
	for (lwmUInt32 blocky = 0; blocky < h; blocky += 4)
	{
		for (lwmUInt32 blockx = 0; blockx < w; blockx += 4)
		{
			for (lwmUInt32 suby = 0; suby < 4; suby += 2)
			{
				for (lwmUInt32 subx = 0; subx < 4; subx += 2)
				{
					lwmUInt32 x = blockx + subx;
					lwmUInt32 y = blocky + suby;

					SB4Block<2> block2(img, x, y);

					ptr[0] = block2.Row(0, 0)[0];
					ptr[1] = block2.Row(0, 0)[1];
					ptr[2] = block2.Row(0, 1)[0];
					ptr[3] = block2.Row(0, 1)[1];

					// Weighted chroma
					ptr[4] = (block2.Row(1, 0)[0] + block2.Row(1, 0)[1] + block2.Row(1, 1)[0] + block2.Row(1, 1)[1]) / 2;
					ptr[5] = (block2.Row(2, 0)[0] + block2.Row(2, 0)[1] + block2.Row(2, 1)[0] + block2.Row(2, 1)[1]) / 2;

					ptr += 6;
				}
			}
		}
	}

	std::vector<int> nearest;
	if (prevImg)
		ptr = &scrubbedPoints[0];
	else
		ptr = &points[0];

	// Generate codebooks
	nearest.resize(numClusters);
	if (prevImg)
		ScrubPoints(&scrubbedPoints[0], &points[0], subcelPredDists, &numClusters, threshold);
	if (numClusters == 0)
	{
		// Always have at least one quad cluster
		numClusters = 4;
		memcpy(&scrubbedPoints[0], &points[0], sizeof(int) * 24);
	}
	ff_init_elbg(ptr, 6, numClusters, &cb2int[0], 256, 1, &nearest[0], rnd);
	ff_do_elbg(ptr, 6, numClusters, &cb2int[0], 256, 1, &nearest[0], rnd);

	qsort(&cb2int[0], 256, 6 * sizeof(int), compareInt6);

	nearest.resize(numClusters / 4);
	ff_init_elbg(ptr, 24, numClusters / 4, &cb4int[0], 256, 1, &nearest[0], rnd);
	ff_do_elbg(ptr, 24, numClusters / 4, &cb4int[0], 256, 1, &nearest[0], rnd);

	qsort(&cb4int[0], 256, 24 * sizeof(int), compareInt24);

	// Store and unpack CB2
	SB4Block<2> cb2unpacked[256];
	for (lwmUInt32 i = 0; i<1536; i += 6)
	{
		UnpackCB2(&cb2int[i + 0], &cb2unpacked[i / 6]);

		cb2[i + 0] = cb2unpacked[i / 6].Row(0, 0)[0];
		cb2[i + 1] = cb2unpacked[i / 6].Row(0, 0)[1];
		cb2[i + 2] = cb2unpacked[i / 6].Row(0, 1)[0];
		cb2[i + 3] = cb2unpacked[i / 6].Row(0, 1)[1];
		cb2[i + 4] = cb2unpacked[i / 6].Row(1, 0)[0];
		cb2[i + 5] = cb2unpacked[i / 6].Row(2, 0)[0];
	}

	// Index CB4 to CB2 and store
	for (lwmUInt32 i = 0; i<6144; i += 6)
	{
		SB4Block<2> block2;
		UnpackCB2(&cb4int[i + 0], &block2);
		cb4[i / 6] = block2.IndexList(cb2unpacked, 256, 0);
	}
}

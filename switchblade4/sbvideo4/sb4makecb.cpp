#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vfw.h>
#include "../sb4sdl.h"
#include "../sb4.h"
#include "../sb4image.h"
#include "../sb4random.h"
#include "../elbg.h"


#include <string>

int compareInt6(const void *a, const void *b)
{
	const int *ia = (const int *)a;
	const int *ib = (const int *)b;

	unsigned int i;
	int at = 0;
	int bt = 0;

	for(i=0;i<4;i++)
	{
		at += *ia++;
		bt += *ib++;
	}

	return at-bt;
}

int compareInt24(const void *a, const void *b)
{
	const int *ia = (const int *)a;
	const int *ib = (const int *)b;

	unsigned int i,j;
	int at = 0;
	int bt = 0;

	for(i=0;i<24;i+=6)
	{
		for(j=0;j<4;j++)
		{
			at += *ia++;
			bt += *ib++;
		}
		ia+=2;
		ib+=2;
	}

	return at-bt;
}

void scrub_points(int *scrubbedPoints, const int *points, const unsigned int *subcelDists, unsigned int *pNumClusters, unsigned int threshold)
{
	unsigned int i, diff;
	unsigned int scrubbed = 0;

	unsigned int numSubcels = (*pNumClusters) / 4;
	unsigned int unscrubbedClusters = 0;

	while(numSubcels--)
	{
		if((*subcelDists) > threshold)
		{
			for(i=0;i<24;i++)
				*scrubbedPoints++ = points[i];
			unscrubbedClusters += 4;
		}
		else
			scrubbed++;
		subcelDists++;
		points += 24;
	}
	*pNumClusters = unscrubbedClusters;
	//printf("   VQ: %i points scrubbed\n", scrubbed);
}

void MakeCodebooks(unsigned char *cb2, unsigned char *cb4, AVRandomState *rnd, const SB4Image *prevImg, const SB4Image *img, const unsigned int *subcelPredDists, unsigned int threshold)
{
	unsigned int w, h, numClusters, blockx, blocky, subx, suby, x, y, i;
	SB4Block<2> block2;
	std::vector<int> points;
	std::vector<int> scrubbedPoints;
	std::vector<int> nearest;
	std::vector<int> cb2int;
	std::vector<int> cb4int;
	int *ptr;
	unsigned char yuv[3][2][2];
	SB4Block<2> cb2unpacked[256];
	SB4Block<4> cb4unpacked[256];

	w = img->Width();
	h = img->Height();

	numClusters = (w/2) * (h/2);

	cb4int.resize(256 * 24);
	cb2int.resize(256 * 6);

	points.resize( numClusters * 6 );
	scrubbedPoints.resize( numClusters * 6 );
	ptr = &points[0];

	// Map in CB4 cluster order to allow the same inputs to be reused
	for(blocky=0;blocky<h;blocky+=4)
	{
		for(blockx=0;blockx<w;blockx+=4)
		{
			for(suby=0;suby<4;suby+=2)
			{
				for(subx=0;subx<4;subx+=2)
				{
					x = blockx+subx;
					y = blocky+suby;

					block2 = SB4Block<2>(img, x, y);

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

	if(prevImg)
		ptr = &scrubbedPoints[0];
	else
		ptr = &points[0];

	// Generate codebooks
	nearest.resize(numClusters);
	if(prevImg)
		scrub_points(&scrubbedPoints[0], &points[0], subcelPredDists, &numClusters, threshold);
	if(numClusters == 0)
	{
		// Always have at least one quad cluster
		numClusters = 4;
		memcpy(&scrubbedPoints[0], &points[0], sizeof(int) * 24);
	}
	ff_init_elbg(ptr, 6, numClusters, &cb2int[0], 256, 1, &nearest[0], rnd);
	ff_do_elbg(ptr, 6, numClusters, &cb2int[0], 256, 1, &nearest[0], rnd);

	qsort(&cb2int[0], 256, 6*sizeof(int), compareInt6);

	nearest.resize(numClusters / 4);
	//if(prevImg)
	//	scrub_points(&scrubbedPoints[0], &points[0], subcelPreds, numClusters, threshold * 4);
	ff_init_elbg(ptr, 24, numClusters/4, &cb4int[0], 256, 1, &nearest[0], rnd);
	ff_do_elbg(ptr, 24, numClusters/4, &cb4int[0], 256, 1, &nearest[0], rnd);

	qsort(&cb4int[0], 256, 24*sizeof(int), compareInt24);

	// Store and unpack CB2
	for(i=0;i<1536;i+=6)
	{
		UnpackCB2(false, &cb2int[i+0], &cb2unpacked[i/6]);

		cb2[i+0] = cb2unpacked[i/6].Row(0, 0)[0];
		cb2[i+1] = cb2unpacked[i/6].Row(0, 0)[1];
		cb2[i+2] = cb2unpacked[i/6].Row(0, 1)[0];
		cb2[i+3] = cb2unpacked[i/6].Row(0, 1)[1];
		cb2[i+4] = cb2unpacked[i/6].Row(1, 0)[0];
		cb2[i+5] = cb2unpacked[i/6].Row(2, 0)[0];
	}

	// Index CB4 to CB2 and store
	for(i=0;i<6144;i+=6)
	{
		UnpackCB2(false, &cb4int[i+0], &block2);
		cb4[i/6] = block2.IndexList(NULL, cb2unpacked, 256, 0);
	}
}

/*
 * RoQ Video Encoder.
 *
 * Copyright (C) 2007 Vitor <vitor1001@gmail.com>
 * Copyright (C) 2004-2007 Eric Lasota
 *	Based on RoQ specs (C) 2001 Tim Ferguson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file roqvideoenc.c
 * Id RoQ encoder by Vitor. Based on the Switchblade3 library and the
 * Switchblade3 FFmpeg glue by Eric Lasota.
 */

/*
 * COSTS:
 * Level 1:
 *  SKIP - 2 bits
 *  MOTION - 10 bits
 *  CODEBOOK - 10 bits
 *  SUBDIVIDE - 2 + combined subcel cost
 *
 * Level 2:
 *  SKIP - 2 bits
 *  MOTION - 10 bits
 *  CODEBOOK - 10 bits
 *  SUBDIVIDE - 34 bits
 *
 * Maximum cost: 138 bits per cel
 *
 * Proper evaluation requires LCD fraction comparison, which requires
 * Squared Error (SE) loss * savings increase
 *
 * Maximum savings increase: 136 bits
 * Maximum SE loss without overflow: 31580641
 * Components in 8x8 supercel: 192
 * Maximum SE precision per component: 164482
 *	>65025, so no truncation is needed (phew)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sb4.h"
#include "sb4videoenc.h"
#include "sb4image.h"
#include "sb4random.h"

#define RoQ_INFO			0x1001
#define RoQ_QUAD_CODEBOOK	0x1002
#define RoQ_QUAD_VQ			0x1011
#define RoQ_QUAD_DVQ		0x1014
#define RoQ_SOUND_MONO		0x1020
#define RoQ_SOUND_STEREO	0x1021

#define RoQ_ID_MOT			  0x00
#define RoQ_ID_FCC			  0x01
#define RoQ_ID_SLD			  0x02
#define RoQ_ID_CCC			  0x03


template <unsigned int _D> static unsigned int block_sse(const SB4Image *img1, const SB4Image *img2, int x1, int y1, int x2, int y2)
{
	SB4Block<_D> b1(img1, x1, y1);
	SB4Block<_D> b2(img2, x2, y2);

	return b1.SquareDiff(b2);
}

template <unsigned int _D> static unsigned int eval_motion_dist(sb4videoenc_t *enc, int x, int y, int mx, int my)
{
	int id = _D;
	if (mx < -7 || mx > 8)
		return INT_MAX;

	if (my < -7 || my > 8)
		return INT_MAX;

	if ((x + mx < 0) || (x + mx > enc->width - id))
		return INT_MAX;

	if ((y + my < 0) || (y + my > enc->height - id))
		return INT_MAX;


	return block_sse<_D>(enc->frame_to_enc, enc->last_frame, x, y,
					 x+mx, y+my);
}

typedef struct
{
	unsigned int eval_dist[4];

	int subCels[4];

	int motionX, motionY;
	int cbEntry;
} roq_subcel_evaluation_t;

typedef struct
{
	unsigned int eval_dist[3];

	roq_subcel_evaluation_t subCels[4];

	int motionX, motionY;
	int cbEntry;

	int sourceX, sourceY;
} cel_evaluation_t;

typedef struct
{
	int evalType;
	int subEvalTypes[4];

	unsigned long codeConsumption;	// 2-bit typecodes
	unsigned long byteConsumption;	// 8-bit arguments
	unsigned long combinedBitConsumption;

	unsigned long dist;

	int allowed;
} roq_possibility_t;


/**
 * 4*4*4*4 subdivide types + 3 non-subdivide main types
 */
#define ROQ_MAX_POSSIBILITIES	(4*4*4*4+3)

typedef struct
{
	roq_possibility_t p[ROQ_MAX_POSSIBILITIES];
	roq_possibility_t *sorted[ROQ_MAX_POSSIBILITIES];
} possibility_list_t;

struct roq_codebooks_t
{
	int numCB4;
	int numCB2;
	SB4Block<2> unpacked_cb2[256];
	SB4Block<4> unpacked_cb4[256];
	SB4Block<8> unpacked_cb4_enlarged[256];

	inline roq_codebooks_t()
	{
		memset(this, 0, sizeof(*this));
	}
};


typedef struct
{
	int distIncrease;
	int bitCostDecrease;

	int valid;

	possibility_list_t *list;
	int plistIndex;

	cel_evaluation_t *eval;
} sort_option_t;


typedef struct
{
	int cb2UsageCount[256];
	int cb4UsageCount[256];
	unsigned char cb4Indexes[256][4];

	int usedCB2;
	int usedCB4;

	int numCodes;
	int numArguments;
} roq_sizecalc_t;

/**
 * Temporary vars
 */
struct roq_tempdata_t
{
	std::vector<cel_evaluation_t> cel_evals;
	std::vector<possibility_list_t> plists;
//	roq_cell4 *yuvClusters;
	std::vector<sort_option_t> sortOptions;
	std::vector<sort_option_t *> sortOptionsSorted;
	std::vector<unsigned int> subcelPredDists;

	std::vector<unsigned char> outbuffer;

	int f2i4[256];
	int i2f4[256];
	int f2i2[256];
	int i2f2[256];

	int mainChunkSize;

	int numCB4;
	int numCB2;

	roq_codebooks_t codebooks;

	inline roq_tempdata_t()
		: mainChunkSize(0)
		, numCB4(0)
		, numCB2(0)
	{
		memset(f2i4, 0, sizeof(f2i4));
		memset(i2f4, 0, sizeof(i2f4));
		memset(f2i2, 0, sizeof(f2i2));
		memset(i2f2, 0, sizeof(i2f2));
	}
};

static void free_temp_data(roq_tempdata_t *tempData)
{
//	av_free(tempData->cel_evals);
//	av_free(tempData->plists);
//	av_free(tempData->yuvClusters);
//	av_free(tempData->sortOptions);
//	av_free(tempData->sortOptionsSorted);
//	av_free(tempData->outbuffer);
//	av_free(tempData->closest_cb2);
}

/**
 * Initializes cel evaluators and sets their source coordinates
 */
int create_cel_evals(sb4videoenc_t *enc, roq_tempdata_t *tempData)
{
	int n,x,y;

	tempData->cel_evals.resize(enc->width*enc->height/64);

	/* Map to the ROQ quadtree order */
	n = 0;
	for (y=0; y<enc->height; y+=16) {
		for (x=0; x<enc->width; x+=16) {
			tempData->cel_evals[n].sourceX = x;
			tempData->cel_evals[n++].sourceY = y;

			tempData->cel_evals[n].sourceX = x+8;
			tempData->cel_evals[n++].sourceY = y;

			tempData->cel_evals[n].sourceX = x;
			tempData->cel_evals[n++].sourceY = y+8;

			tempData->cel_evals[n].sourceX = x+8;
			tempData->cel_evals[n++].sourceY = y+8;
		}
	}

	return 1;
}

// TODO: Allow MOT with fsk == 1 on second frame only, use first frame
static void initialize_single_possibility_list(possibility_list_t *plist, int fsk)
{
	int i,j,k,l;
	int n=3;
	int firstAllowed;
	int lastAllowed = 3;

	plist->p[0].evalType = RoQ_ID_FCC;
	plist->p[0].allowed = (fsk >= 1);

	plist->p[1].evalType = RoQ_ID_MOT;
	plist->p[1].allowed = (fsk >= 2);

	plist->p[2].evalType = RoQ_ID_SLD;
	plist->p[2].allowed = 1;

	for(i=3;i<259;i++)
		plist->p[i].allowed = 0;

	/**
	 * This exploits the enumeration of RoQ tags
	 *  0-3 = Allow CCC(3), SLD(2), FCC(1), MOT(0)
	 *  1-3 = Allow CCC(3), SLD(2), FCC(1)
	 *  2-3 = Allow CCC(3), SLD(2)
	 */

	if (fsk >= 2)
		firstAllowed = 0;
	else if (fsk >= 1)
		firstAllowed = 1;
	else
		firstAllowed = 2;

	for (i=firstAllowed; i<=lastAllowed; i++)
		for (j=firstAllowed; j<=lastAllowed; j++)
			for (k=firstAllowed; k<=lastAllowed; k++)
				for (l=firstAllowed; l<=lastAllowed; l++) {
					plist->p[n].evalType = RoQ_ID_CCC;
					plist->p[n].allowed = 1;
					plist->p[n].subEvalTypes[0] = i;
					plist->p[n].subEvalTypes[1] = j;
					plist->p[n].subEvalTypes[2] = k;
					plist->p[n].subEvalTypes[3] = l;
					n++;
				}

	while(n < ROQ_MAX_POSSIBILITIES)
		plist->p[n++].allowed = 0;
}

/**
 * Initializes all possibility lists
 */
int create_possibility_lists(sb4videoenc_t *enc, roq_tempdata_t *tempData)
{
	possibility_list_t *plists;
	int max, fsk;

	max = enc->width*enc->height>>6;

	tempData->plists.resize(max);
	plists = &tempData->plists[0];

	fsk = enc->framesSinceKeyframe;
	while (max--)
		initialize_single_possibility_list(plists++, fsk);

	return 1;
}



#define EVAL_MOTION(MOTION) \
	do { \
		diff = eval_motion_dist<_D>(enc, j, i, (MOTION).dx, \
								 (MOTION).dy); \
			\
		if (diff < lowestdiff) { \
			lowestdiff = diff; \
			bestpick = MOTION; \
		} \
	} while(0)


template <unsigned int _D> static void motion_search(sb4videoenc_t *enc, bool fullSearch)
{
	motion_vect offsets[9] = {
		{ 0, 0},
		{ 0,-1},
		{ 0, 1},
		{-1, 0},
		{ 1, 0},
		{-1, 1},
		{ 1,-1},
		{-1,-1},
		{ 1, 1},
	};

	int diff, lowestdiff, oldbest;
	int off[3];
	motion_vect bestpick;
	int i, j, k, offset;

	motion_vect *last_motion;
	motion_vect *this_motion;
	motion_vect vect, vect2;

	int max=(enc->width/_D)*(enc->height/_D);

	if (_D == 4) {
		last_motion = enc->last_motion4;
		this_motion = enc->this_motion4;
	} else {
		last_motion = enc->last_motion8;
		this_motion = enc->this_motion8;
	}

	for (i=0; i<enc->height; i+=_D)
		for (j=0; j<enc->width; j+=_D) {
			lowestdiff = eval_motion_dist<_D>(enc, j, i, 0, 0);
			bestpick.dx = 0;
			bestpick.dy = 0;

			if(fullSearch)
			{
				for (int y=-7;y<=8;y++)
				{
					for (int x=-7;x<=8;x++)
					{
						vect.dx = x;
						vect.dy = y;
						EVAL_MOTION(vect);
					}
				}
			}
			else
			{
				if (_D == 4)
					EVAL_MOTION(enc->this_motion8[(i/8)*(enc->width/8) + j/8]);

				offset = (i/_D)*(enc->width/_D) + j/_D;
				if (offset < max && offset >= 0)
					EVAL_MOTION(last_motion[offset]);

				offset++;
				if (offset < max && offset >= 0)
					EVAL_MOTION(last_motion[offset]);

				offset = (i/_D + 1)*(enc->width/_D) + j/_D;
				if (offset < max && offset >= 0)
					EVAL_MOTION(last_motion[offset]);

				off[0]= (i/_D)*(enc->width/_D) + j/_D - 1;
				off[1]= off[0] - (enc->width/_D) + 1;
				off[2]= off[1] + 1;
				if(i){
					vect.dx= mid_pred(this_motion[off[0]].dx, this_motion[off[1]].dx, this_motion[off[2]].dx);
					vect.dy= mid_pred(this_motion[off[0]].dy, this_motion[off[1]].dy, this_motion[off[2]].dy);
					EVAL_MOTION(vect);
					for(k=0; k<3; k++)
						EVAL_MOTION(this_motion[off[k]]);
				}else if(j)
					EVAL_MOTION(this_motion[off[0]]);

				vect = bestpick;

				oldbest = -1;
				while (oldbest != lowestdiff) {
					oldbest = lowestdiff;
					for (k=0; k<9; k++) {
						vect2 = vect;
						vect2.dx += offsets[k].dx;
						vect2.dy += offsets[k].dy;
						EVAL_MOTION(vect2);
					}
					vect = bestpick;
				}
			}
			offset = (i/_D)*(enc->width/_D) + j/_D;
			this_motion[offset] = bestpick;
		}

}

/**
 * Gets distortion for all options available to a subcel
 */
static void gather_data_for_subcel(roq_subcel_evaluation_t *subcel, int x,
								   int y, sb4videoenc_t *enc, roq_tempdata_t *tempData, bool motionPass)
{
	SB4Block<4> mb4;
	SB4Block<2> mb2;
	SB4Block<4> pmb4;
	SB4Block<2> pmb2;
	unsigned int diff;
	int cluster_index;
	int i;
	int ssc_offsets[4][2] = {
		{0,0},
		{2,0},
		{0,2},
		{2,2},
	};

	if (enc->framesSinceKeyframe >= 1) {
		if(motionPass)
		{
			subcel->motionX = enc->this_motion4[y*enc->width/16 + x/4].dx;
			subcel->motionY = enc->this_motion4[y*enc->width/16 + x/4].dy;
			subcel->eval_dist[RoQ_ID_FCC] =
				eval_motion_dist<4>(enc, x, y,
							 enc->this_motion4[y*enc->width/16 + x/4].dx,
							 enc->this_motion4[y*enc->width/16 + x/4].dy);
		}
	} else
		subcel->eval_dist[RoQ_ID_FCC] = INT_MAX;

	if (enc->framesSinceKeyframe >= 2)
	{
		if(motionPass)
		{
			subcel->eval_dist[RoQ_ID_MOT] = block_sse<4>(enc->frame_to_enc,
												 enc->current_frame, x, y, x, y);
		}
	}

	if(motionPass)
	{
		return;
	}

	cluster_index = y*enc->width/16 + x/4;

	mb4.LoadFromImage(enc->frame_to_enc, x, y);
	if(!enc->delta_encode)
	{
		subcel->cbEntry = mb4.IndexList(NULL, tempData->codebooks.unpacked_cb4, 256, mb4.LumaAverage(), (unsigned int *)&subcel->eval_dist[RoQ_ID_SLD]);
	}
	else
	{
		pmb4.LoadFromImage(enc->last_frame, x, y);
		subcel->cbEntry = mb4.IndexList(&pmb4, tempData->codebooks.unpacked_cb4, 256, mb4.LumaAverage(), (unsigned int *)&subcel->eval_dist[RoQ_ID_SLD]);
	}

	subcel->eval_dist[RoQ_ID_CCC] = 0;
	for(i=0;i<4;i++)
	{
		mb2.LoadFromImage(enc->frame_to_enc, x + ssc_offsets[i][0], y + ssc_offsets[i][1]);
		if(!enc->delta_encode)
		{
			if(!motionPass)
				subcel->subCels[i] = mb2.IndexList(NULL, tempData->codebooks.unpacked_cb2, 256, mb2.LumaAverage(), &diff);
		}
		else
		{
			pmb2.LoadFromImage(enc->last_frame, x + ssc_offsets[i][0], y + ssc_offsets[i][1]);
			subcel->subCels[i] = mb2.IndexList(&pmb2, tempData->codebooks.unpacked_cb2, 256, mb2.LumaAverage(), &diff);
		}

		subcel->eval_dist[RoQ_ID_CCC] += diff;
	}
}

/**
 * Gets distortion for all options available to a cel
 */
static void gather_data_for_cel(cel_evaluation_t *cel, sb4videoenc_t *enc,
								roq_tempdata_t *tempData, bool motionPass)
{
	SB4Block<8> mb8;
	SB4Block<8> pmb8;

	int index = cel->sourceY*enc->width/64 + cel->sourceX/8;

	if (enc->framesSinceKeyframe >= 1) {
		cel->motionX = enc->this_motion8[index].dx;
		cel->motionY = enc->this_motion8[index].dy;
		if(motionPass)
			cel->eval_dist[RoQ_ID_FCC] = eval_motion_dist<8>(enc, cel->sourceX, cel->sourceY,
							 enc->this_motion8[index].dx, enc->this_motion8[index].dy);
	}

	if (enc->framesSinceKeyframe >= 2)
	{
		if(motionPass)
			cel->eval_dist[RoQ_ID_MOT] = block_sse<8>(enc->frame_to_enc,
										   enc->current_frame,
										   cel->sourceX, cel->sourceY,
										   cel->sourceX, cel->sourceY);
	}

	mb8.LoadFromImage(enc->frame_to_enc, cel->sourceX, cel->sourceY);

	if(!enc->delta_encode)
	{
		if(!motionPass)
			cel->cbEntry = mb8.IndexList(NULL, tempData->codebooks.unpacked_cb4_enlarged, 256, mb8.LumaAverage(), &cel->eval_dist[RoQ_ID_SLD]);
	}
	else
	{
		pmb8.LoadFromImage(enc->last_frame, cel->sourceX, cel->sourceY);
		cel->cbEntry = mb8.IndexList(&pmb8, tempData->codebooks.unpacked_cb4_enlarged, 256, mb8.LumaAverage(), &cel->eval_dist[RoQ_ID_SLD]);
	}

	gather_data_for_subcel(cel->subCels + 0, cel->sourceX+0, cel->sourceY+0, enc, tempData, motionPass);
	gather_data_for_subcel(cel->subCels + 1, cel->sourceX+4, cel->sourceY+0, enc, tempData, motionPass);
	gather_data_for_subcel(cel->subCels + 2, cel->sourceX+0, cel->sourceY+4, enc, tempData, motionPass);
	gather_data_for_subcel(cel->subCels + 3, cel->sourceX+4, cel->sourceY+4, enc, tempData, motionPass);

}

/**
 * Gathers SE data for all feasible possibilities
 */
int gather_cel_data(sb4videoenc_t *enc, roq_tempdata_t *tempData, bool motionPass)
{
	int max;
	cel_evaluation_t *cel_evals;

	max = enc->width*enc->height/64;
	cel_evals = &tempData->cel_evals[0];

	while (max--) {
		gather_data_for_cel(cel_evals, enc, tempData, motionPass);
		cel_evals++;
	}

	return 1;
}

/**
 * Loads possibility lists with actual data for one block,
 * assigning all possibilities a cached SE and bit consumption
 */
static void gather_possibility_data_for_block(possibility_list_t *plist,
											  cel_evaluation_t *celEval)
{
	int i,j;
	roq_possibility_t *p;

	for (i=0; i<ROQ_MAX_POSSIBILITIES; i++) {
		p = plist->p+i;
		if (!p->allowed)
			continue;

		if (p->evalType == RoQ_ID_MOT) {
			p->codeConsumption = 1;
			p->byteConsumption = 0;
			p->dist = celEval->eval_dist[RoQ_ID_MOT];
		} else if (p->evalType == RoQ_ID_FCC || p->evalType == RoQ_ID_SLD) {
			p->codeConsumption = 1;
			/* 3.11 - Was = 0, oops */
			p->byteConsumption = 1;
			p->dist = celEval->eval_dist[p->evalType];
		} else { //if (p->evalType == RoQ_ID_CCC)
			p->codeConsumption = 5;	  // 1 for main code, 4 for the subcodes
			p->byteConsumption = 0;
			p->dist = 0;

			for (j=0; j<4; j++) {
				p->dist += celEval->subCels[j].eval_dist[p->subEvalTypes[j]];

				if (p->subEvalTypes[j] == RoQ_ID_FCC ||
					p->subEvalTypes[j] == RoQ_ID_SLD)
					p->byteConsumption++;
				else if (p->subEvalTypes[j] == RoQ_ID_CCC)
					p->byteConsumption += 4;
			}
		}

		p->combinedBitConsumption = p->codeConsumption + 4*p->byteConsumption;
	}

}

/**
 * Builds a possibility list such that each entry is a less-efficient use of
 * the bit budget than the previous one.  The first entry is the
 * lowest-distortion entry, or if tied, the one that consumes the fewest bits.
 * TODO: Bias towards options that don't use the codebook
 */
static void sort_possibility_data(possibility_list_t *plists)
{
   int i;
   int nvp;
   roq_possibility_t *base;
   roq_possibility_t *best;
   roq_possibility_t *cmp;

   /* Find the best-quality possibility.  If there's a tie, find the cheapest. */
   best = NULL;
   for(i=0;i<ROQ_MAX_POSSIBILITIES;i++)
   {
	   cmp = plists->p + i;
	   if(!cmp->allowed)
		   continue;

	   if(!best || plists->p[i].dist < best->dist ||
		   (cmp->dist == best->dist && cmp->combinedBitConsumption < best->combinedBitConsumption))
		   best = cmp;
   }

   nvp = 1;
   plists->sorted[0] = best;

   while(1)
   {
	   base = plists->sorted[nvp-1];

	   best = NULL;
	   /* Find the best exchange for bit budget from the previous best */
	   for(i=0;i<ROQ_MAX_POSSIBILITIES;i++)
	   {
		   cmp = plists->p + i;
		   if(!cmp->allowed || cmp->dist <= base->dist || cmp->combinedBitConsumption >= base->combinedBitConsumption)
			   continue;
		   if(!best ||
			   ((cmp->dist - base->dist) * (base->combinedBitConsumption - best->combinedBitConsumption)
			   < (best->dist - base->dist) * (base->combinedBitConsumption - cmp->combinedBitConsumption)))
			   best = cmp;
	   }

	   if(best)
		   plists->sorted[nvp++] = best;
	   else
		   break;
   }
   plists->sorted[nvp] = NULL;
}

/**
 * Loads possibility lists with actual data
 */
static int gather_possibility_data(possibility_list_t *plists,
								   cel_evaluation_t *celEvals, int numBlocks)
{
	while(numBlocks--) {
		gather_possibility_data_for_block(plists, celEvals);
		sort_possibility_data(plists);

		plists++;
		celEvals++;
	}

	return 1;
}

/**
 * Updates the validity and distortion/bit changes for moving up a notch
 * in a sort option list
 */
static void update_sort_option(sort_option_t *sortOption)
{
	roq_possibility_t *current;
	roq_possibility_t *next;

	current = sortOption->list->sorted[sortOption->plistIndex];
	next = sortOption->list->sorted[sortOption->plistIndex+1];

	if (!next) {
		sortOption->valid = 0;
		return;
	}

	sortOption->valid = 1;
	sortOption->bitCostDecrease = current->combinedBitConsumption -
								  next->combinedBitConsumption;
	sortOption->distIncrease = next->dist - current->dist;
}

// TODO: Bias towards options that don't use the codebook
int initial_plist_sort(const void *a, const void *b)
{
	const sort_option_t *so1;
	const sort_option_t *so2;

	int q1, q2;

	so1 = *((const sort_option_t **) a);
	so2 = *((const sort_option_t **) b);

	/* Sort primarily by validity */
	if (!so1->valid)
		return so2->valid;

	if (!so2->valid)
		return -1;

	/* Sort by seGain/bitLoss
	 * Cross-multiply so both have the same divisor */
	q1 = so1->distIncrease * so2->bitCostDecrease;
	q2 = so2->distIncrease * so1->bitCostDecrease;

	/* Lower SE/bits precedes */
	if (q1 != q2)
		return q1 - q2;
	/* Compare pointer addresses to force consistency */
	return (int) (so1 - so2);
}

static void modify_size_calc(roq_possibility_t *p, cel_evaluation_t *eval, roq_sizecalc_t *sizeCalc, int mod)
{
	unsigned long cb4Changes[4];
	unsigned long cb2Changes[16];
	int numCB4Changes=0;
	int numCB2Changes=0;
	int argumentsChange=0;
	int codeChange=1;
	int i;

	if (p->evalType == RoQ_ID_MOT) {
	} else if (p->evalType == RoQ_ID_FCC) {
		argumentsChange++;
	} else if (p->evalType == RoQ_ID_SLD) {
		argumentsChange++;
		cb4Changes[numCB4Changes++] = eval->cbEntry;
	} else {
		for (i=0; i<4; i++) {
			codeChange++;
			if (p->subEvalTypes[i] == RoQ_ID_MOT) {
			} else if (p->subEvalTypes[i] == RoQ_ID_FCC)
				argumentsChange++;\
			else if (p->subEvalTypes[i] == RoQ_ID_SLD) {
				argumentsChange++;
				cb4Changes[numCB4Changes++] = eval->subCels[i].cbEntry;
			} else if (p->subEvalTypes[i] == RoQ_ID_CCC) {
				argumentsChange+=4;
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[0];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[1];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[2];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[3];
			}
		}
	}

	/* Modify CB4 entries */
	for (i=0; i<numCB4Changes; i++) {
		if (mod == -1)
			sizeCalc->cb4UsageCount[cb4Changes[i]]--;

		if (!sizeCalc->cb4UsageCount[cb4Changes[i]]) {
			sizeCalc->usedCB4 += mod;
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][0];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][1];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][2];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][3];
		}

		if (mod == 1)
			sizeCalc->cb4UsageCount[cb4Changes[i]]++;
	}

	/* Modify CB2 entries */
	for (i=0; i<numCB2Changes; i++) {
		if (mod == -1)
			sizeCalc->cb2UsageCount[cb2Changes[i]]--;

		if (!sizeCalc->cb2UsageCount[cb2Changes[i]])
			sizeCalc->usedCB2 += mod;

		if (mod == 1)
			sizeCalc->cb2UsageCount[cb2Changes[i]]++;
	}

	sizeCalc->numArguments += argumentsChange * mod;
	sizeCalc->numCodes += codeChange * mod;
}

inline int calculate_size(roq_sizecalc_t *sizeCalc)
{
	int cbSize, mainBlockSize;
	int numCodeWordsSpooled;

	/** If all CB4 entries are used, all CB2 entries must be used too,
	 *  or it'll encode it as zero entries */
	if (sizeCalc->usedCB4 == 256)
		cbSize = 8 + 256*6 + 256*4;
	if (sizeCalc->usedCB2 || sizeCalc->usedCB4)
		cbSize = 8 + sizeCalc->usedCB2*6 + sizeCalc->usedCB4*4;
	else
		cbSize = 0;

	/* 1 code = 2 bits, round up to 2 byte divisible */
	numCodeWordsSpooled = ((sizeCalc->numCodes + 7) >> 3);

	mainBlockSize = 8 + numCodeWordsSpooled*2 + sizeCalc->numArguments;

	return cbSize + mainBlockSize;
}

/**
 * Returns the size of just the main block with no headers
 */
inline int calculate_size_main_block(roq_sizecalc_t *sizeCalc)
{
	int numCodeWordsSpooled;

	numCodeWordsSpooled = ((sizeCalc->numCodes + 7) >> 3);

	return numCodeWordsSpooled*2 + sizeCalc->numArguments;
}

/**
 * Repeatedly reduces by the option that will cause the least distortion increase
 * per bit cost increase.  This does the actual compression. :)
 */
int reduce_p_lists(sb4videoenc_t *enc, possibility_list_t *plists, cel_evaluation_t *evals,
				   roq_tempdata_t *tempData, int numBlocks, int sizeLowerLimit, int sizeLimit, __int64 *outDist)
{
	int i, idx;
	int belowDist;
	__int64 dist;

	roq_sizecalc_t sizeCalc;

	sort_option_t *sortOptions;
	sort_option_t **sortOptionsSorted;

	sort_option_t *swapTemp;

	/* Allocate memory */
	tempData->sortOptions.resize(numBlocks);
	sortOptions = &tempData->sortOptions[0];

	tempData->sortOptionsSorted.resize(numBlocks);
	sortOptionsSorted = &tempData->sortOptionsSorted[0];

	/* Set up codebook stuff */
	memset(&sizeCalc, 0, sizeof(sizeCalc));

	for (i=0; i<256; i++) {
		sizeCalc.cb4Indexes[i][0] = enc->cb4[i*4+0];
		sizeCalc.cb4Indexes[i][1] = enc->cb4[i*4+1];
		sizeCalc.cb4Indexes[i][2] = enc->cb4[i*4+2];
		sizeCalc.cb4Indexes[i][3] = enc->cb4[i*4+3];
	}

	/* Set up sort options */
	for (i=0; i<numBlocks; i++) {
		sortOptions[i].list = plists + i;
		sortOptions[i].plistIndex = 0;
		sortOptions[i].eval = evals + i;

		update_sort_option(sortOptions + i);

		sortOptionsSorted[i] = sortOptions + i;
	}

	/* Run initial sort */
	qsort(sortOptionsSorted, numBlocks, sizeof(sort_option_t *),
		  initial_plist_sort);

	/* Add all current options to the size calculation */
	dist = 0;
	for (i=0; i<numBlocks; i++)
	{
		dist += sortOptions[i].list->sorted[0]->dist;
		modify_size_calc(sortOptions[i].list->sorted[0], sortOptions[i].eval, &sizeCalc, 1);
	}

	belowDist = 1;
	while (calculate_size(&sizeCalc) > sizeLimit || belowDist || calculate_size_main_block(&sizeCalc) >= 65536) {
		if (!sortOptionsSorted[0]->valid)
			break;		// Can't be reduced any further

		/* Use the most feasible downshift */
		idx = sortOptionsSorted[0]->plistIndex;

		dist += sortOptionsSorted[0]->distIncrease;

		if(!enc->target_dist || calculate_size(&sizeCalc) < sizeLowerLimit)
			belowDist = 0;
		else
			belowDist = dist < enc->target_dist;

		/* Update the size calculator */
		modify_size_calc(sortOptionsSorted[0]->list->sorted[idx],
						 sortOptionsSorted[0]->eval, &sizeCalc, -1);
		modify_size_calc(sortOptionsSorted[0]->list->sorted[idx+1],
						 sortOptionsSorted[0]->eval, &sizeCalc, 1);

		/* Update the actual sort option */
		sortOptionsSorted[0]->plistIndex = idx+1;
		update_sort_option(sortOptionsSorted[0]);

		/* Bubble sort to where it belongs now */
		for (i=1; i<numBlocks; i++) {
			if (initial_plist_sort(&sortOptionsSorted[i-1], &sortOptionsSorted[i]) <= 0)
				break;

			swapTemp = sortOptionsSorted[i];
			sortOptionsSorted[i] = sortOptionsSorted[i-1];
			sortOptionsSorted[i-1] = swapTemp;
		}
	}

	*outDist = dist;

	/* Make remaps for the final codebook usage */
	idx = 0;
	for (i=0; i<256; i++) {
		if (sizeCalc.cb2UsageCount[i] || sizeCalc.usedCB4 == 256) {
			tempData->i2f2[i] = idx;
			tempData->f2i2[idx] = i;
			idx++;
		}
	}

	idx = 0;
	for (i=0; i<256; i++) {
		if (sizeCalc.cb4UsageCount[i]) {
			tempData->i2f4[i] = idx;
			tempData->f2i4[idx] = i;
			idx++;
		}
	}

	tempData->numCB4 = sizeCalc.usedCB4;
	tempData->numCB2 = sizeCalc.usedCB2;
	if(sizeCalc.usedCB4 == 256)
		tempData->numCB2 = 256;

	tempData->mainChunkSize = calculate_size_main_block(&sizeCalc);

	int calculatedSize = calculate_size(&sizeCalc) * 3;
	std::vector<unsigned char> test;
	test.resize(calculatedSize);
	test.resize(calculatedSize - 1);

	tempData->outbuffer.resize(calculatedSize);

	return 1;
}

/* NOTE: Typecodes must be spooled AFTER arguments!! */
#define SPOOL_ARGUMENT(arg)		\
do {\
	argumentSpool[argumentSpoolLength++] = (unsigned char)(arg);\
} while(0)

#define SPOOL_MOTION(dx, dy)	\
do {\
	unsigned char arg, ax, ay;\
	ax = 8 - (unsigned char)(dx);\
	ay = 8 - (unsigned char)(dy);\
	arg = (unsigned char)(((ax&15)<<4) | (ay&15));\
	SPOOL_ARGUMENT(arg);\
} while(0)



#define SPOOL_TYPECODE(type)		\
do {\
	typeSpool |= ((type) & 3) << (14 - typeSpoolLength);\
	typeSpoolLength += 2;\
	if (typeSpoolLength == 16) {\
		bytestream_put_le16(&vqData, typeSpool); \
		for (a=0; a<argumentSpoolLength; a++)\
			bytestream_put_byte(&vqData, argumentSpool[a]); \
		typeSpoolLength = 0;\
		typeSpool = 0;\
		argumentSpoolLength = 0;\
	}\
} while(0)

typedef struct
{
	int x,y;
} roq_subcel_offset_t;

int reconstruct_and_encode_image(sb4videoenc_t *enc, roq_tempdata_t *tempData, int w, int h, int numBlocks, void *handle, SB4Image *outImage, sb4writecallback_t cb)
{
	int i, j, k, a;
	int x, y;
	unsigned int subX, subY, fragX, fragY;
	SB4Block<8> block8;
	SB4Block<4> block4;
	SB4Block<2> block2;

	cel_evaluation_t *eval;
	roq_possibility_t *p;

	unsigned char *output;
	unsigned char *cbData;
	unsigned char *vqData;

	unsigned long typeSpool=0;
	int typeSpoolLength=0;
	unsigned char argumentSpool[64];
	int argumentSpoolLength=0;

	unsigned long chunkSize;

	int se = 0;

	roq_subcel_offset_t subcelOffsets[4] = {
		{0,0},
		{4,0},
		{0,4},
		{4,4},
	};

	roq_subcel_offset_t subsubcelOffsets[4] = {
		{0,0},
		{2,0},
		{0,2},
		{2,2},
	};

	output = &tempData->outbuffer[0];

	chunkSize = tempData->numCB2*6 + tempData->numCB4*4;

	/* Create codebook chunk */
	cbData = output;
	if (tempData->numCB2) {
		bytestream_put_le16(&cbData, RoQ_QUAD_CODEBOOK);

		bytestream_put_le32(&cbData, chunkSize);
		bytestream_put_byte(&cbData, tempData->numCB4);
		bytestream_put_byte(&cbData, tempData->numCB2);

		for (i=0; i<tempData->numCB2; i++) {
			for (j=0; j<6; j++)
				bytestream_put_byte(&cbData, enc->cb2[tempData->f2i2[i]*6 + j]);
		}

		for (i=0; i<tempData->numCB4; i++)
			for (j=0; j<4; j++)
				bytestream_put_byte(&cbData, tempData->i2f2[enc->cb4[tempData->f2i4[i]*4 + j]]);

	}

	/* Write the video chunk */
	chunkSize = tempData->mainChunkSize;

	vqData = cbData;

	if(enc->delta_encode)
		bytestream_put_le16(&vqData, RoQ_QUAD_DVQ);
	else
		bytestream_put_le16(&vqData, RoQ_QUAD_VQ);
	bytestream_put_le32(&vqData, chunkSize);
	bytestream_put_byte(&vqData, 0x0);
	bytestream_put_byte(&vqData, 0x0);

	for (i=0; i<numBlocks; i++) {
		eval = tempData->sortOptions[i].eval;
		p = tempData->sortOptions[i].list->sorted[tempData->sortOptions[i].plistIndex];
		se += p->dist;

		x = eval->sourceX;
		y = eval->sourceY;

		switch (p->evalType) {
		case RoQ_ID_MOT:
			SPOOL_TYPECODE(RoQ_ID_MOT);
			block8.LoadFromImage(enc->current_frame, x, y);
			block8.Blit(outImage, x, y);
			break;

		case RoQ_ID_FCC:
			SPOOL_MOTION(eval->motionX, eval->motionY);
			SPOOL_TYPECODE(RoQ_ID_FCC);

			block8.LoadFromImage(enc->last_frame, x + eval->motionX, y + eval->motionY);
			block8.Blit(outImage, x, y);
			break;

		case RoQ_ID_SLD:
			SPOOL_ARGUMENT(tempData->i2f4[eval->cbEntry]);
			SPOOL_TYPECODE(RoQ_ID_SLD);

			if(!enc->delta_encode)
				tempData->codebooks.unpacked_cb4_enlarged[eval->cbEntry].Blit(outImage, x, y);
			else
			{
				block8.LoadFromImage(enc->last_frame, x, y);
				tempData->codebooks.unpacked_cb4_enlarged[eval->cbEntry].DBlit(&block8, outImage, x, y);
			}
			break;

		case RoQ_ID_CCC:
			SPOOL_TYPECODE(RoQ_ID_CCC);
			for (j=0; j<4; j++) {
				subX = subcelOffsets[j].x + x;
				subY = subcelOffsets[j].y + y;

				switch(p->subEvalTypes[j]) {
				case RoQ_ID_MOT:
					block4.LoadFromImage(enc->current_frame, subX, subY);
					block4.Blit(outImage, subX, subY);
					SPOOL_TYPECODE(RoQ_ID_MOT);
					break;

				case RoQ_ID_FCC:
					SPOOL_MOTION(eval->subCels[j].motionX, eval->subCels[j].motionY);
					SPOOL_TYPECODE(RoQ_ID_FCC);

					block4.LoadFromImage(enc->last_frame, subX + eval->subCels[j].motionX,
										 subY + eval->subCels[j].motionY);
					block4.Blit(outImage, subX, subY);
					break;

				case RoQ_ID_SLD:
					SPOOL_ARGUMENT(tempData->i2f4[eval->subCels[j].cbEntry]);
					SPOOL_TYPECODE(RoQ_ID_SLD);

					if(!enc->delta_encode)
						tempData->codebooks.unpacked_cb4[eval->subCels[j].cbEntry].Blit(outImage, subX, subY);
					else
					{
						block4.LoadFromImage(enc->last_frame, subX, subY);
						tempData->codebooks.unpacked_cb4[eval->subCels[j].cbEntry].DBlit(&block4, outImage, subX, subY);
					}

					break;

				case RoQ_ID_CCC:
					for (k=0; k<4; k++) {
						SPOOL_ARGUMENT(tempData->i2f2[eval->subCels[j].subCels[k]]);

						fragX = subX + subsubcelOffsets[k].x;
						fragY = subY + subsubcelOffsets[k].y;

						if(!enc->delta_encode)
							tempData->codebooks.unpacked_cb2[eval->subCels[j].subCels[k]].Blit
								(outImage, fragX, fragY);
						else
						{
							block2.LoadFromImage(enc->last_frame, fragX, fragY);
							tempData->codebooks.unpacked_cb2[eval->subCels[j].subCels[k]].DBlit
								(&block2, outImage, fragX, fragY);
						}
					}
					SPOOL_TYPECODE(RoQ_ID_CCC);
					break;
				}
			}
			break;
		}
	}

	/* Flush the remainder of the argument/type spool */
	while (typeSpoolLength) {
		SPOOL_TYPECODE(0);
	}

	/* Write it all */
	cb(handle, output, vqData - output);
	enc->final_size = (int) (vqData - output);

	return 0;
}

int unpack_codebooks(sb4videoenc_t *enc, roq_tempdata_t *tempData)
{
	unsigned int i;

	UnpackAllCB2(enc->cb2, tempData->codebooks.unpacked_cb2);
	UnpackAllCB4(tempData->codebooks.unpacked_cb2, enc->cb4, tempData->codebooks.unpacked_cb4);

	for(i=0;i<256;i++)
		tempData->codebooks.unpacked_cb4_enlarged[i] = Enlarge4to8(tempData->codebooks.unpacked_cb4[i]);

	return 0;
}

void vectorExpandCallback(void *h, const void *bytes, size_t numBytes)
{
	std::vector<unsigned char> *vh;

	vh = (std::vector<unsigned char> *)h;

	vh->resize(numBytes);
	memcpy(&(*vh)[0], bytes, numBytes);
}

int vq_threshold_pass(sb4videoenc_t *enc, roq_tempdata_t *tempData, std::vector<unsigned char> *v, unsigned int threshold,
	SB4Image *outImage, __int64 *outDist)
{
	//AVRandomState randState = enc->rnd;

	if (enc->framesSinceKeyframe == 0)
		MakeCodebooks(enc->cb2, enc->cb4, &enc->rnd, NULL, enc->frame_to_enc, NULL, 0);
	else
		MakeCodebooks(enc->cb2, enc->cb4, &enc->rnd, enc->last_frame, enc->frame_to_enc, &tempData->subcelPredDists[0], threshold);

	if (unpack_codebooks(enc, tempData) < 0)
		return -1;

	if (gather_cel_data(enc, tempData, false) < 0)
		return -1;

	if (gather_possibility_data(&tempData->plists[0], &tempData->cel_evals[0],
								enc->width*enc->height/64) < 0)
		return -1;

	if (reduce_p_lists(enc, &tempData->plists[0], &tempData->cel_evals[0],
					   tempData,
					   enc->width*enc->height/64, enc->minBytes, enc->maxBytes, outDist) < 0)
		return -1;

	if (reconstruct_and_encode_image(enc, tempData, enc->width, enc->height,
									 enc->width*enc->height/64, v, outImage, vectorExpandCallback) < 0)
		return -1;
	return 0;
}

int vq_encode(sb4videoenc_t *enc, roq_tempdata_t *tempData, std::vector<unsigned char> *v)
{
	unsigned int highThreshold = enc->cb_omit_threshold;
	unsigned int lowThreshold = 0;

	if(enc->framesSinceKeyframe == 0)
	{
		if (gather_cel_data(enc, tempData, true) < 0)
			return -1;
		return vq_threshold_pass(enc, tempData, v, 0, enc->output_frameTEMP, &enc->distTEMP);
	}
	else
	{
		std::vector<unsigned char> highOutput;
		std::vector<unsigned char> lowOutput;

		SB4Image highImage;
		__int64 highDist;
		highImage.Init(enc->output_frameTEMP->Width(), enc->output_frameTEMP->Height());
		SB4Image lowImage;
		__int64 lowDist;
		lowImage.Init(enc->output_frameTEMP->Width(), enc->output_frameTEMP->Height());

		AVRandomState randState = enc->rnd;

		// Run motion pass
		if (gather_cel_data(enc, tempData, true) < 0)
			return -1;

		if(enc->framesSinceKeyframe > 0)
		{
			tempData->subcelPredDists.resize(enc->width * enc->height / 16);

			unsigned int celWidth = enc->width / 8;
			unsigned int celHeight = enc->height / 8;

			const cel_evaluation_t *celEval = &tempData->cel_evals[0];
			for(unsigned int cely=0;cely<celHeight;cely++)
			{
				for(unsigned int celx=0;celx<celWidth;celx++)
				{
					unsigned int *distLocs[4];
					distLocs[0] = &tempData->subcelPredDists[(cely * 2) * (celWidth * 2) + (celx * 2)];
					distLocs[1] = distLocs[0] + 1;
					distLocs[2] = distLocs[0] + (celWidth * 2);
					distLocs[3] = distLocs[2] + 1;

					for(int subEvalIdx=0;subEvalIdx<4;subEvalIdx++)
					{
						const roq_subcel_evaluation_t *subcelEval = celEval->subCels + subEvalIdx;
						unsigned int motDist = subcelEval->eval_dist[RoQ_ID_MOT];
						unsigned int fccDist = subcelEval->eval_dist[RoQ_ID_FCC];
						*distLocs[subEvalIdx] = (motDist < fccDist) ? motDist : fccDist;
					}
					celEval++;
				}
			}
		}

		// Generate initial high/low thresholds
		if(vq_threshold_pass(enc, tempData, &highOutput, enc->cb_omit_threshold, &highImage, &highDist) < 0)
			return -1;
		if(vq_threshold_pass(enc, tempData, &lowOutput, 0, &lowImage, &lowDist) < 0)
			return -1;

		while(true)
		{
			//printf("Refining thresholds: Current spread %i (%i) .. %i (%i)\n", lowThreshold, lowOutput.size(), highThreshold, highOutput.size());
			unsigned int midThreshold = (lowThreshold + highThreshold) / 2;
			if(midThreshold == lowThreshold)
			{
				if(lowDist < highDist)
				{
					*v = lowOutput;
					enc->distTEMP = lowDist;
					lowImage.CopyTo(enc->output_frameTEMP);
				}
				else
				{
					*v = highOutput;
					enc->distTEMP = highDist;
					highImage.CopyTo(enc->output_frameTEMP);
				}
				break;
			}
			
			SB4Image midImage;
			__int64 midDist;
			std::vector<unsigned char> midOutput;
			midImage.Init(enc->output_frameTEMP->Width(), enc->output_frameTEMP->Height());

			if(vq_threshold_pass(enc, tempData, &midOutput, midThreshold, &midImage, &midDist) < 0)
				return -1;

			if(lowDist < highDist)
			{
				highDist = midDist;
				highOutput = midOutput;
				midImage.CopyTo(&highImage);
				highThreshold = midThreshold;
			}
			else
			{
				lowDist = midDist;
				lowOutput = midOutput;
				midImage.CopyTo(&lowImage);
				lowThreshold = midThreshold;
			}
			midOutput.clear();
		}
	}

	return 0;
}

typedef struct
{
	std::vector<unsigned char> bytes;
	SB4Image image;
	__int64 dist;
	bool valid;
} sb4videoattempt_t;

enum
{
	SB4_ATTEMPT_VQ,
	SB4_ATTEMPT_DVQ,
	SB4_ATTEMPT_JPEG,
	SB4_ATTEMPT_DJPEG,

	SB4_ATTEMPT_COUNT,
};

int roq_encode_video(sb4videoenc_t *enc, void *h, sb4writecallback_t cb)
{
	roq_tempdata_t tempData;
	double lambda, targetLambda;
	unsigned int selected, i;
	sb4videoattempt_t attempts[SB4_ATTEMPT_COUNT];

	//attempts[SB4_ATTEMPT_DVQ].valid = attempts[SB4_ATTEMPT_DJPEG].valid = (enc->first_frame == 0);
	attempts[SB4_ATTEMPT_VQ].valid = true;
	//attempts[SB4_ATTEMPT_JPEG].valid = true;

	// No delta coding for now
	attempts[SB4_ATTEMPT_JPEG].valid = false;
	attempts[SB4_ATTEMPT_DVQ].valid = false;
	attempts[SB4_ATTEMPT_DJPEG].valid = false;

	if (create_cel_evals(enc, &tempData) < 0)
		goto error;

	if (create_possibility_lists(enc, &tempData) < 0)
		goto error;

	if (enc->framesSinceKeyframe > 0) {
		motion_search<8>(enc, true);
		motion_search<4>(enc, true);
	}

	// Try doing a regular encode
	if(enc->framesSinceKeyframe == 0)
		enc->cb_omit_threshold = 0;
	else
		enc->cb_omit_threshold = 1024;

	if(attempts[SB4_ATTEMPT_VQ].valid)
	{
		enc->delta_encode = 0;

		attempts[SB4_ATTEMPT_VQ].image.Init(enc->width, enc->height);

		enc->output_frameTEMP = &attempts[SB4_ATTEMPT_VQ].image;
		vq_encode(enc, &tempData, &attempts[SB4_ATTEMPT_VQ].bytes);
		attempts[SB4_ATTEMPT_VQ].dist = enc->distTEMP;
	}

	if(attempts[SB4_ATTEMPT_DVQ].valid)
	{
		enc->delta_encode = 1;

		attempts[SB4_ATTEMPT_DVQ].image.Init(enc->width, enc->height);

		enc->output_frameTEMP = &attempts[SB4_ATTEMPT_DVQ].image;
		vq_encode(enc, &tempData, &attempts[SB4_ATTEMPT_DVQ].bytes);
		attempts[SB4_ATTEMPT_DVQ].dist = enc->distTEMP;
	}

	/*
	if(attempts[SB4_ATTEMPT_JPEG].valid)
	{
		enc->delta_encode = 0;

		attempts[SB4_ATTEMPT_JPEG].image.Init(enc->width, enc->height);

		enc->output_frame = &attempts[SB4_ATTEMPT_JPEG].image;
		SB4ProgressiveReduceJPEG(enc, &attempts[SB4_ATTEMPT_JPEG].image, attempts[SB4_ATTEMPT_JPEG].bytes, false);
		attempts[SB4_ATTEMPT_JPEG].dist = enc->dist;
	}

	if(attempts[SB4_ATTEMPT_DJPEG].valid)
	{
		enc->delta_encode = 1;

		attempts[SB4_ATTEMPT_DJPEG].image.Init(enc->width, enc->height);

		enc->output_frame = &attempts[SB4_ATTEMPT_DJPEG].image;
		SB4ProgressiveReduceJPEG(enc, &attempts[SB4_ATTEMPT_DJPEG].image, attempts[SB4_ATTEMPT_DJPEG].bytes, true);
		attempts[SB4_ATTEMPT_DJPEG].dist = enc->dist;
	}
	*/

	selected = 0;
	targetLambda = (double)(attempts[0].dist) / (double)(attempts[0].bytes.size());
	printf("Lambda[0] = %f (%f / %i)\n", targetLambda, (double)attempts[0].dist, attempts[0].bytes.size());
	if(attempts[0].dist < 0)
		printf("Dist is negative!\n");
	for(i=1;i<SB4_ATTEMPT_COUNT;i++)
	{
		if(!attempts[i].valid)
			continue;
		// See which option has the lowest lambda
		lambda = (double)(attempts[i].dist) / (double)(attempts[i].bytes.size());
		printf("Lambda[%i] = %f (%f / %i)\n", i, lambda, (double)attempts[i].dist, attempts[i].bytes.size());
		if(attempts[i].dist < 0)
			printf("Dist is negative!\n");
		if(lambda < targetLambda)
		{
			targetLambda = lambda;
			selected = i;
		}
	}

	cb(h, &attempts[selected].bytes[0], attempts[selected].bytes.size());
	attempts[selected].image.CopyTo(enc->current_frame);
	printf("Encoded with technique %i\n", selected);

	/* Rotate frame history */
	Swap<SB4Image *>(enc->current_frame, enc->last_frame);
	Swap<motion_vect *>(enc->last_motion4, enc->this_motion4);
	Swap<motion_vect *>(enc->last_motion8, enc->this_motion8);

	free_temp_data(&tempData);

	enc->framesSinceKeyframe++;

	return 1;

 error:
	free_temp_data(&tempData);
	return -1;
}

int roq_encode_init(sb4videoenc_t *enc, int width, int height, int keyrate, int irate, int prate)
{
	motion_vect blank_vect;

	/* Clear out the context */
	memset(enc, 0, sizeof(*enc));
	new (enc) sb4videoenc_t();

	av_init_random(1, &enc->rnd);

	enc->framesToKeyframe = enc->keyrate = keyrate;
	enc->ibitrate = irate;
	enc->pbitrate = prate;

	enc->slip = 0;

	if ((width & 0xf) || (height & 0xf)) {
		MessageBox(NULL, "Video dimensions not divisible by 16", NULL, 0);
		return -1;
	}

	if (((width)&(width-1)) ||
		((height)&(height-1)))
	{
		MessageBox(NULL, "Video dimensions not a power of two, Q3/D3-based games will not play this!", "Warning", 0);
	}

	enc->width = width;
	enc->height = height;

	enc->framesSinceKeyframe = 0;
	enc->first_frame = 1;

	enc->last_frame	= &enc->frames[0];
	enc->current_frame = &enc->frames[1];

	blank_vect.dx = blank_vect.dy = 0;

	enc->motion4[0].resize(enc->width*enc->height/16, blank_vect);
	enc->motion4[1].resize(enc->width*enc->height/16, blank_vect);
	enc->motion8[0].resize(enc->width*enc->height/64, blank_vect);
	enc->motion8[1].resize(enc->width*enc->height/64, blank_vect);

	enc->this_motion4 = &enc->motion4[0][0];
	enc->last_motion4 = &enc->motion4[1][0];
	enc->this_motion8 = &enc->motion8[0][0];
	enc->last_motion8 = &enc->motion8[1][0];

	enc->frames[0].Init(width, height);
	enc->frames[1].Init(width, height);

	return 0;
}

static int roq_write_video_info_chunk(sb4videoenc_t *enc, void *h, sb4writecallback_t cb)
{
	unsigned char chunk[16];
	unsigned char *buf = chunk;

	/* ROQ info chunk */
	bytestream_put_le16(&buf, RoQ_INFO);

	/* Size: 8 bytes */
	bytestream_put_le32(&buf, 8);

	/* Unused argument */
	bytestream_put_byte(&buf, 0x00);
	bytestream_put_byte(&buf, 0x00);

	/* Width */
	bytestream_put_le16(&buf, enc->width);

	/* Height */
	bytestream_put_le16(&buf, enc->height);

	/* Unused in Quake 3, mimics the output of the real encoder */
	bytestream_put_byte(&buf, 0x08);
	bytestream_put_byte(&buf, 0x00);
	bytestream_put_byte(&buf, 0x04);
	bytestream_put_byte(&buf, 0x00);

	cb(h, chunk, buf - chunk);
	return 0;
}

int roq_encode_frame(sb4videoenc_t *enc, const SB4Image *img, void *h, sb4writecallback_t cb)
{
	int rate;
	int targetSize;
	__int64 actual_dist;

	enc->frame_to_enc = img;

	/* Check for I frame */
	if (!enc->framesToKeyframe) {
		enc->framesSinceKeyframe = 0;
		enc->framesToKeyframe = enc->keyrate;
		rate = enc->ibitrate;
	} else
		rate = enc->pbitrate;

	// No keyframes...
	//enc->framesToKeyframe--;

	targetSize = rate / 240;

	if (enc->first_frame)
	{
		enc->maxBytes = targetSize * 2 - 24;

		enc->target_dist = 0;

		/* Before the first video frame, write a "video info" chunk */
		roq_write_video_info_chunk(enc, h, cb);
	}
	else
	{
		enc->maxBytes = targetSize * 2 - (enc->slip / 4);
		enc->minBytes = targetSize / 2;

		if(enc->minBytes < 0)
			enc->minBytes = 0;
		if(enc->maxBytes < 0)
			enc->maxBytes = 0;
		//enc->maxBytes = targetSize;
		// enc->minBytes = 0;
		//enc->target_lambda = 0;
	}

	/* Encode the actual frame */
	if (roq_encode_video(enc, h, cb) < 0)
		return -1;

	// Update the byte slip
	enc->slip += enc->final_size - targetSize;

	// If too many easy-to-encode images show up, frames will drop below the
	// minimum, causing slip to decrease rapidly.
	// Cap slip at 4 frames worth of data.
	if(enc->slip < -targetSize*4)
		enc->slip = -targetSize*4;

	actual_dist = enc->distTEMP;

	printf("Dist: %llu\nTarget: %llu\n", actual_dist, enc->target_dist);

	if(enc->first_frame)
		enc->target_dist = actual_dist;
	else
		enc->target_dist = (enc->target_dist * 6 / 10) + (actual_dist * 4 / 10);

	// If we're over by more than 3 frames, nudge target_dist
	if(enc->slip > targetSize * 3)
		enc->target_dist += enc->target_dist / 20;
	else if(enc->slip > targetSize)
		enc->target_dist += enc->target_dist / 40;
	else if(enc->slip < -targetSize)
		enc->target_dist -= enc->target_dist / 20;

	printf("New target: %llu\n", enc->target_dist);

	enc->first_frame = 0;

	return 0;
}


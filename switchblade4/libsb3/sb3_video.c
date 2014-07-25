// COSTS:
// Level 1:
//  SKIP - 2 bits
//  MOTION - 10 bits
//  CODEBOOK - 10 bits
//  SUBDIVIDE - 2 + combined subcel cost
//
// Level 2:
//  SKIP - 2 bits
//  MOTION - 10 bits
//  CODEBOOK - 10 bits
//  SUBDIVIDE - 34 bits
//
// Maximum cost: 138 bits per cel
//
// Proper evaluation requires LCD fraction comparison, which requires
// MSE loss * savings increase
//
// Maximum savings increase: 136 bits
// Maximum MSE loss without overflow: 31580641
// Components in 8x8 supercel: 192
// Maximum MSE precision per component: 164482
//    >65025, so no truncation is needed (phew)

#include <stdlib.h>
#include <string.h>

#include "sb3_internal.h"
#include "sb3_yuv.h"
#include "sb3_vq.h"


typedef struct
{
	u32 eval_mse[4];

	u8 subCels[4];

	i8 motionX, motionY;
	u8 cbEntry;
} sb3_subcel_evaluation_t;

typedef struct
{
	u32 eval_mse[3];

	sb3_subcel_evaluation_t subCels[4];

	i8 motionX, motionY;
	u8 cbEntry;

	uint sourceX, sourceY;
} sb3_cel_evaluation_t;


// This must match the ROQ format order
#define SB3_EVALTYPE_SKIP		0
#define SB3_EVALTYPE_MOTION		1
#define SB3_EVALTYPE_CODEBOOK	2
#define SB3_EVALTYPE_SUBDIVIDE	3


// SUB (SUB SUB SUB SUB)
// 5 codes (10 bits) + 16 arguments (128) = 138 bits
#define SB3_MAX_BITS_PER_PLIST	138

// 4*4*4*4 subdivide types + 3 non-subdivide main types
#define SB3_MAX_POSSIBILITIES	259



typedef struct
{
	int evalType;
	int subEvalTypes[4];

	u32 byteConsumption;	// Number of bytes used for data encodes consumed
	u32 codeConsumption;	// Number of 2-bit codes consumed
	u32 combinedBitConsumption;

	u32 mse;

	int allowed;
} sb3_possibility_t;

typedef struct
{
	sb3_possibility_t p[SB3_MAX_POSSIBILITIES];
	sb3_possibility_t *sorted[SB3_MAX_POSSIBILITIES];
} sb3_possibility_list_t;

typedef struct
{
	sb3_rgbapixel_t image[16];
	u8 indexes[4];
} sb3_cb4_entry_t;

typedef struct
{
	sb3_rgbapixel_t image[4];
	u8 yuv[6];
} sb3_cb2_entry_t;

typedef struct
{
	sb3_cb4_entry_t cb4[256];
	uint numCB4;
	sb3_cb2_entry_t cb2[256];
	uint numCB2;
} sb3_codebooks_t;


typedef struct
{
	u32 mseIncrease;
	u32 bitCostDecrease;

	int valid;

	sb3_possibility_list_t *list;
	uint plistIndex;

	sb3_cel_evaluation_t *eval;
} sb3_sort_option_t;


typedef struct
{
	u32 cb2UsageCount[256];
	u32 cb4UsageCount[256];
	u8 cb4Indexes[256][4];

	u32 usedCB2;
	u32 usedCB4;

	u32 numCodes;
	u32 numArguments;
} sb3_sizecalc_t;


// =================================================
// Blit: Fast blitter
rc_inline void Blit(const sb3_rgbapixel_t *in, uint inwidth, sb3_rgbapixel_t *out, uint outwidth, uint copywidth, uint copyheight)
{
	while(copyheight)
	{
		memcpy(out, in, sizeof(sb3_rgbapixel_t)*copywidth);
		in += inwidth;
		out += outwidth;
		copyheight--;
	}
}


// DiffRGB: Returns MSE between two RGB blocks
rc_inline u32 DiffRGB(const sb3_rgbapixel_t *a, uint awidth, const sb3_rgbapixel_t *b, uint bwidth, uint checkwidth, uint checkheight)
{
	u32 diff=0;
	u32 subDiffs[3];
	u32 x;

	while(checkheight)
	{
		x = checkwidth;
		while(x)
		{
			subDiffs[0] = (a->r - b->r);
			subDiffs[1] = (a->g - b->g);
			subDiffs[2] = (a->b - b->b);
			diff += subDiffs[0]*subDiffs[0] + subDiffs[1]*subDiffs[1] + subDiffs[2]*subDiffs[2];

			a++;
			b++;
			x--;
		}

		a += awidth - checkwidth;
		b += bwidth - checkwidth;
		checkheight--;
	}

	return diff;
}

// DiffRGBLimited: Returns MSE between two RGB blocks, but cuts it off if it goes over the limit
rc_inline u32 DiffRGBLimited(const sb3_rgbapixel_t *a, uint awidth, const sb3_rgbapixel_t *b, uint bwidth, uint checkwidth, uint checkheight, uint limit)
{
	u32 diff=0;
	u32 subDiffs[3];
	u32 x;

	while(checkheight)
	{
		x = checkwidth;
		while(x)
		{
			subDiffs[0] = (a->r - b->r);
			subDiffs[1] = (a->g - b->g);
			subDiffs[2] = (a->b - b->b);
			diff += subDiffs[0]*subDiffs[0] + subDiffs[1]*subDiffs[1] + subDiffs[2]*subDiffs[2];

			a++;
			b++;
			x--;
		}

		if(diff > limit) return diff;

		a += awidth - checkwidth;
		b += bwidth - checkwidth;
		checkheight--;
	}

	return diff;
}

#define ENLARGE_ELEMENT(x,y)	\
	src = &image4[(y*4)+x];\
	memcpy(&image8[(y*16)+(x*2)], src, sizeof(sb3_rgbapixel_t));\
	memcpy(&image8[(y*16)+(x*2)+1], src, sizeof(sb3_rgbapixel_t));\
	memcpy(&image8[(y*16)+(x*2)+8], src, sizeof(sb3_rgbapixel_t));\
	memcpy(&image8[(y*16)+(x*2)+9], src, sizeof(sb3_rgbapixel_t))
	


rc_inline void Enlarge(sb3_rgbapixel_t *image4, sb3_rgbapixel_t *image8)
{
	sb3_rgbapixel_t *src;

	ENLARGE_ELEMENT(0,0);
	ENLARGE_ELEMENT(1,0);
	ENLARGE_ELEMENT(2,0);
	ENLARGE_ELEMENT(3,0);

	ENLARGE_ELEMENT(0,1);
	ENLARGE_ELEMENT(1,1);
	ENLARGE_ELEMENT(2,1);
	ENLARGE_ELEMENT(3,1);

	ENLARGE_ELEMENT(0,2);
	ENLARGE_ELEMENT(1,2);
	ENLARGE_ELEMENT(2,2);
	ENLARGE_ELEMENT(3,2);

	ENLARGE_ELEMENT(0,3);
	ENLARGE_ELEMENT(1,3);
	ENLARGE_ELEMENT(2,3);
	ENLARGE_ELEMENT(3,3);
}




// =================================================
// Temporary vars
typedef struct
{
	sb3_cel_evaluation_t *cel_evals;
	sb3_possibility_list_t *plists;
	sb3_yuvcluster4_t *yuvClusters;
	sb3_sort_option_t *sortOptions;
	sb3_sort_option_t **sortOptionsSorted;

	sb3_rgbapixel_t *reconstruct;

	u8 *outbuffer;

	u32 f2i4[256];
	u32 i2f4[256];
	u32 f2i2[256];
	u32 i2f2[256];

	u32 numCB4;
	u32 numCB2;

	u32 mainChunkSize;

	sb3_codebooks_t codebooks;
} sb3_tempdata_t;

static void FreeTempData(sb3_tempdata_t *tempData)
{
	if(tempData->cel_evals) free(tempData->cel_evals);
	if(tempData->plists) free(tempData->plists);
	if(tempData->yuvClusters) free(tempData->yuvClusters);
	if(tempData->sortOptions) free(tempData->sortOptions);
	if(tempData->sortOptionsSorted) free(tempData->sortOptionsSorted);
	if(tempData->reconstruct) free(tempData->reconstruct);
	if(tempData->outbuffer) free(tempData->outbuffer);
}


// ================================================

// CreateCelEvals: Initializes cel evaluators and sets their source coordinates
static int CreateCelEvals(sb3_encoder_t *enc, sb3_tempdata_t *tempData)
{
	uint width, height;
	uint n,x,y;

	width = enc->config.width;
	height= enc->config.height;

	tempData->cel_evals = malloc(width*height/64 * sizeof(sb3_cel_evaluation_t));
	if(!tempData->cel_evals) return 0;

	// Map to the ROQ quadtree order
	n = 0;
	for(y=0;y<height;y+=16)
	{
		for(x=0;x<width;x+=16)
		{
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


// CreatePossibilities: Initializes one possibility list
static int InitializeSinglePossibilityList(sb3_possibility_list_t *plist, uint fsk)
{
	uint i,j,k,l,m;
	uint n;
	uint firstAllowed;

	plist->p[0].evalType = SB3_EVALTYPE_MOTION;
	plist->p[0].allowed = (fsk >= 1);

	plist->p[1].evalType = SB3_EVALTYPE_SKIP;
	plist->p[1].allowed = (fsk >= 2);

	plist->p[2].evalType = SB3_EVALTYPE_CODEBOOK;
	plist->p[2].allowed = 1;

	n = 3;

	if(fsk >= 2) firstAllowed = 0;
	else if(fsk >= 1) firstAllowed = 1;
	else firstAllowed = 2;

	for(i=firstAllowed;i<4;i++)
	for(j=firstAllowed;j<4;j++)
	for(k=firstAllowed;k<4;k++)
	for(l=firstAllowed;l<4;l++)
	{
		plist->p[n].evalType = SB3_EVALTYPE_SUBDIVIDE;
		plist->p[n].allowed = 1;
		plist->p[n].subEvalTypes[0] = i;
		plist->p[n].subEvalTypes[1] = j;
		plist->p[n].subEvalTypes[2] = k;
		plist->p[n].subEvalTypes[3] = l;
		n++;
	}

	while(n < SB3_MAX_POSSIBILITIES)
	{
		plist->p[n].allowed = 0;
		n++;
	}

	return 1;
}


// CreatePossibilityLists: Initializes all possibility lists
static int CreatePossibilityLists(sb3_encoder_t *enc, sb3_tempdata_t *tempData)
{
	sb3_possibility_list_t *plists;
	uint n, max, fsk;

	max = enc->config.width*enc->config.height/64;

	tempData->plists = plists = malloc(sizeof(sb3_possibility_list_t) * max);
	if(!plists) return 0;

	fsk = enc->framesSinceKeyframe;
	while(max)
	{
		InitializeSinglePossibilityList(plists, fsk);
		plists++;
		max--;
	}

	return 1;
}


// CreateYUVClusters: Creates YUV clusters for the entire image
static void CreateYUVClusters(const sb3_rgbapixel_t *image, uint w, uint h, sb3_yuvcluster4_t *yuvClusters)
{
	// Although it's a 4x4 block, it's treated like an 8x2 block during this
	sb3_rgbapixel_t blocks2[16];
	uint x, y, i, i2;

	u8 y_components[4];
	u8 u_components[4];
	u8 v_components[4];

	void *dump;
	sb3_rgbapixel_t dumpPixels[4];

	uint c_average;

	for(y=0;y<h;y+=4)
	{
		for(x=0;x<w;x+=4)
		{
			// Copy RGB data
			Blit(image + (y*w)+x, w, &blocks2[0], 8, 2, 2);
			Blit(image + (y*w)+x+2, w, &blocks2[2], 8, 2, 2);
			Blit(image + ((y+2)*w)+x, w, &blocks2[4], 8, 2, 2);
			Blit(image + ((y+2)*w)+x+2, w, &blocks2[6], 8, 2, 2);

			// Convert to YUV
			i2 = 0;
			for(i=0;i<4;i++)
			{
				rgb2yuv(blocks2[i2].r, blocks2[i2].g, blocks2[i2].b, y_components+0, u_components+0, v_components+0);
				rgb2yuv(blocks2[i2+1].r, blocks2[i2+1].g, blocks2[i2+1].b, y_components+1, u_components+1, v_components+1);
				rgb2yuv(blocks2[i2+8].r, blocks2[i2+8].g, blocks2[i2+8].b, y_components+2, u_components+2, v_components+2);
				rgb2yuv(blocks2[i2+9].r, blocks2[i2+9].g, blocks2[i2+9].b, y_components+3, u_components+3, v_components+3);

				yuvClusters->block[i].y[0] = y_components[0];
				yuvClusters->block[i].y[1] = y_components[1];
				yuvClusters->block[i].y[2] = y_components[2];
				yuvClusters->block[i].y[3] = y_components[3];
				c_average = (u_components[0] + u_components[1] + u_components[2] + u_components[3] + 2) / 4;
				yuvClusters->block[i].u = (u8)c_average;
				c_average = (v_components[0] + v_components[1] + v_components[2] + v_components[3] + 2) / 4;
				yuvClusters->block[i].v = (u8)c_average;

				i2+=2;
			}

			yuvClusters++;
		}
	}
}


// ConvertCB2ToRGB: Converts a 2x2 codebook list into cached RGB mini-images
void ConvertCB2ToRGB(sb3_cb2_entry_t *cb2, uint numCB2)
{
	u8 y[4];
	u8 u,v;
	uint i;

	while(numCB2)
	{
		y[0] = cb2->yuv[0];
		y[1] = cb2->yuv[1];
		y[2] = cb2->yuv[2];
		y[3] = cb2->yuv[3];
		u = cb2->yuv[4];
		v = cb2->yuv[5];

		for(i=0;i<4;i++)
			yuv2rgb(y[i], u, v, &cb2->image[i].r, &cb2->image[i].g, &cb2->image[i].b);

		cb2++;
		numCB2--;
	}
}


// ConvertCB4ToRGB: Converts a 4x4 codebook list into cached RGB mini-images
static void ConvertCB4ToRGB(sb3_cb4_entry_t *cb4, sb3_cb2_entry_t *cb2, uint numCB4)
{
	uint i;
	u8 index;

	while(numCB4)
	{
		Blit(cb2[cb4->indexes[0]].image, 2, cb4->image + 0, 4, 2, 2);
		Blit(cb2[cb4->indexes[1]].image, 2, cb4->image + 2, 4, 2, 2);
		Blit(cb2[cb4->indexes[2]].image, 2, cb4->image + 8, 4, 2, 2);
		Blit(cb2[cb4->indexes[3]].image, 2, cb4->image + 10, 4, 2, 2);

		cb4++;
		numCB4--;
	}
}


// IndexCB4SubCel: Indexes a 2x2 subcel from a 4x4 entry to the closest 2x2 entry
static u8 IndexCB4SubCel(sb3_yuvcluster2_t *cluster2, sb3_cb2_entry_t *cb2, uint numCB2)
{
	u32 diff;
	u32 lowestDiff;
	u8 lowest;
	uint i;
	sb3_rgbapixel_t miniImage[4];

	// Convert to RGB
	for(i=0;i<4;i++)
		yuv2rgb(cluster2->y[i], cluster2->u, cluster2->v, &miniImage[i].r, &miniImage[i].g, &miniImage[i].b);

	// Match to the lowest one
	lowestDiff = 0xFFFFFFFF;
	lowest = 0;

	for(i=0;i<256;i++)
	{
		diff = DiffRGBLimited(cb2[i].image, 2, miniImage, 2, 2, 2, lowestDiff);
		if(diff < lowestDiff)
		{
			lowestDiff = diff;
			lowest = (u8)i;
		}
	}

	return lowest;
}


// GetCodebooks: Generates new codebooks
static int GenerateNewCodebooks(sb3_encoder_t *enc, sb3_tempdata_t *tempData, const sb3_rgbapixel_t *image, u8 *cacheOut)
{
	uint w, h;
	u32 numCB2, numCB4, max;
	sb3_yuvcluster4_t *results4;
	sb3_yuvcluster2_t *results2;
	sb3_codebooks_t *codebooks;
	uint i,n;

	w = enc->config.width;
	h = enc->config.height;
	max = w*h/16;

	tempData->yuvClusters = malloc(sizeof(sb3_yuvcluster4_t)*max);
	if(!tempData->yuvClusters) return 0;

	codebooks = &tempData->codebooks;

	memset(cacheOut, 0, SB3_CODEBOOK_CACHE_SIZE);

	// Create YUV data
	CreateYUVClusters(image, w, h, tempData->yuvClusters);

	// Create 2x2 codebooks
	if(enc->config.useNeuQuant)
	{
		if(!YUVGenerateCodebooks2_NQ(enc, tempData->yuvClusters->block, max * 4, 256, &numCB2, &results2)) return 0;
	}
	else
	{
		if(!YUVGenerateCodebooks2(enc, tempData->yuvClusters->block, max * 4, 256, &numCB2, &results2)) return 0;
	}

	codebooks->numCB2 = numCB2;

	// Copy out YUV data
	n = 0;
	for(i=0;i<numCB2;i++)
	{
		cacheOut[n++] = codebooks->cb2[i].yuv[0] = results2[i].y[0];
		cacheOut[n++] = codebooks->cb2[i].yuv[1] = results2[i].y[1];
		cacheOut[n++] = codebooks->cb2[i].yuv[2] = results2[i].y[2];
		cacheOut[n++] = codebooks->cb2[i].yuv[3] = results2[i].y[3];
		cacheOut[n++] = codebooks->cb2[i].yuv[4] = results2[i].u;
		cacheOut[n++] = codebooks->cb2[i].yuv[5] = results2[i].v;		
	}

	// Free 2x2 results
	free(results2);

	// Convert CB2 entries into regular RGB entries
	ConvertCB2ToRGB(tempData->codebooks.cb2, numCB2);

	// Create 4x4 codebooks
	if(enc->config.useNeuQuant)
	{
		if(!YUVGenerateCodebooks4_NQ(enc, tempData->yuvClusters, max, 256, &numCB4, &results4)) return 0;
	}
	else
	{
		if(!YUVGenerateCodebooks4(enc, tempData->yuvClusters, max, 256, &numCB4, &results4)) return 0;
	}

	codebooks->numCB4 = numCB4;

	n = 256*6;

	// Index all subimages
	for(i=0;i<numCB4;i++)
	{
		cacheOut[n++] = codebooks->cb4[i].indexes[0] = IndexCB4SubCel(&results4[i].block[0], codebooks->cb2, numCB2);
		cacheOut[n++] = codebooks->cb4[i].indexes[1] = IndexCB4SubCel(&results4[i].block[1], codebooks->cb2, numCB2);
		cacheOut[n++] = codebooks->cb4[i].indexes[2] = IndexCB4SubCel(&results4[i].block[2], codebooks->cb2, numCB2);
		cacheOut[n++] = codebooks->cb4[i].indexes[3] = IndexCB4SubCel(&results4[i].block[3], codebooks->cb2, numCB2);
	}

	// Free 4x4 results
	free(results4);

	// Convert CB4 entries into regular RGB entries
	ConvertCB4ToRGB(tempData->codebooks.cb4, tempData->codebooks.cb2, numCB4);

	return 1;
}



// GetCodebooks: Either generates a new set of codebooks, or loads cached ones, and converts
// them to RGB data
static int GetCodebooks(sb3_encoder_t *enc, sb3_tempdata_t *tempData, const sb3_rgbapixel_t *image, const sb3_hostapi_t *hapi, void *handle)
{
	u8 cbCache[SB3_CODEBOOK_CACHE_SIZE];
	uint i,j,n;
	int loadedFromCache = 0;

	sb3_rgbapixel_t debug[64*64];
	u32 x,y;

	if(hapi->readCodebookCache && hapi->readCodebookCache(handle, cbCache))
	{
		loadedFromCache = 1;

		// Parse cache
		tempData->codebooks.numCB2 = tempData->codebooks.numCB4 = 256;

		n = 0;
		for(i=0;i<256;i++)
			for(j=0;j<6;j++)
				tempData->codebooks.cb2[i].yuv[j] = cbCache[n++];
		for(i=0;i<256;i++)
			for(j=0;j<4;j++)
				tempData->codebooks.cb4[i].indexes[j] = cbCache[n++];

		// Convert to RGB
		ConvertCB2ToRGB(tempData->codebooks.cb2, tempData->codebooks.numCB2);
		ConvertCB4ToRGB(tempData->codebooks.cb4, tempData->codebooks.cb2, tempData->codebooks.numCB4);
	}
	else
	{
		// Generate new codebooks
		if(!GenerateNewCodebooks(enc, tempData, image, cbCache)) return 0;

		// Save the codebook cache
		if(hapi->writeCodebookCache)
			hapi->writeCodebookCache(handle, cbCache);
	}

	return 1;
}


typedef struct
{
	int dx, dy;
} sb3_motionsearch_vector_t;


// MotionSearch: Performs motion searching on an image at an offset, sets outDX and outDY to motion offset
static u32 MotionSearch(sb3_encoder_t *enc, const sb3_rgbapixel_t *image, u32 x, u32 y, i8 *outDX, i8 *outDY, uint d)
{
	sb3_motionsearch_vector_t offsets[9] = {
		{0,0},
		{0,-1},
		{-1,-1},
		{-1,0},
		{-1,1},
		{0,1},
		{1,1},
		{1,0},
		{1,-1},
	};

	u32 diffs[9];
	u32 diffPick, lowestDiff;

	u32 w,h,i;

	u32 rx,ry;

	i32 finalDX, finalDY;

	i32 step;

	w = enc->config.width;
	h = enc->config.height;

	finalDX = 0;
	finalDY = 0;

	// Simple three-step search

	// The first cel is centered and recycled, so it has to be calculated for the first one now
	diffs[0] = DiffRGB(image + (y*w)+x, w, enc->frameHistory1 + (y*w)+x, w, d, d);

	step = 8;
	while(step != 1)
	{
		step >>= 1;

		// Try in nearby offsets
		for(i=1;i<9;i++)
		{
			rx = x + finalDX + (step*offsets[i].dx);
			ry = y + finalDY + (step*offsets[i].dy);

			// Unsigned, so it'll be above the limit if it loops under
			// 3.11 - Fixed, was ry >= y
			if(rx >= w || ry >= h || (rx+d) > w || (ry+d) > h)
				diffs[i] = 0xFFFFFFFF;
			else
				diffs[i] = DiffRGBLimited(image + (y*w)+x, w, enc->frameHistory1 + (ry*w)+rx, w, d, d, diffs[0]);
		}

		// Pick the lowest diff
		lowestDiff = diffs[0];
		diffPick = 0;
		for(i=1;i<9;i++)
		{
			if(diffs[i] < lowestDiff)
			{
				lowestDiff = diffs[i];
				diffPick = i;
			}
		}

		// Update the result DX/DY
		finalDX += step*offsets[diffPick].dx;
		finalDY += step*offsets[diffPick].dy;

		// Recycle the diff
		diffs[0] = diffs[diffPick];
	}

	*outDX = (i8)finalDX;
	*outDY = (i8)finalDY;


	return diffs[0];
}


// EnlargedCBEntryMSE: Returns MSE of a section to its nearest enlarged 4x4 codebook entry
static u32 EnlargedCBEntryMSE(const sb3_rgbapixel_t *image, u32 width, sb3_cb4_entry_t *cb4, uint numCB4, u8 *outIndex)
{
	sb3_rgbapixel_t enlarged[64];
	u32 diff;
	u32 pick, lowestDiff;
	u32 i;

	// Create initial diffs
	Enlarge(cb4[0].image, enlarged);
	lowestDiff = DiffRGB(image, width, enlarged, 8, 8, 8);
	pick = 0;

	// Diff against the others
	for(i=1;i<numCB4;i++)
	{
		Enlarge(cb4[i].image, enlarged);

		diff = DiffRGBLimited(image, width, enlarged, 8, 8, 8, lowestDiff);
		if(diff < lowestDiff)
		{
			lowestDiff = diff;
			pick = i;
		}
	}

	*outIndex = (u8)pick;
	return lowestDiff;
}

static u32 CB4EntryMSE(const sb3_rgbapixel_t *image, u32 width, sb3_cb4_entry_t *cb, uint numCB, u8 *outIndex)
{
	u32 diff;
	u32 pick, lowestDiff;
	u32 i;

	// Create initial diffs
	lowestDiff = DiffRGB(image, width, cb[0].image, 4, 4, 4);
	pick = 0;

	for(i=1;i<numCB;i++)
	{
		diff = DiffRGBLimited(image, width, cb[i].image, 4, 4, 4, lowestDiff);
		if(diff < lowestDiff)
		{
			lowestDiff = diff;
			pick = i;
		}
	}

	*outIndex = (u8)pick;
	return lowestDiff;
}


static u32 CB2EntryMSE(const sb3_rgbapixel_t *image, u32 width, sb3_cb2_entry_t *cb, uint numCB, u8 *outIndex)
{
	u32 diff;
	u32 pick, lowestDiff;
	u32 i;

	// Create initial diffs
	lowestDiff = DiffRGB(image, width, cb[0].image, 2, 2, 2);
	pick = 0;

	for(i=1;i<numCB;i++)
	{
		diff = DiffRGBLimited(image, width, cb[i].image, 2, 2, 2, lowestDiff);
		if(diff < lowestDiff)
		{
			lowestDiff = diff;
			pick = i;
		}
	}

	*outIndex = (u8)pick;
	return lowestDiff;
}




// GatherDataForCel: Gets MSE for all options available to a subcel
static int GatherDataForSubCel(sb3_subcel_evaluation_t *subcel, u32 x, u32 y, sb3_encoder_t *enc, const sb3_rgbapixel_t *image, sb3_tempdata_t *tempData)
{
	u32 poffset, w;

	w = enc->config.width;
	poffset = (y*w) + x;

	if(enc->framesSinceKeyframe >= 1)
		subcel->eval_mse[SB3_EVALTYPE_MOTION] = MotionSearch(enc, image, x, y, &subcel->motionX, &subcel->motionY, 4);
	// 3.11 - Forgot to add poffset on skip!!
	if(enc->framesSinceKeyframe >= 2)
		subcel->eval_mse[SB3_EVALTYPE_SKIP] = DiffRGB(image + poffset, w, enc->frameHistory2 + poffset, w, 4, 4);

	subcel->eval_mse[SB3_EVALTYPE_CODEBOOK] = CB4EntryMSE(image + poffset, w, tempData->codebooks.cb4, tempData->codebooks.numCB4, &subcel->cbEntry);

	subcel->eval_mse[SB3_EVALTYPE_SUBDIVIDE] = CB2EntryMSE(image + poffset, w, tempData->codebooks.cb2, tempData->codebooks.numCB2, &subcel->subCels[0]);
	subcel->eval_mse[SB3_EVALTYPE_SUBDIVIDE] += CB2EntryMSE(image + poffset + 2, w, tempData->codebooks.cb2, tempData->codebooks.numCB2, &subcel->subCels[1]);
	subcel->eval_mse[SB3_EVALTYPE_SUBDIVIDE] += CB2EntryMSE(image + poffset + (w*2), w, tempData->codebooks.cb2, tempData->codebooks.numCB2, &subcel->subCels[2]);
	subcel->eval_mse[SB3_EVALTYPE_SUBDIVIDE] += CB2EntryMSE(image + poffset + (w*2) + 2, w, tempData->codebooks.cb2, tempData->codebooks.numCB2, &subcel->subCels[3]);

	return 1;
}



// GatherDataForCel: Gets MSE for all options available to a cel
static int GatherDataForCel(sb3_cel_evaluation_t *cel, sb3_encoder_t *enc, const sb3_rgbapixel_t *image, sb3_tempdata_t *tempData)
{
	u32 poffset, w;

	w = enc->config.width;
	poffset = (cel->sourceY*w) + cel->sourceX;

	if(enc->framesSinceKeyframe >= 1)
		cel->eval_mse[SB3_EVALTYPE_MOTION] = MotionSearch(enc, image, cel->sourceX, cel->sourceY, &cel->motionX, &cel->motionY, 8);
	// 3.11 - Forgot to add poffset on skips!!
	if(enc->framesSinceKeyframe >= 2)
		cel->eval_mse[SB3_EVALTYPE_SKIP] = DiffRGB(image + poffset, w, enc->frameHistory2 + poffset, w, 8, 8);
	cel->eval_mse[SB3_EVALTYPE_CODEBOOK] = EnlargedCBEntryMSE(image + poffset, w, tempData->codebooks.cb4, tempData->codebooks.numCB4, &cel->cbEntry);

	if(!GatherDataForSubCel(cel->subCels + 0, cel->sourceX+0, cel->sourceY+0, enc, image, tempData)) return 0;
	if(!GatherDataForSubCel(cel->subCels + 1, cel->sourceX+4, cel->sourceY+0, enc, image, tempData)) return 0;
	if(!GatherDataForSubCel(cel->subCels + 2, cel->sourceX+0, cel->sourceY+4, enc, image, tempData)) return 0;
	if(!GatherDataForSubCel(cel->subCels + 3, cel->sourceX+4, cel->sourceY+4, enc, image, tempData)) return 0;

	return 1;
}




// GatherCelData: Gathers MSE data for all feasible possibilities
static int GatherCelData(sb3_encoder_t *enc, const sb3_rgbapixel_t *image, sb3_tempdata_t *tempData)
{
	uint i, w, h, max;
	sb3_cel_evaluation_t *cel_evals;

	w = enc->config.width;
	h = enc->config.height;
	max = w*h/64;
	cel_evals = tempData->cel_evals;

	while(max)
	{
		if(!GatherDataForCel(cel_evals, enc, image, tempData)) return 0;
		cel_evals++;
		max--;
	}

	return 1;
}


// GatherPossibilityDataForBlock: Loads possibility lists with actual data for one block,
// assigning all possibilities a cached MSE and bit consumption
static int GatherPossibilityDataForBlock(sb3_possibility_list_t *plist, sb3_cel_evaluation_t *celEval)
{
	u32 i,j;
	sb3_possibility_t *p;

	for(i=0;i<SB3_MAX_POSSIBILITIES;i++)
	{
		p = plist->p+i;
		if(!p->allowed) continue;

		if(p->evalType == SB3_EVALTYPE_SKIP)
		{
			p->codeConsumption = 1;
			p->byteConsumption = 0;
			p->mse = celEval->eval_mse[SB3_EVALTYPE_SKIP];
		}
		else if(p->evalType == SB3_EVALTYPE_MOTION || p->evalType == SB3_EVALTYPE_CODEBOOK)
		{
			p->codeConsumption = 1;
			// 3.11 - Was = 0, oops
			p->byteConsumption = 1;
			p->mse = celEval->eval_mse[p->evalType];
		}
		else //if(p->evalType == SB3_EVALTYPE_SUBDIVIDE)
		{
			p->codeConsumption = 5;		// 1 for main code, 4 for the subcodes
			p->byteConsumption = 0;
			p->mse = 0;

			for(j=0;j<4;j++)
			{
				p->mse += celEval->subCels[j].eval_mse[p->subEvalTypes[j]];
				//if(p->subEvalTypes[j] == SB3_EVALTYPE_SKIP)
				//{
				//}
				//else
				if(p->subEvalTypes[j] == SB3_EVALTYPE_MOTION || p->subEvalTypes[j] == SB3_EVALTYPE_CODEBOOK)
					p->byteConsumption++;
				else if(p->subEvalTypes[j] == SB3_EVALTYPE_SUBDIVIDE)
					p->byteConsumption += 4;
			}
		}

		p->combinedBitConsumption = (p->codeConsumption * 1) + (p->byteConsumption * 4);
	}
	
	return 1;
}


// SortPossibilityData: Converts a possibility list into a list of useful possibilities,
// such that each entry is lower-quality than the one before it and consumes fewer bits.
// This does not take codebook usage into consideration to avoid chicken-and-egg problems.
// No dummy element is needed, because there will always be a lot of disallowed entries due to
// the order-forcing.  At most, only 69 entries will be valid.

static int plistBaseSort(const void *a, const void *b)
{
	const sb3_possibility_t **p1;
	const sb3_possibility_t **p2;
	i32 i;

	p1 = a;
	p2 = b;

	// Disallowed = later in the list
	if(!p2[0]->allowed)
		if(p1[0]->allowed) return -1; else return 0;
	if(!p1[0]->allowed)
		if(p2[0]->allowed) return 1; else return 0;


	// More bit consumption = earlier in the list
	i = p2[0]->combinedBitConsumption - p1[0]->combinedBitConsumption;

	if(i != 0) return i;

	// More MSE = earlier in the list
	return p2[0]->mse - p1[0]->mse;
}

static int plistBitUseSort(const void *a, const void *b)
{
	const sb3_possibility_t **p1;
	const sb3_possibility_t **p2;
	i32 i;

	p1 = a;
	p2 = b;

	// Disallowed = later in the list
	if(!p2[0]->allowed)
		if(p1[0]->allowed) return -1; else return 0;
	if(!p1[0]->allowed)
		if(p2[0]->allowed) return 1; else return 0;

	// More bit consumption = earlier in the list
	return p2[0]->combinedBitConsumption - p1[0]->combinedBitConsumption;
}


static int SortPossibilityData(sb3_possibility_list_t *plists, uint fsk)
{
	u32 i;
	int ok;
	sb3_possibility_t *last;
	sb3_possibility_t *current;

	// Sort possibility list by bit usage, then MSE, force non-allowed entries
	// to the end of the list
	for(i=0;i<SB3_MAX_POSSIBILITIES;i++)
		plists->sorted[i] = &plists->p[i];

	qsort(plists->sorted, SB3_MAX_POSSIBILITIES, sizeof(sb3_possibility_t *), plistBaseSort);

	// Remove duplicate bit consumptions so only the lowest MSE option remains for each
	// bit consumption option.  This can be done by disallowing earlier entries, since
	// they are already in high-MSE-first order
	last = plists->sorted[0];
	for(i=1;i<SB3_MAX_POSSIBILITIES;i++)
	{
		current = plists->sorted[i];
		if(!current->allowed)
			break;
		if(current->combinedBitConsumption == last->combinedBitConsumption)
			last->allowed = 0;
		last = current;
	}

	// Now, every entry with allowed==1 has its own unique bit usage, but they may not be in
	// increasing MSE, so until the list is good to go.
	ok = 0;
	while(!ok)
	{
		ok = 1;

		// Sort the list by bit consumption
		qsort(plists->sorted, SB3_MAX_POSSIBILITIES, sizeof(sb3_possibility_t *), plistBitUseSort);

		last = plists->sorted[0];
		for(i=1;i<SB3_MAX_POSSIBILITIES;i++)
		{
			current = plists->sorted[i];
			if(!current->allowed)
				break;
			if(current->mse <= last->mse)
			{
				ok = 0;
				last->allowed = 0;
			}
			last = current;
		}
	}

	return 1;
}


// GatherPossibilityData: Loads possibility lists with actual data
static int GatherPossibilityData(sb3_possibility_list_t *plists, sb3_cel_evaluation_t *celEvals, u32 numBlocks, uint fsk)
{
	while(numBlocks)
	{
		if(!GatherPossibilityDataForBlock(plists, celEvals)) return 0;
		if(!SortPossibilityData(plists, fsk)) return 0;

		plists++;
		celEvals++;
		numBlocks--;
	}

	return 1;
}

// UpdateSortOption: Updates the validity and MSE/bit changes for moving up a notch
// in a sort option list
static int UpdateSortOption(sb3_sort_option_t *sortOption)
{
	sb3_possibility_t *current;
	sb3_possibility_t *next;

	current = sortOption->list->sorted[sortOption->plistIndex];
	next = sortOption->list->sorted[sortOption->plistIndex+1];

	if(!next->allowed)
	{
		sortOption->valid = 0;
		return 1;
	}

	sortOption->valid = 1;
	sortOption->bitCostDecrease = current->combinedBitConsumption - next->combinedBitConsumption;
	sortOption->mseIncrease = next->mse - current->mse;

	return 1;
}


int initialPListSort(const void *a, const void *b)
{
	const sb3_sort_option_t **so1;
	const sb3_sort_option_t **so2;

	u32 q1, q2;

	so1 = a;
	so2 = b;

	// Sort primarily by validity
	if(!so1[0]->valid)
		if(!so2[0]->valid) return 0; else return 1;
	if(!so2[0]->valid) return -1;

	// Sort by mseGain/bitLoss
	// Cross-multiply so both have the same divisor
	q1 = so1[0]->mseIncrease * so2[0]->bitCostDecrease;
	q2 = so2[0]->mseIncrease * so1[0]->bitCostDecrease;

	// Lower MSE/bits precedes

	return q1 - q2;
}


// Template code for both add and subtract size calc modifiers
#define SIZE_CALC_BASE_CODE		\
	uint cb4Changes[4];\
	uint cb2Changes[16];\
	uint numCB4Changes=0;\
	uint numCB2Changes=0;\
	uint argumentsChange=0;\
	uint codeChange=0;\
	uint i;\
\
	codeChange = 1;\
\
	if(p->evalType == SB3_EVALTYPE_SKIP)\
	{\
	}\
	else if(p->evalType == SB3_EVALTYPE_MOTION)\
	{\
		argumentsChange++;\
	}\
	else if(p->evalType == SB3_EVALTYPE_CODEBOOK)\
	{\
		argumentsChange++;\
		cb4Changes[numCB4Changes++] = eval->cbEntry;\
	}\
	else\
	{\
		for(i=0;i<4;i++)\
		{\
			codeChange++;\
			if(p->subEvalTypes[i] == SB3_EVALTYPE_SKIP)\
			{\
			}\
			else if(p->subEvalTypes[i] == SB3_EVALTYPE_MOTION)\
				argumentsChange++;\
			else if(p->subEvalTypes[i] == SB3_EVALTYPE_CODEBOOK)\
			{\
				argumentsChange++;\
				cb4Changes[numCB4Changes++] = eval->subCels[i].cbEntry;\
			}\
			else if(p->subEvalTypes[i] == SB3_EVALTYPE_SUBDIVIDE)\
			{\
				argumentsChange+=4;\
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[0];\
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[1];\
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[2];\
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[3];\
			}\
		}\
	}


static void AddToSizeCalc(sb3_possibility_t *p, sb3_cel_evaluation_t *eval, sb3_sizecalc_t *sizeCalc)
{
	SIZE_CALC_BASE_CODE;

	// Modify CB4 entries
	for(i=0;i<numCB4Changes;i++)
	{
		if(!sizeCalc->cb4UsageCount[cb4Changes[i]])
		{
			sizeCalc->usedCB4++;
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][0];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][1];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][2];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][3];
		}

		sizeCalc->cb4UsageCount[cb4Changes[i]]++;
	}

	// Modify CB2 entries
	for(i=0;i<numCB2Changes;i++)
	{
		if(!sizeCalc->cb2UsageCount[cb2Changes[i]])
			sizeCalc->usedCB2++;

		sizeCalc->cb2UsageCount[cb2Changes[i]]++;
	}

	sizeCalc->numArguments += argumentsChange;
	sizeCalc->numCodes += codeChange;
}

static void RemoveFromSizeCalc(sb3_possibility_t *p, sb3_cel_evaluation_t *eval, sb3_sizecalc_t *sizeCalc)
{
	SIZE_CALC_BASE_CODE;

	// Modify CB4 entries
	for(i=0;i<numCB4Changes;i++)
	{
		sizeCalc->cb4UsageCount[cb4Changes[i]]--;

		if(!sizeCalc->cb4UsageCount[cb4Changes[i]])
		{
			sizeCalc->usedCB4--;
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][0];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][1];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][2];
			cb2Changes[numCB2Changes++] = sizeCalc->cb4Indexes[cb4Changes[i]][3];
		}
	}

	// Modify CB2 entries
	for(i=0;i<numCB2Changes;i++)
	{
		sizeCalc->cb2UsageCount[cb2Changes[i]]--;

		if(!sizeCalc->cb2UsageCount[cb2Changes[i]])
			sizeCalc->usedCB2--;
	}

	sizeCalc->numArguments -= argumentsChange;
	sizeCalc->numCodes -= codeChange;
}

rc_inline u32 CalculateSize(sb3_sizecalc_t *sizeCalc)
{
	u32 cbSize, mainBlockSize;
	u32 numCodeWordsSpooled;

	if(sizeCalc->usedCB2 || sizeCalc->usedCB4)
		cbSize = 8 + sizeCalc->usedCB2*6 + sizeCalc->usedCB4*4;
	else
		cbSize = 0;

	// 1 code = 2 bits, round up to 2 byte divisible
	numCodeWordsSpooled = ((sizeCalc->numCodes + 7) >> 3);

	mainBlockSize = 8 + numCodeWordsSpooled*2 + sizeCalc->numArguments;

	// Pad to 4-byte alignment???
	//mainBlockSize = ((mainBlockSize + 3) & (~3));

	return cbSize + mainBlockSize;
}


// CalculateSizeMainBlock: Returns the size of just the main block with no headers
rc_inline u32 CalculateSizeMainBlock(sb3_sizecalc_t *sizeCalc)
{
	u32 numCodeWordsSpooled;

	numCodeWordsSpooled = ((sizeCalc->numCodes + 7) >> 3);

	return numCodeWordsSpooled*2 + sizeCalc->numArguments;
}

// ReducePLists: Repeatedly reduces by the option that will cause the least MSE increase
// per bit cost increase.  This does the actual compression. :)
static int ReducePLists(sb3_possibility_list_t *plists, sb3_cel_evaluation_t *evals, sb3_cb4_entry_t *cb4, sb3_tempdata_t *tempData, u32 numBlocks, u32 sizeLimit)
{
	u32 i, idx;

	sb3_sizecalc_t sizeCalc;

	sb3_sort_option_t *sortOptions;
	sb3_sort_option_t **sortOptionsSorted;

	sb3_sort_option_t *swapTemp;

	// Allocate memory
	tempData->sortOptions = sortOptions = malloc(numBlocks * sizeof(sb3_sort_option_t));
	if(!tempData->sortOptions) return 0;
	tempData->sortOptionsSorted = sortOptionsSorted = malloc(numBlocks * sizeof(sb3_sort_option_t *));
	if(!tempData->sortOptionsSorted) return 0;

	// Set up codebook stuff
	memset(&sizeCalc, 0, sizeof(sizeCalc));

	for(i=0;i<256;i++)
	{
		sizeCalc.cb4Indexes[i][0] = cb4[i].indexes[0];
		sizeCalc.cb4Indexes[i][1] = cb4[i].indexes[1];
		sizeCalc.cb4Indexes[i][2] = cb4[i].indexes[2];
		sizeCalc.cb4Indexes[i][3] = cb4[i].indexes[3];
	}

	// Set up sort options
	for(i=0;i<numBlocks;i++)
	{
		sortOptions[i].list = plists + i;
		sortOptions[i].plistIndex = 0;
		sortOptions[i].eval = evals + i;

		UpdateSortOption(sortOptions + i);

		sortOptionsSorted[i] = sortOptions + i;
	}

	// Run initial sort
	qsort(sortOptionsSorted, numBlocks, sizeof(sb3_sort_option_t *), initialPListSort);

	// Add all current options to the size calculation
	for(i=0;i<numBlocks;i++)
		AddToSizeCalc(sortOptions[i].list->sorted[0], sortOptions[i].eval, &sizeCalc);

	while(CalculateSize(&sizeCalc) > sizeLimit)
	{
		if(!sortOptionsSorted[0]->valid) break;		// Can't be reduced any further

		// Use the most feasible downshift
		idx = sortOptionsSorted[0]->plistIndex;

		// Update the size calculator
		RemoveFromSizeCalc(sortOptionsSorted[0]->list->sorted[idx], sortOptionsSorted[0]->eval, &sizeCalc);
		AddToSizeCalc(sortOptionsSorted[0]->list->sorted[idx+1], sortOptionsSorted[0]->eval, &sizeCalc);

		// Update the actual sort option
		sortOptionsSorted[0]->plistIndex = idx+1;
		UpdateSortOption(sortOptionsSorted[0]);

		// Bubble sort to where it belongs now
		for(i=1;i<numBlocks;i++)
		{
			if(initialPListSort(&sortOptionsSorted[i-1], &sortOptionsSorted[i]) <= 0)
				break;

			swapTemp = sortOptionsSorted[i];
			sortOptionsSorted[i] = sortOptionsSorted[i-1];
			sortOptionsSorted[i-1] = swapTemp;
		}
	}

	// Make remaps for the final codebook usage
	idx = 0;
	for(i=0;i<256;i++)
	{
		if(sizeCalc.cb2UsageCount[i])
		{
			tempData->i2f2[i] = idx;
			tempData->f2i2[idx] = i;
			idx++;
		}
	}

	idx = 0;
	for(i=0;i<256;i++)
	{
		if(sizeCalc.cb4UsageCount[i])
		{
			tempData->i2f4[i] = idx;
			tempData->f2i4[idx] = i;
			idx++;
		}
	}

	tempData->numCB4 = sizeCalc.usedCB4;
	tempData->numCB2 = sizeCalc.usedCB2;

	tempData->mainChunkSize = CalculateSizeMainBlock(&sizeCalc);

	tempData->outbuffer = malloc(CalculateSize(&sizeCalc) * 3);
	if(!tempData->outbuffer) return 0;

	return 1;
}



// NOTE: Typecodes must be spooled AFTER arguments!!
#define SPOOL_ARGUMENT(arg)		\
do {\
	argumentSpool[argumentSpoolLength++] = (u8)(arg);\
	stats_numArguments++;\
} while(0)

#define SPOOL_MOTION(dx, dy)	\
do {\
	u8 arg, ax, ay;\
	ax = 8 - (u8)dx;\
	ay = 8 - (u8)dy;\
	arg = (u8)(((ax&15)<<4) | (ay&15));\
	SPOOL_ARGUMENT(arg);\
} while(0)



#define SPOOL_TYPECODE(type)		\
do {\
	typeSpool |= (type & 3) << (14 - typeSpoolLength);\
	typeSpoolLength += 2;\
	if(typeSpoolLength == 16)\
	{\
		vqData[n++] = (u8)(typeSpool >> 0);\
		vqData[n++] = (u8)(typeSpool >> 8);\
		for(a=0;a<argumentSpoolLength;a++)\
			vqData[n++] = argumentSpool[a];\
		typeSpoolLength = 0;\
		typeSpool = 0;\
		argumentSpoolLength = 0;\
	}\
	stats_numCodes++;\
} while(0)


typedef struct
{
	u32 x,y;
} sb3_subcel_offset_t;


// ReconstructImage: Reconstructs the image based and also writes the compressed data to the data buffer
static int ReconstructAndEncodeImage(sb3_encoder_t *enc, sb3_tempdata_t *tempData, u32 w, u32 h, u32 numBlocks, const sb3_hostapi_t *hapi, void *handle)
{
	u32 i, j, k, n, a;
	u32 x, y;
	u32 subX, subY;

	sb3_cel_evaluation_t *eval;
	sb3_possibility_t *p;

	u8 *output;
	u8 *cbData;
	u8 *vqData;

	u32 typeSpool=0;
	u32 typeSpoolLength=0;
	u8 argumentSpool[64];
	u32 argumentSpoolLength=0;

	u8 chunkHeader[8];
	u32 chunkSize;

	u8 arg;

	sb3_rgbapixel_t enlarged[64];

	sb3_rgbapixel_t *reconstruct;

	u32 stats_numArguments=0;
	u32 stats_numCodes=0;


	sb3_subcel_offset_t subcelOffsets[4] = {
		{0,0},
		{4,0},
		{0,4},
		{4,4},
	};

	sb3_subcel_offset_t subsubcelOffsets[4] = {
		{0,0},
		{2,0},
		{0,2},
		{2,2},
	};

	tempData->reconstruct = reconstruct = malloc(w*h*sizeof(sb3_rgbapixel_t));
	if(!tempData->reconstruct) return 0;

	output = tempData->outbuffer;

	chunkSize = tempData->numCB2*6 + tempData->numCB4*4;
	n=0;


	// Create codebook chunk
	cbData = output;
	if(tempData->numCB2)
	{
		cbData[n++] = 0x02;
		cbData[n++] = 0x10;

		cbData[n++] = (u8)(chunkSize >> 0);
		cbData[n++] = (u8)(chunkSize >> 8);
		cbData[n++] = (u8)(chunkSize >> 16);
		cbData[n++] = (u8)(chunkSize >> 24);

		cbData[n++] = (u8)(tempData->numCB4);
		cbData[n++] = (u8)(tempData->numCB2);

		for(i=0;i<tempData->numCB2;i++)
			for(j=0;j<6;j++)
				cbData[n++] = tempData->codebooks.cb2[tempData->f2i2[i]].yuv[j];

		for(i=0;i<tempData->numCB4;i++)
			for(j=0;j<4;j++)
				cbData[n++] = (u8)tempData->i2f2[tempData->codebooks.cb4[tempData->f2i4[i]].indexes[j]];

	}


	// Write the video chunk
	chunkSize = tempData->mainChunkSize;

	vqData = cbData+n;
	n = 0;

	vqData[n++] = 0x11;
	vqData[n++] = 0x10;

	vqData[n++] = (u8)(chunkSize >> 0);
	vqData[n++] = (u8)(chunkSize >> 8);
	vqData[n++] = (u8)(chunkSize >> 16);
	vqData[n++] = (u8)(chunkSize >> 24);

	vqData[n++] = 0;
	vqData[n++] = 0;


	for(i=0;i<numBlocks;i++)
	{
		eval = tempData->sortOptions[i].eval;
		p = tempData->sortOptions[i].list->sorted[tempData->sortOptions[i].plistIndex];

		x = eval->sourceX;
		y = eval->sourceY;

		switch(p->evalType)
		{
		case SB3_EVALTYPE_SKIP:
			SPOOL_TYPECODE(SB3_EVALTYPE_SKIP);
			Blit(enc->frameHistory2 + (y*w) + x, w, reconstruct + (y*w) + x, w, 8, 8);
			break;

		case SB3_EVALTYPE_MOTION:
			SPOOL_MOTION(eval->motionX, eval->motionY);
			SPOOL_TYPECODE(SB3_EVALTYPE_MOTION);

			Blit(enc->frameHistory1 + ((y+eval->motionY)*w) + (x+eval->motionX), w, reconstruct + (y*w) + x, w, 8, 8);
			break;

		case SB3_EVALTYPE_CODEBOOK:
			SPOOL_ARGUMENT(tempData->i2f4[eval->cbEntry]);
			SPOOL_TYPECODE(SB3_EVALTYPE_CODEBOOK);

			Enlarge(tempData->codebooks.cb4[eval->cbEntry].image, enlarged);
			Blit(enlarged, 8, reconstruct + (y*w) + x, w, 8, 8);
			break;

		case SB3_EVALTYPE_SUBDIVIDE:
			SPOOL_TYPECODE(SB3_EVALTYPE_SUBDIVIDE);
			for(j=0;j<4;j++)
			{
				subX = subcelOffsets[j].x + x;
				subY = subcelOffsets[j].y + y;

				switch(p->subEvalTypes[j])
				{
				case SB3_EVALTYPE_SKIP:
					SPOOL_TYPECODE(SB3_EVALTYPE_SKIP);
					Blit(enc->frameHistory2 + (subY*w) + subX, w, reconstruct + (subY*w) + subX, w, 4, 4);
					break;

				case SB3_EVALTYPE_MOTION:
					SPOOL_MOTION(eval->subCels[j].motionX, eval->subCels[j].motionY);
					SPOOL_TYPECODE(SB3_EVALTYPE_MOTION);

					Blit(enc->frameHistory1 + ((subY+eval->subCels[j].motionY)*w) + (subX+eval->subCels[j].motionX), w, reconstruct + (subY*w) + subX, w, 4, 4);
					break;

				case SB3_EVALTYPE_CODEBOOK:
					SPOOL_ARGUMENT(tempData->i2f4[eval->subCels[j].cbEntry]);
					SPOOL_TYPECODE(SB3_EVALTYPE_CODEBOOK);

					Blit(tempData->codebooks.cb4[eval->subCels[j].cbEntry].image, 4, reconstruct + (subY*w) + subX, w, 4, 4);
					break;

				case SB3_EVALTYPE_SUBDIVIDE:
					for(k=0;k<4;k++)
					{
						SPOOL_ARGUMENT(tempData->i2f2[eval->subCels[j].subCels[k]]);
						Blit(tempData->codebooks.cb2[eval->subCels[j].subCels[k]].image, 2, reconstruct + ((subY+subsubcelOffsets[k].y)*w) + (subX+subsubcelOffsets[k].x), w, 2, 2);
					}
					SPOOL_TYPECODE(SB3_EVALTYPE_SUBDIVIDE);
					break;
				}
			}
			break;
		}
	}

	// Flush the remainder of the argument/type spool
	while(typeSpoolLength)
	{
		SPOOL_TYPECODE(0);
	}

	// Write it all
	hapi->writebytes(handle, output, (vqData+n) - output);

	return 1;
}



int SwitchBlade3_EncodeVideo(sb3_encoder_t *enc, const sb3_rgbapixel_t *rgbData, const sb3_hostapi_t *hapi, void *handle)
{
	uint width, height;
	sb3_tempdata_t tempData;
	sb3_rgbapixel_t *img;

	memset(&tempData, 0, sizeof(tempData));

	width = enc->config.width;
	height = enc->config.height;

	if(!CreateCelEvals(enc, &tempData)) { FreeTempData(&tempData); return 0; }
	if(!CreatePossibilityLists(enc, &tempData)) { FreeTempData(&tempData); return 0; }
	if(!GetCodebooks(enc, &tempData, rgbData, hapi, handle)) { FreeTempData(&tempData); return 0; }
	if(!GatherCelData(enc, rgbData, &tempData))  { FreeTempData(&tempData); return 0; }
	if(!GatherPossibilityData(tempData.plists, tempData.cel_evals, width*height/64, enc->framesSinceKeyframe))  { FreeTempData(&tempData); return 0; }
	if(!ReducePLists(tempData.plists, tempData.cel_evals, tempData.codebooks.cb4, &tempData, width*height/64, enc->maxBytes))  { FreeTempData(&tempData); return 0; }
	if(!ReconstructAndEncodeImage(enc, &tempData, width, height, width*height/64, hapi, handle))  { FreeTempData(&tempData); return 0; }

	// Rotate frame history
	img = enc->frameHistory2;
	enc->frameHistory2 = enc->frameHistory1;
	enc->frameHistory1 = tempData.reconstruct;
	tempData.reconstruct = img;

	FreeTempData(&tempData);

	enc->framesSinceKeyframe++;

	return 1;
}


int BasicQuant(unsigned char *c, int count, int step, unsigned char *result)
{
	int numCels = 0;
	int i,j;
	int match;
	int mismatch;

	while(count)
	{
		// Look for a match
		match = 0;
		for(i=0;i<numCels;i++)
		{
			mismatch = 0;
			for(j=0;j<step;j++)
			{
				if(c[j] != result[i*step + j])
				{
					mismatch = 1;
					break;
				}
			}

			if(!mismatch)
			{
				match = 1;
				break;
			}
		}

		// No match?  Make a new one
		if(!match)
		{
			if(numCels == 256)
				return 0;

			for(i=0;i<step;i++)
				result[numCels*step + i] = c[i];

			numCels++;
		}

		count--;
		c += step;
	}

	return numCels;
}


int QuickRemakeYUV(unsigned char *rgb, unsigned char *out)
{
	int utotal, vtotal;
	unsigned char y,u,v;

	rgb2yuv(rgb[0], rgb[1], rgb[2], out+0, &u, &v);
	utotal = 2+u;
	vtotal = 2+v;
	rgb2yuv(rgb[3], rgb[4], rgb[5], out+1, &u, &v);
	utotal += u;
	vtotal += v;
	rgb2yuv(rgb[6], rgb[7], rgb[8], out+2, &u, &v);
	utotal += u;
	vtotal += v;
	rgb2yuv(rgb[9], rgb[10], rgb[11], out+3, &u, &v);
	utotal += u;
	vtotal += v;

	out[4] = utotal / 4;
	out[5] = vtotal / 4;

	return 1;
}

int QuickRemakeRGB(sb3_yuvcluster2_t *cluster, unsigned char *rgb)
{
	yuv2rgb(cluster->y[0], cluster->u, cluster->v, rgb+0, rgb+1, rgb+2);
	yuv2rgb(cluster->y[1], cluster->u, cluster->v, rgb+3, rgb+4, rgb+5);
	yuv2rgb(cluster->y[2], cluster->u, cluster->v, rgb+6, rgb+7, rgb+8);
	yuv2rgb(cluster->y[3], cluster->u, cluster->v, rgb+9, rgb+10, rgb+11);

	return 1;
}

#include <limits.h>
#include <stdlib.h>
#include "SB4Encoder.hpp"

enum SB4BlockType
{
	SB4BT_Skip = 0,
	SB4BT_Motion = 1,
	SB4BT_Vector = 2,
	SB4BT_Subdivide = 3
};

struct SB4SubCelEvaluation
{
	lwmUInt32 evalDist[4];

	lwmUInt32 subCels[4];

	lwmSInt32 motionX, motionY;
	lwmUInt32 cbEntry;
} roq_subcel_evaluation_t;

struct SB4CelEvaluation
{
	lwmUInt32 evalDist[3];

	SB4SubCelEvaluation subCels[4];

	lwmSInt32 motionX, motionY;
	lwmUInt32 cbEntry;

	lwmUInt32 sourceX, sourceY;
} cel_evaluation_t;

struct SB4Possibility
{
	SB4BlockType evalType;
	SB4BlockType subEvalTypes[4];

	lwmUInt32 codeConsumption;	// 2-bit typecodes
	lwmUInt32 byteConsumption;	// 8-bit arguments
	lwmUInt32 combinedBitConsumption;

	lwmUInt32 dist;

	bool allowed;
};

//4*4*4*4 subdivide types + 3 non-subdivide main types
#define ROQ_MAX_POSSIBILITIES	(4*4*4*4+3)

struct SB4PossibilityList
{
	SB4Possibility p[ROQ_MAX_POSSIBILITIES];
	SB4Possibility *sorted[ROQ_MAX_POSSIBILITIES];
};

struct SB4SortOption
{
	lwmUInt32 distIncrease;
	lwmUInt32 bitCostDecrease;

	bool valid;

	SB4PossibilityList *list;
	lwmUInt32 plistIndex;

	SB4CelEvaluation *eval;
};

struct SB4Codebooks
{
	lwmUInt32 numCB4;
	lwmUInt32 numCB2;
	SB4Block<2> unpacked_cb2[256];
	SB4Block<4> unpacked_cb4[256];
	SB4Block<8> unpacked_cb4_enlarged[256];

	inline SB4Codebooks()
	{
		memset(this, 0, sizeof(*this));
	}
};

struct SB4SizeCalc
{
	lwmUInt32 cb2UsageCount[256];
	lwmUInt32 cb4UsageCount[256];
	lwmUInt8 cb4Indexes[256][4];

	lwmUInt32 usedCB2;
	lwmUInt32 usedCB4;

	lwmUInt32 numCodes;
	lwmUInt32 numArguments;

	lwmUInt32 CalculateSize() const;
	lwmUInt32 CalculateSizeMainBlock() const;
};

struct SB4RoQTempData
{
	std::vector<SB4CelEvaluation> celEvals;
	std::vector<SB4PossibilityList> plists;
	//	roq_cell4 *yuvClusters;
	std::vector<SB4SortOption> sortOptions;
	std::vector<SB4SortOption *> sortOptionsSorted;
	std::vector<lwmUInt32> subcelPredDists;

	std::vector<lwmUInt8> outbuffer;

	lwmUInt32 f2i4[256];
	lwmUInt32 i2f4[256];
	lwmUInt32 f2i2[256];
	lwmUInt32 i2f2[256];

	lwmUInt32 mainChunkSize;

	lwmUInt32 numCB4;
	lwmUInt32 numCB2;

	SB4Codebooks codebooks;

	inline SB4RoQTempData()
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

void SB4RoQEncoder::WriteVideoInfoChunk(void *h, SB4WriteCallback cb)
{
	unsigned char chunk[16];
	unsigned char *buf = chunk;

	/* ROQ info chunk */
	PutLE16(&buf, 0x1001);	// RoQ_INFO

	/* Size: 8 bytes */
	PutLE32(&buf, 8);

	/* Unused argument */
	PutByte(&buf, 0x00);
	PutByte(&buf, 0x00);

	/* Width */
	PutLE16(&buf, static_cast<lwmUInt16>(m_width));

	/* Height */
	PutLE16(&buf, static_cast<lwmUInt16>(m_height));

	/* Unused in Quake 3, mimics the output of the real encoder */
	PutByte(&buf, 0x08);
	PutByte(&buf, 0x00);
	PutByte(&buf, 0x04);
	PutByte(&buf, 0x00);

	cb(h, chunk, buf - chunk, SB4WT_MovieInfo);
}

bool SB4RoQEncoder::EncodeVideo(lwmUInt32 highThreshold, lwmUInt32 numThresholdPhases, void *h, SB4WriteCallback cb, void *logH, SB4LogCallback logCB)
{
	SB4RoQTempData tempData;

	if (!CreateCelEvals(tempData))
		return false;

	if (!CreatePossibilityLists(tempData))
		return false;

	if (m_framesSinceKeyframe > 0)
	{
		logCB(logH, "Running motion search...\n");
		this->MotionSearch<8>(true);
		this->MotionSearch<4>(true);
	}

	// Try doing a regular encode
	if (m_framesSinceKeyframe == 0)
		m_cbOmitThreshold = 0;
	else
		m_cbOmitThreshold = highThreshold;


	SB4Image attemptImage;
	attemptImage.Init(m_width, m_height);
	std::vector<lwmUInt8> attemptBytes;

	this->m_outputFrame = &attemptImage;
	VQEncode(numThresholdPhases, tempData, attemptBytes, logH, logCB);

	lwmUInt64 attemptDist = m_dist;

	lwmFloat64 targetLambda = (lwmFloat64)(attemptDist) / (lwmFloat64)(attemptBytes.size());
	attemptImage.CopyTo(m_currentFrame);

	/* Rotate frame history */
	SimpleSwap<SB4Image *>(m_currentFrame, m_lastFrame);
	SimpleSwap<SB4MotionVector *>(m_lastMotion4, m_thisMotion4);
	SimpleSwap<SB4MotionVector *>(m_lastMotion8, m_thisMotion8);

	m_framesSinceKeyframe++;

	cb(h, &attemptBytes[0], attemptBytes.size(), SB4WT_Frame);
	return true;
}

bool SB4RoQEncoder::EncodeFrame(const SB4Image *img, lwmUInt32 highThreshold, lwmUInt32 numThresholdPhases, void *streamH, SB4WriteCallback cb, void *logH, SB4LogCallback logCB)
{
	lwmUInt32 targetSize;

	m_frameToEnc = img;

	/* Check for I frame */
	if (m_framesToKeyframe == 0)
	{
		m_framesSinceKeyframe = 0;
		m_framesToKeyframe = m_keyrate;
		targetSize = m_iframeSize;
	}
	else
		targetSize = m_pframeSize;

	m_framesToKeyframe--;

	if (m_firstFrame)
	{
		m_maxBytes = targetSize * 2 - 24;

		m_targetDist = 0;

		/* Before the first video frame, write a "video info" chunk */
		WriteVideoInfoChunk(streamH, cb);
	}
	else
	{
		lwmSInt32 slippedMax = static_cast<lwmSInt32>(targetSize * 2) - (m_slip / 4);
		if (slippedMax < 0)
			m_maxBytes = 0;
		else
			m_maxBytes = static_cast<lwmUInt32>(slippedMax);
		m_minBytes = targetSize / 2;

		//enc->maxBytes = targetSize;
		// enc->minBytes = 0;
		//enc->target_lambda = 0;
	}

	logCB(logH, "Target frame size: %i-%i  Target dist: %0.0f\n", static_cast<int>(m_minBytes), static_cast<int>(m_maxBytes), static_cast<double>(m_targetDist));

	/* Encode the actual frame */
	if (!EncodeVideo(highThreshold, numThresholdPhases, streamH, cb, logH, logCB))
		return false;

	// Update the byte slip
	m_slip += static_cast<lwmSInt32>(m_finalSize) - static_cast<lwmSInt32>(targetSize);

	// If too many easy-to-encode images show up, frames will drop below the
	// minimum, causing slip to decrease rapidly.
	// Cap slip at 4 frames worth of data.
	if (m_slip < -static_cast<lwmSInt32>(targetSize) * 4)
		m_slip = -static_cast<lwmSInt32>(targetSize) * 4;

	lwmUInt64 actualDist = m_dist;

	if (m_firstFrame)
		m_targetDist = actualDist;
	else
		m_targetDist = (m_targetDist * 6 / 10) + (actualDist * 4 / 10);

	// If we're over by more than 3 frames, nudge target dist
	if (m_slip > static_cast<lwmSInt32>(targetSize) * 3)
		m_targetDist += m_targetDist / 20;
	else if (m_slip > static_cast<lwmSInt32>(targetSize))
		m_targetDist += m_targetDist / 40;
	else if (m_slip < -static_cast<lwmSInt32>(targetSize))
		m_targetDist -= m_targetDist / 20;

	m_firstFrame = false;

	return true;
}

void SB4RoQEncoder::WriteFileHeader(void *h, SB4WriteCallback cb)
{
	static const lwmUInt8 roqHeader[] =
	{ 0x84, 0x10, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00 };

	cb(h, roqHeader, 8, SB4WT_MovieHeader);
}

bool SB4RoQEncoder::CreateCelEvals(SB4RoQTempData &tempData)
{
	tempData.celEvals.resize(m_width*m_height / 64);

	/* Map to the ROQ quadtree order */
	lwmUInt32 n = 0;
	for (lwmUInt32 y = 0; y<m_height; y += 16) {
		for (lwmUInt32 x = 0; x<m_width; x += 16) {
			tempData.celEvals[n].sourceX = x;
			tempData.celEvals[n++].sourceY = y;

			tempData.celEvals[n].sourceX = x + 8;
			tempData.celEvals[n++].sourceY = y;

			tempData.celEvals[n].sourceX = x;
			tempData.celEvals[n++].sourceY = y + 8;

			tempData.celEvals[n].sourceX = x + 8;
			tempData.celEvals[n++].sourceY = y + 8;
		}
	}

	return true;
}

bool SB4RoQEncoder::CreatePossibilityLists(SB4RoQTempData &tempData)
{
	lwmUInt32 max = m_width*m_height >> 6;

	tempData.plists.resize(max);
	SB4PossibilityList *plists = &tempData.plists[0];

	lwmUInt32 fsk = m_framesSinceKeyframe;
	while (max--)
		InitializeSinglePossibilityList(plists++, fsk);

	return true;
}

void SB4RoQEncoder::InitializeSinglePossibilityList(SB4PossibilityList *plist, lwmUInt32 fsk)
{

	plist->p[0].evalType = SB4BT_Motion;
	plist->p[0].allowed = (fsk >= 1);

	plist->p[1].evalType = SB4BT_Skip;
	plist->p[1].allowed = (fsk >= 2);

	plist->p[2].evalType = SB4BT_Vector;
	plist->p[2].allowed = 1;

	for (lwmUInt32 i = 3; i<259; i++)
		plist->p[i].allowed = false;

	/**
	* This exploits the enumeration of RoQ tags
	*  0-3 = Allow CCC(3), SLD(2), FCC(1), MOT(0)
	*  1-3 = Allow CCC(3), SLD(2), FCC(1)
	*  2-3 = Allow CCC(3), SLD(2)
	*/
	lwmUInt32 lastAllowed = 3;

	lwmUInt32 firstAllowed;
	if (fsk >= 2)
		firstAllowed = 0;
	else if (fsk >= 1)
		firstAllowed = 1;
	else
		firstAllowed = 2;

	lwmUInt32 n = 3;
	for (lwmUInt32 i = firstAllowed; i <= lastAllowed; i++)
		for (lwmUInt32 j = firstAllowed; j <= lastAllowed; j++)
			for (lwmUInt32 k = firstAllowed; k <= lastAllowed; k++)
				for (lwmUInt32 l = firstAllowed; l <= lastAllowed; l++) {
					plist->p[n].evalType = SB4BT_Subdivide;
					plist->p[n].allowed = 1;
					plist->p[n].subEvalTypes[0] = static_cast<SB4BlockType>(i);
					plist->p[n].subEvalTypes[1] = static_cast<SB4BlockType>(j);
					plist->p[n].subEvalTypes[2] = static_cast<SB4BlockType>(k);
					plist->p[n].subEvalTypes[3] = static_cast<SB4BlockType>(l);
					n++;
				}

	while (n < ROQ_MAX_POSSIBILITIES)
		plist->p[n++].allowed = false;
}


bool SB4RoQEncoder::VQEncode(lwmUInt32 numPhases, SB4RoQTempData &tempData, std::vector<lwmUInt8> &v, void *logH, SB4LogCallback logCB)
{
	lwmUInt32 highThreshold = this->m_cbOmitThreshold;
	lwmUInt32 lowThreshold = 0;

	if (m_framesSinceKeyframe == 0)
	{
		if (!GatherCelData(tempData, true))
			return false;
		logCB(logH, "Encoding key frame...\n");
		return VQThresholdPass(tempData, v, 0, m_outputFrame, &m_dist);
	}
	else
	{
		logCB(logH, "Encoding predictive frame...\n");
		std::vector<unsigned char> highOutput;
		std::vector<unsigned char> lowOutput;

		SB4Image highImage;
		lwmUInt64 highDist;
		highImage.Init(m_outputFrame->Width(), m_outputFrame->Height());
		SB4Image lowImage;
		lwmUInt64 lowDist;
		lowImage.Init(m_outputFrame->Width(), m_outputFrame->Height());

		SB4RandomState randState = m_rng;

		// Run motion pass
		if (!GatherCelData(tempData, true))
			return false;

		if (m_framesSinceKeyframe > 0)
		{
			tempData.subcelPredDists.resize(m_width * m_height / 16);

			lwmUInt32 celWidth = m_width / 8;
			lwmUInt32 celHeight = m_height / 8;

			const SB4CelEvaluation *celEval = &tempData.celEvals[0];
			for (lwmUInt32 cely = 0; cely<celHeight; cely++)
			{
				for (lwmUInt32 celx = 0; celx<celWidth; celx++)
				{
					lwmUInt32 *distLocs[4];
					distLocs[0] = &tempData.subcelPredDists[(cely * 2) * (celWidth * 2) + (celx * 2)];
					distLocs[1] = distLocs[0] + 1;
					distLocs[2] = distLocs[0] + (celWidth * 2);
					distLocs[3] = distLocs[2] + 1;

					for (lwmUInt32 subEvalIdx = 0; subEvalIdx<4; subEvalIdx++)
					{
						const SB4SubCelEvaluation *subcelEval = celEval->subCels + subEvalIdx;
						unsigned int skipDist = subcelEval->evalDist[SB4BT_Skip];
						unsigned int motionDist = subcelEval->evalDist[SB4BT_Motion];
						*distLocs[subEvalIdx] = (skipDist < motionDist) ? skipDist : motionDist;
					}
					celEval++;
				}
			}
		}

		// Generate initial high/low thresholds
		if (!VQThresholdPass(tempData, highOutput, m_cbOmitThreshold, &highImage, &highDist))
			return false;
		if (!VQThresholdPass(tempData, lowOutput, 0, &lowImage, &lowDist))
			return false;

		lwmUInt32 spanLimit = 0;
		while (true)
		{
			lwmUInt32 midThreshold = (lowThreshold + highThreshold) / 2;
			if (numPhases == 0 || midThreshold == lowThreshold || highThreshold - midThreshold < spanLimit)
			{
				if (lowDist < highDist)
				{
					v = lowOutput;
					m_dist = lowDist;
					lowImage.CopyTo(m_outputFrame);
				}
				else
				{
					v = highOutput;
					m_dist = highDist;
					highImage.CopyTo(m_outputFrame);
				}
				logCB(logH, "Finished frame.  Size: %i bytes   Dist: %0.0f\n", static_cast<int>(v.size()), static_cast<double>(m_dist));
				break;
			}

			numPhases--;

			SB4Image midImage;
			lwmUInt64 midDist;
			std::vector<unsigned char> midOutput;
			midImage.Init(m_outputFrame->Width(), m_outputFrame->Height());

			if (!VQThresholdPass(tempData, midOutput, midThreshold, &midImage, &midDist))
				return false;

			if (lowDist < highDist)
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

	return true;
}


bool SB4RoQEncoder::GatherCelData(SB4RoQTempData &tempData, bool motionPass)
{
	lwmUInt32 max;
	SB4CelEvaluation *celEvals;

	max = m_width*m_height / 64;
	celEvals = &tempData.celEvals[0];

	while (max--)
	{
		GatherDataForCel(tempData, celEvals, motionPass);
		celEvals++;
	}

	return true;
}

// Gets distortion for all options available to a cel
void SB4RoQEncoder::GatherDataForCel(SB4RoQTempData &tempData, SB4CelEvaluation *cel, bool motionPass)
{
	SB4Block<8> mb8;
	SB4Block<8> pmb8;

	lwmUInt32 index = cel->sourceY*m_width / 64 + cel->sourceX / 8;

	if (m_framesSinceKeyframe >= 1)
	{
		cel->motionX = m_thisMotion8[index].dx;
		cel->motionY = m_thisMotion8[index].dy;
		if (motionPass)
			cel->evalDist[SB4BT_Motion] = EvalMotionDist<8>(cel->sourceX, cel->sourceY,
			m_thisMotion8[index].dx, m_thisMotion8[index].dy);
	}

	if (m_framesSinceKeyframe >= 2)
	{
		if (motionPass)
			cel->evalDist[SB4BT_Skip] = BlockSSE<8>(this->m_frameToEnc,
			this->m_currentFrame,
			cel->sourceX, cel->sourceY,
			cel->sourceX, cel->sourceY);
	}

	mb8.LoadFromImage(m_frameToEnc, cel->sourceX, cel->sourceY);

	if (!motionPass)
		cel->cbEntry = mb8.IndexList(tempData.codebooks.unpacked_cb4_enlarged, 256, mb8.LumaAverage(), &cel->evalDist[SB4BT_Vector]);

	GatherDataForSubCel(tempData, cel->subCels + 0, cel->sourceX + 0, cel->sourceY + 0, motionPass);
	GatherDataForSubCel(tempData, cel->subCels + 1, cel->sourceX + 4, cel->sourceY + 0, motionPass);
	GatherDataForSubCel(tempData, cel->subCels + 2, cel->sourceX + 0, cel->sourceY + 4, motionPass);
	GatherDataForSubCel(tempData, cel->subCels + 3, cel->sourceX + 4, cel->sourceY + 4, motionPass);
}

// Gets distortion for all options available to a subcel
void SB4RoQEncoder::GatherDataForSubCel(SB4RoQTempData &tempData, SB4SubCelEvaluation *subcel, lwmUInt32 x, lwmUInt32 y, bool motionPass)
{
	/*
	SB4Block<4> pmb4;
	SB4Block<2> pmb2;
	unsigned int diff;
	int cluster_index;
	int i;
	*/
	lwmUInt32 ssc_offsets[4][2] =
	{
		{ 0, 0 },
		{ 2, 0 },
		{ 0, 2 },
		{ 2, 2 },
	};

	if (m_framesSinceKeyframe >= 1)
	{
		if (motionPass)
		{
			subcel->motionX = m_thisMotion4[y*m_width / 16 + x / 4].dx;
			subcel->motionY = m_thisMotion4[y*m_width / 16 + x / 4].dy;
			subcel->evalDist[SB4BT_Motion] =
				EvalMotionDist<4>(x, y,
				m_thisMotion4[y*m_width / 16 + x / 4].dx,
				m_thisMotion4[y*m_width / 16 + x / 4].dy);
		}
	}
	else
		subcel->evalDist[SB4BT_Motion] = INT_MAX;

	if (m_framesSinceKeyframe >= 2)
	{
		if (motionPass)
		{
			subcel->evalDist[SB4BT_Skip] = BlockSSE<4>(m_frameToEnc,
				m_currentFrame, x, y, x, y);
		}
	}
	else
		subcel->evalDist[SB4BT_Skip] = INT_MAX;

	if (motionPass)
		return;

	lwmUInt32 clusterIndex = y*m_width / 16 + x / 4;

	SB4Block<4> mb4;
	mb4.LoadFromImage(m_frameToEnc, x, y);
	subcel->cbEntry = mb4.IndexList(tempData.codebooks.unpacked_cb4, 256, mb4.LumaAverage(), &subcel->evalDist[SB4BT_Vector]);

	subcel->evalDist[SB4BT_Subdivide] = 0;
	for (lwmUInt32 i = 0; i<4; i++)
	{
		SB4Block<2> mb2;
		mb2.LoadFromImage(m_frameToEnc, x + ssc_offsets[i][0], y + ssc_offsets[i][1]);
		if (!motionPass)
		{
			lwmUInt32 diff;
			subcel->subCels[i] = mb2.IndexList(tempData.codebooks.unpacked_cb2, 256, mb2.LumaAverage(), &diff);
			subcel->evalDist[SB4BT_Subdivide] += diff;
		}
	}
}

static void vectorExpandCallback(void *h, const void *bytes, lwmLargeUInt numBytes, SB4WriteType writeType)
{
	std::vector<lwmUInt8> *vh;

	vh = static_cast<std::vector<lwmUInt8> *>(h);

	lwmLargeUInt oldSize = vh->size();

	vh->resize(numBytes + oldSize);
	memcpy(&(*vh)[oldSize], bytes, numBytes);
}

bool SB4RoQEncoder::VQThresholdPass(SB4RoQTempData &tempData, std::vector<lwmUInt8> &v, lwmUInt32 threshold, SB4Image *outImage, lwmUInt64 *outDist)
{
	if (m_framesSinceKeyframe == 0)
		MakeCodebooks(m_cb2, m_cb4, &m_rng, NULL, m_frameToEnc, NULL, 0);
	else
		MakeCodebooks(m_cb2, m_cb4, &m_rng, m_lastFrame, m_frameToEnc, &tempData.subcelPredDists[0], threshold);

	if (!UnpackCodebooks(tempData))
		return false;

	if (!GatherCelData(tempData, false))
		return false;

	if (!GatherPossibilityData(&tempData.plists[0], &tempData.celEvals[0],
		m_width*m_height / 64))
		return false;

	if (!ReducePLists(tempData, &tempData.plists[0], &tempData.celEvals[0],
		m_width*m_height / 64, m_minBytes, m_maxBytes, outDist))
		return false;

	if (!ReconstructAndEncodeImage(tempData, m_width, m_height,
		m_width*m_height / 64, &v, outImage, vectorExpandCallback))
		return false;
	return true;
}

bool SB4RoQEncoder::UnpackCodebooks(SB4RoQTempData &tempData)
{
	lwmUInt32 i;

	UnpackAllCB2(m_cb2, tempData.codebooks.unpacked_cb2);
	UnpackAllCB4(tempData.codebooks.unpacked_cb2, m_cb4, tempData.codebooks.unpacked_cb4);

	for (i = 0; i<256; i++)
		tempData.codebooks.unpacked_cb4_enlarged[i] = Enlarge4to8(tempData.codebooks.unpacked_cb4[i]);

	return true;
}

// Loads possibility lists with actual data
bool SB4RoQEncoder::GatherPossibilityData(SB4PossibilityList *plists, SB4CelEvaluation *celEvals, lwmUInt32 numBlocks)
{
	while (numBlocks--)
	{
		GatherPossibilityDataForBlock(plists, celEvals);
		SortPossibilityData(plists);

		plists++;
		celEvals++;
	}

	return true;
}


// Loads possibility lists with actual data for one block,
// assigning all possibilities a cached SE and bit consumption
bool SB4RoQEncoder::GatherPossibilityDataForBlock(SB4PossibilityList *plist, SB4CelEvaluation *celEval)
{
	for (lwmUInt32 i = 0; i < ROQ_MAX_POSSIBILITIES; i++)
	{
		SB4Possibility *p = plist->p + i;
		if (!p->allowed)
			continue;

		if (p->evalType == SB4BT_Skip)
		{
			p->codeConsumption = 1;
			p->byteConsumption = 0;
			p->dist = celEval->evalDist[SB4BT_Skip];
		}
		else if (p->evalType == SB4BT_Motion || p->evalType == SB4BT_Vector)
		{
			p->codeConsumption = 1;
			p->byteConsumption = 1;
			p->dist = celEval->evalDist[p->evalType];
		}
		else
		{ //if (p->evalType == SB4BT_Subdivide)
			p->codeConsumption = 5;	  // 1 for main code, 4 for the subcodes
			p->byteConsumption = 0;
			p->dist = 0;

			for (lwmUInt32 j = 0; j < 4; j++)
			{
				p->dist += celEval->subCels[j].evalDist[p->subEvalTypes[j]];

				if (p->subEvalTypes[j] == SB4BT_Motion ||
					p->subEvalTypes[j] == SB4BT_Vector)
					p->byteConsumption++;
				else if (p->subEvalTypes[j] == SB4BT_Subdivide)
					p->byteConsumption += 4;
			}
		}

		p->combinedBitConsumption = p->codeConsumption + 4 * p->byteConsumption;
	}

	return true;
}

// Builds a possibility list such that each entry is a less - efficient use of
// the bit budget than the previous one.The first entry is the
// lowest - distortion entry, or if tied, the one that consumes the fewest bits.
// TODO: Bias towards options that don't use the codebook
bool SB4RoQEncoder::SortPossibilityData(SB4PossibilityList *plists)
{
	/* Find the best-quality possibility.  If there's a tie, find the cheapest. */
	SB4Possibility *best = NULL;
	for (lwmUInt32 i = 0; i<ROQ_MAX_POSSIBILITIES; i++)
	{
		SB4Possibility *cmp = plists->p + i;
		if (!cmp->allowed)
			continue;

		if (!best || plists->p[i].dist < best->dist ||
			(cmp->dist == best->dist && cmp->combinedBitConsumption < best->combinedBitConsumption))
			best = cmp;
	}

	lwmUInt32 nvp = 1;
	plists->sorted[0] = best;

	while (true)
	{
		SB4Possibility *base = plists->sorted[nvp - 1];

		best = NULL;
		/* Find the best exchange for bit budget from the previous best */
		for (lwmUInt32 i = 0; i<ROQ_MAX_POSSIBILITIES; i++)
		{
			SB4Possibility *cmp = plists->p + i;
			if (!cmp->allowed || cmp->dist <= base->dist || cmp->combinedBitConsumption >= base->combinedBitConsumption)
				continue;
			if (!best ||
				((cmp->dist - base->dist) * (base->combinedBitConsumption - best->combinedBitConsumption)
				< (best->dist - base->dist) * (base->combinedBitConsumption - cmp->combinedBitConsumption)))
				best = cmp;
		}

		if (best)
			plists->sorted[nvp++] = best;
		else
			break;
	}
	plists->sorted[nvp] = NULL;

	return true;
}

static int initial_plist_sort(const void *a, const void *b)
{
	const SB4SortOption *so1 = *(static_cast<const SB4SortOption *const*>(a));
	const SB4SortOption *so2 = *(static_cast<const SB4SortOption *const*>(b));

	/* Sort primarily by validity */
	if (!so1->valid)
		return so2->valid;

	if (!so2->valid)
		return -1;

	/* Sort by seGain/bitLoss
	* Cross-multiply so both have the same divisor */
	lwmUInt32 q1 = so1->distIncrease * so2->bitCostDecrease;
	lwmUInt32 q2 = so2->distIncrease * so1->bitCostDecrease;

	/* Lower SE/bits precedes */
	if (q2 > q1)
		return -1;
	if (q1 > q2)
		return 1;
	/* Compare pointer addresses to force consistency */
	if (so2 > so1)
		return -1;
	if (so1 > so2)
		return 1;
	return 0;
}

// Repeatedly reduces by the option that will cause the least distortion increase
// per bit cost increase.  This does the actual compression.
bool SB4RoQEncoder::ReducePLists(SB4RoQTempData &tempData, SB4PossibilityList *plists, SB4CelEvaluation *evals, lwmUInt32 numBlocks,
	lwmUInt32 sizeLowerLimit, lwmUInt32 sizeLimit, lwmUInt64 *outDist)
{
	/* Allocate memory */
	tempData.sortOptions.resize(numBlocks);
	SB4SortOption *sortOptions = &tempData.sortOptions[0];

	tempData.sortOptionsSorted.resize(numBlocks);
	SB4SortOption **sortOptionsSorted = &tempData.sortOptionsSorted[0];

	/* Set up codebook stuff */
	SB4SizeCalc sizeCalc;
	memset(&sizeCalc, 0, sizeof(sizeCalc));

	for (lwmUInt32 i = 0; i<256; i++)
	{
		sizeCalc.cb4Indexes[i][0] = m_cb4[i * 4 + 0];
		sizeCalc.cb4Indexes[i][1] = m_cb4[i * 4 + 1];
		sizeCalc.cb4Indexes[i][2] = m_cb4[i * 4 + 2];
		sizeCalc.cb4Indexes[i][3] = m_cb4[i * 4 + 3];
	}

	/* Set up sort options */
	for (lwmUInt32 i = 0; i<numBlocks; i++)
	{
		sortOptions[i].list = plists + i;
		sortOptions[i].plistIndex = 0;
		sortOptions[i].eval = evals + i;

		UpdateSortOption(sortOptions + i);

		sortOptionsSorted[i] = sortOptions + i;
	}

	/* Run initial sort */
	qsort(sortOptionsSorted, numBlocks, sizeof(SB4SortOption *),
		initial_plist_sort);

	/* Add all current options to the size calculation */
	lwmUInt64 dist = 0;
	for (lwmUInt32 i = 0; i<numBlocks; i++)
	{
		dist += sortOptions[i].list->sorted[0]->dist;
		ModifySizeCalc(sortOptions[i].list->sorted[0], sortOptions[i].eval, &sizeCalc, 1);
	}

	lwmUInt32 belowDist = 1;
	while (sizeCalc.CalculateSize() > sizeLimit || belowDist || sizeCalc.CalculateSizeMainBlock() >= 65536)
	{
		if (!sortOptionsSorted[0]->valid)
			break;		// Can't be reduced any further

		/* Use the most feasible downshift */
		lwmUInt32 idx = sortOptionsSorted[0]->plistIndex;

		dist += sortOptionsSorted[0]->distIncrease;

		if (!m_targetDist || sizeCalc.CalculateSize() < sizeLowerLimit)
			belowDist = 0;
		else
			belowDist = dist < m_targetDist;

		/* Update the size calculator */
		ModifySizeCalc(sortOptionsSorted[0]->list->sorted[idx],
			sortOptionsSorted[0]->eval, &sizeCalc, -1);
		ModifySizeCalc(sortOptionsSorted[0]->list->sorted[idx + 1],
			sortOptionsSorted[0]->eval, &sizeCalc, 1);

		/* Update the actual sort option */
		sortOptionsSorted[0]->plistIndex = idx + 1;
		UpdateSortOption(sortOptionsSorted[0]);

		/* Bubble sort to where it belongs now */
		for (lwmUInt32 i = 1; i<numBlocks; i++) {
			if (initial_plist_sort(&sortOptionsSorted[i - 1], &sortOptionsSorted[i]) <= 0)
				break;

			SB4SortOption *swapTemp = sortOptionsSorted[i];
			sortOptionsSorted[i] = sortOptionsSorted[i - 1];
			sortOptionsSorted[i - 1] = swapTemp;
		}
	}

	*outDist = dist;

	/* Make remaps for the final codebook usage */
	lwmUInt32 idx = 0;
	for (lwmUInt32 i = 0; i<256; i++)
	{
		if (sizeCalc.cb2UsageCount[i] || sizeCalc.usedCB4 == 256)
		{
			tempData.i2f2[i] = idx;
			tempData.f2i2[idx] = i;
			idx++;
		}
	}

	idx = 0;
	for (lwmUInt32 i = 0; i<256; i++)
	{
		if (sizeCalc.cb4UsageCount[i])
		{
			tempData.i2f4[i] = idx;
			tempData.f2i4[idx] = i;
			idx++;
		}
	}

	tempData.numCB4 = sizeCalc.usedCB4;
	tempData.numCB2 = sizeCalc.usedCB2;
	if (sizeCalc.usedCB4 == 256)
		tempData.numCB2 = 256;

	tempData.mainChunkSize = sizeCalc.CalculateSizeMainBlock();

	lwmUInt32 calculatedSize = sizeCalc.CalculateSize() * 3;
	std::vector<unsigned char> test;
	test.resize(calculatedSize);
	test.resize(calculatedSize - 1);

	tempData.outbuffer.resize(calculatedSize);

	return true;
}

// Updates the validity and distortion / bit changes for moving up a notch
// in a sort option list
void SB4RoQEncoder::UpdateSortOption(SB4SortOption *sortOption)
{
	SB4Possibility *current;
	SB4Possibility *next;

	current = sortOption->list->sorted[sortOption->plistIndex];
	next = sortOption->list->sorted[sortOption->plistIndex + 1];

	if (!next)
	{
		sortOption->valid = 0;
		return;
	}

	sortOption->valid = 1;
	sortOption->bitCostDecrease = current->combinedBitConsumption -
		next->combinedBitConsumption;
	sortOption->distIncrease = next->dist - current->dist;
}

void SB4RoQEncoder::ModifySizeCalc(SB4Possibility *p, SB4CelEvaluation *eval, SB4SizeCalc *sizeCalc, int mod)
{
	lwmUInt32 cb4Changes[4];
	lwmUInt32 cb2Changes[16];
	lwmUInt32 numCB4Changes = 0;
	lwmUInt32 numCB2Changes = 0;
	lwmUInt32 argumentsChange = 0;
	lwmUInt32 codeChange = 1;

	if (p->evalType == SB4BT_Skip)
	{
	}
	else if (p->evalType == SB4BT_Motion)
	{
		argumentsChange++;
	}
	else if (p->evalType == SB4BT_Vector)
	{
		argumentsChange++;
		cb4Changes[numCB4Changes++] = eval->cbEntry;
	}
	else
	{
		for (lwmUInt32 i = 0; i<4; i++)
		{
			codeChange++;
			if (p->subEvalTypes[i] == SB4BT_Skip)
			{
			}
			else if (p->subEvalTypes[i] == SB4BT_Motion)
				argumentsChange++;
			else if (p->subEvalTypes[i] == SB4BT_Vector)
			{
				argumentsChange++;
				cb4Changes[numCB4Changes++] = eval->subCels[i].cbEntry;
			}
			else if (p->subEvalTypes[i] == SB4BT_Subdivide)
			{
				argumentsChange += 4;
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[0];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[1];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[2];
				cb2Changes[numCB2Changes++] = eval->subCels[i].subCels[3];
			}
		}
	}

	/* Modify CB4 entries */
	for (lwmUInt32 i = 0; i<numCB4Changes; i++)
	{
		if (mod == -1)
			sizeCalc->cb4UsageCount[cb4Changes[i]]--;

		if (!sizeCalc->cb4UsageCount[cb4Changes[i]])
		{
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
	for (lwmUInt32 i = 0; i<numCB2Changes; i++)
	{
		if (mod == -1)
			sizeCalc->cb2UsageCount[cb2Changes[i]]--;

		if (!sizeCalc->cb2UsageCount[cb2Changes[i]])
			sizeCalc->usedCB2 += mod;

		if (mod == 1)
			sizeCalc->cb2UsageCount[cb2Changes[i]]++;
	}

	if (mod == 1)
	{
		sizeCalc->numArguments += argumentsChange;
		sizeCalc->numCodes += codeChange;
	}
	if (mod == -1)
	{
		sizeCalc->numArguments -= argumentsChange;
		sizeCalc->numCodes -= codeChange;
	}
}

#define SPOOL_ARGUMENT(arg)		\
do {\
	argumentSpool[argumentSpoolLength++] = (unsigned char)(arg);\
} while(0)

#define SPOOL_MOTION(dx, dy)	\
do {\
	lwmUInt8 arg, ax, ay;\
	ax = 8 - static_cast<lwmUInt8>(dx);\
	ay = 8 - static_cast<lwmUInt8>(dy);\
	arg = static_cast<lwmUInt8>(((ax&15)<<4) | (ay&15));\
	SPOOL_ARGUMENT(arg);\
} while(0)



#define SPOOL_TYPECODE(type)		\
do {\
	typeSpool |= ((type) & 3) << (14 - typeSpoolLength);\
	typeSpoolLength += 2;\
	if (typeSpoolLength == 16) {\
		PutLE16(&vqData, typeSpool); \
		for (lwmUInt32 a=0; a<argumentSpoolLength; a++)\
			PutByte(&vqData, argumentSpool[a]); \
		typeSpoolLength = 0;\
		typeSpool = 0;\
		argumentSpoolLength = 0;\
	}\
} while(0)

bool SB4RoQEncoder::ReconstructAndEncodeImage(SB4RoQTempData &tempData, lwmUInt32 w, lwmUInt32 h, lwmUInt32 numBlocks, void *handle, SB4Image *outImage, SB4WriteCallback cb)
{
	struct SubcelOffset
	{
		lwmUInt32 x, y;
	};

	lwmUInt32 typeSpool = 0;
	lwmUInt32 typeSpoolLength = 0;
	lwmUInt8 argumentSpool[64];
	lwmUInt32 argumentSpoolLength = 0;

	SubcelOffset subcelOffsets[4] = {
		{ 0, 0 },
		{ 4, 0 },
		{ 0, 4 },
		{ 4, 4 },
	};

	SubcelOffset subsubcelOffsets[4] = {
		{ 0, 0 },
		{ 2, 0 },
		{ 0, 2 },
		{ 2, 2 },
	};

	lwmUInt8 *output = &tempData.outbuffer[0];

	lwmUInt32 chunkSize = tempData.numCB2 * 6 + tempData.numCB4 * 4;

	/* Create codebook chunk */
	lwmUInt8 *cbData = output;
	if (tempData.numCB2)
	{
		PutLE16(&cbData, 0x1002);	// RoQ_QUAD_CODEBOOK

		PutLE32(&cbData, chunkSize);
		PutByte(&cbData, tempData.numCB4);
		PutByte(&cbData, tempData.numCB2);

		for (lwmUInt32 i = 0; i<tempData.numCB2; i++) {
			for (lwmUInt32 j = 0; j<6; j++)
				PutByte(&cbData, m_cb2[tempData.f2i2[i] * 6 + j]);
		}

		for (lwmUInt32 i = 0; i<tempData.numCB4; i++)
			for (lwmUInt32 j = 0; j<4; j++)
				PutByte(&cbData, tempData.i2f2[m_cb4[tempData.f2i4[i] * 4 + j]]);

	}

	/* Write the video chunk */
	chunkSize = tempData.mainChunkSize;

	lwmUInt8 *vqData = cbData;

	PutLE16(&vqData, 0x1011);	// RoQ_QUAD_VQ
	PutLE32(&vqData, chunkSize);
	PutByte(&vqData, 0x0);
	PutByte(&vqData, 0x0);

	for (lwmUInt32 i = 0; i<numBlocks; i++)
	{
		SB4CelEvaluation *eval = tempData.sortOptions[i].eval;
		SB4Possibility *p = tempData.sortOptions[i].list->sorted[tempData.sortOptions[i].plistIndex];

		lwmUInt32 x = eval->sourceX;
		lwmUInt32 y = eval->sourceY;

		switch (p->evalType)
		{
		case SB4BT_Skip:
			{
				SPOOL_TYPECODE(SB4BT_Skip);

				SB4Block<8> block8;
				block8.LoadFromImage(m_currentFrame, x, y);
				block8.Blit(outImage, x, y);
			}
			break;

		case SB4BT_Motion:
			{
				SPOOL_MOTION(eval->motionX, eval->motionY);
				SPOOL_TYPECODE(SB4BT_Motion);

				SB4Block<8> block8;
				block8.LoadFromImage(m_lastFrame, static_cast<lwmSInt32>(x) + eval->motionX, static_cast<lwmSInt32>(y) + eval->motionY);
				block8.Blit(outImage, x, y);
			}
			break;

		case SB4BT_Vector:
			{
				SPOOL_ARGUMENT(tempData.i2f4[eval->cbEntry]);
				SPOOL_TYPECODE(SB4BT_Vector);

				tempData.codebooks.unpacked_cb4_enlarged[eval->cbEntry].Blit(outImage, x, y);
			}
			break;

		case SB4BT_Subdivide:
			{
				SPOOL_TYPECODE(SB4BT_Subdivide);
				for (lwmUInt32 j = 0; j < 4; j++)
				{
					lwmUInt32 subX = subcelOffsets[j].x + x;
					lwmUInt32 subY = subcelOffsets[j].y + y;

					switch (p->subEvalTypes[j])
					{
					case SB4BT_Skip:
						{
							SPOOL_TYPECODE(SB4BT_Skip);

							SB4Block<4> block4;
							block4.LoadFromImage(m_currentFrame, subX, subY);
							block4.Blit(outImage, subX, subY);
						}
						break;

					case SB4BT_Motion:
						{
							SPOOL_MOTION(eval->subCels[j].motionX, eval->subCels[j].motionY);
							SPOOL_TYPECODE(SB4BT_Motion);

							SB4Block<4> block4;
							block4.LoadFromImage(m_lastFrame, static_cast<lwmSInt32>(subX) + eval->subCels[j].motionX,
								static_cast<lwmSInt32>(subY) + eval->subCels[j].motionY);
							block4.Blit(outImage, subX, subY);
						}
						break;

					case SB4BT_Vector:
						{
							SPOOL_ARGUMENT(tempData.i2f4[eval->subCels[j].cbEntry]);
							SPOOL_TYPECODE(SB4BT_Vector);

							tempData.codebooks.unpacked_cb4[eval->subCels[j].cbEntry].Blit(outImage, subX, subY);
						}
						break;

					case SB4BT_Subdivide:
						{
							for (lwmUInt32 k = 0; k < 4; k++)
							{
								SPOOL_ARGUMENT(tempData.i2f2[eval->subCels[j].subCels[k]]);

								lwmUInt32 fragX = subX + subsubcelOffsets[k].x;
								lwmUInt32 fragY = subY + subsubcelOffsets[k].y;

								tempData.codebooks.unpacked_cb2[eval->subCels[j].subCels[k]].Blit(outImage, fragX, fragY);
							}
							SPOOL_TYPECODE(SB4BT_Subdivide);
						}
						break;
					}
				}
			}
			break;
		}
	}

	/* Flush the remainder of the argument/type spool */
	while (typeSpoolLength)
	{
		SPOOL_TYPECODE(0);
	}

	/* Write it all */
	cb(handle, output, vqData - output, SB4WT_Frame);
	m_finalSize = static_cast<lwmUInt32>(vqData - output);

	return true;
}


template<class T>
inline void SB4RoQEncoder::SimpleSwap(T &a, T &b)
{
	T temp(a);
	a = b;
	b = temp;
}

#define EVAL_MOTION(MOTION) \
do	\
{	\
	lwmUInt32 diff = EvalMotionDist<TD>(j, i, (MOTION).dx, \
	(MOTION).dy); \
	if (diff < lowestdiff)\
	{\
		lowestdiff = diff; \
		bestpick = MOTION; \
	} \
} while (0)

template <lwmUInt32 TD>
void SB4RoQEncoder::MotionSearch(bool fullSearch)
{
	SB4MotionVector offsets[9] = {
		{ 0, 0 },
		{ 0, -1 },
		{ 0, 1 },
		{ -1, 0 },
		{ 1, 0 },
		{ -1, 1 },
		{ 1, -1 },
		{ -1, -1 },
		{ 1, 1 },
	};

	lwmUInt32 max = (m_width / TD)*(m_height / TD);

	SB4MotionVector *lastMotion;
	SB4MotionVector *thisMotion;
	if (TD == 4)
	{
		lastMotion = m_lastMotion4;
		thisMotion = m_thisMotion4;
	}
	else
	{
		lastMotion = m_lastMotion8;
		thisMotion = m_thisMotion8;
	}

	for (lwmUInt32 i = 0; i<m_height; i += TD)
		for (lwmUInt32 j = 0; j<m_width; j += TD)
		{
			lwmUInt32 lowestdiff = this->EvalMotionDist<TD>(j, i, 0, 0);
			SB4MotionVector bestpick;
			bestpick.dx = 0;
			bestpick.dy = 0;

			if (fullSearch)
			{
				for (lwmSInt32 y = -7; y <= 8; y++)
				{
					for (lwmSInt32 x = -7; x <= 8; x++)
					{
						SB4MotionVector vect;
						vect.dx = x;
						vect.dy = y;
						EVAL_MOTION(vect);
					}
				}
			}
			else
			{
				if (TD == 4)
					EVAL_MOTION(m_thisMotion8[(i / 8)*(m_width / 8) + j / 8]);

				lwmUInt32 offset = (i / TD)*(m_width / TD) + j / TD;
				if (offset < max && offset >= 0)
					EVAL_MOTION(lastMotion[offset]);

				offset++;
				if (offset < max && offset >= 0)
					EVAL_MOTION(lastMotion[offset]);

				offset = (i / TD + 1)*(m_width / TD) + j / TD;
				if (offset < max && offset >= 0)
					EVAL_MOTION(lastMotion[offset]);

				lwmSInt32 off[3];
				off[0] = static_cast<lwmSInt32>((i / TD)*(m_width / TD) + j / TD) - 1;
				off[1] = off[0] - static_cast<lwmSInt32>((m_width / TD) + 1);
				off[2] = off[1] + 1;
				if (i)
				{
					SB4MotionVector vect;
					vect.dx = MidPred(thisMotion[off[0]].dx, thisMotion[off[1]].dx, thisMotion[off[2]].dx);
					vect.dy = MidPred(thisMotion[off[0]].dy, thisMotion[off[1]].dy, thisMotion[off[2]].dy);
					EVAL_MOTION(vect);
					for (lwmUInt32 k = 0; k<3; k++)
						EVAL_MOTION(thisMotion[off[k]]);
				}
				else if (j)
					EVAL_MOTION(thisMotion[off[0]]);

				SB4MotionVector vect = bestpick;

				lwmUInt32 oldbest = INT_MAX;
				while (oldbest != lowestdiff)
				{
					oldbest = lowestdiff;
					for (lwmUInt32 k = 0; k<9; k++) {
						SB4MotionVector vect2 = vect;
						vect2.dx += offsets[k].dx;
						vect2.dy += offsets[k].dy;
						EVAL_MOTION(vect2);
					}
					vect = bestpick;
				}
			}
			lwmUInt32 offset = (i / TD)*(m_width / TD) + j / TD;
			thisMotion[offset] = bestpick;
		}

}


template <lwmUInt32 TD>
inline lwmUInt32 SB4RoQEncoder::EvalMotionDist(lwmUInt32 x, lwmUInt32 y, lwmSInt32 mx, lwmSInt32 my)
{
	lwmSInt32 sx = static_cast<lwmSInt32>(x);
	lwmSInt32 sy = static_cast<lwmSInt32>(y);
	if (mx < -7 || mx > 8)
		return INT_MAX;

	if (my < -7 || my > 8)
		return INT_MAX;

	if ((sx + mx < 0) || (static_cast<lwmUInt32>(sx + mx) > m_width - TD))
		return INT_MAX;

	if ((sy + my < 0) || (static_cast<lwmUInt32>(sy + my) > m_height - TD))
		return INT_MAX;


	return BlockSSE<TD>(m_frameToEnc, m_lastFrame, x, y,
		static_cast<lwmUInt32>(sx + mx), static_cast<lwmUInt32>(sy + my));
}

template <lwmUInt32 TD>
inline lwmUInt32 SB4RoQEncoder::BlockSSE(const SB4Image *img1, const SB4Image *img2, lwmUInt32 x1, lwmUInt32 y1, lwmUInt32 x2, lwmUInt32 y2)
{
	SB4Block<TD> b1(img1, x1, y1);
	SB4Block<TD> b2(img2, x2, y2);

	return b1.SquareDiff(b2);

}

lwmSInt32 SB4RoQEncoder::MidPred(lwmSInt32 a, lwmSInt32 b, lwmSInt32 c)
{
	if (a>b)
	{
		if (c>b)
		{
			if (c>a) b = a;
			else    b = c;
		}
	}
	else
	{
		if (b>c)
		{
			if (c>a) b = c;
			else    b = a;
		}
	}
	return b;
}

void SB4RoQEncoder::PutLE32(lwmUInt8 **h, lwmUInt32 n)
{
	lwmUInt8 *ptr;
	ptr = *h;

	ptr[0] = static_cast<lwmUInt8>(n);
	ptr[1] = static_cast<lwmUInt8>((n >> 8) & 0xff);
	ptr[2] = static_cast<lwmUInt8>((n >> 16) & 0xff);
	ptr[3] = static_cast<lwmUInt8>((n >> 24) & 0xff);
	*h = ptr + 4;
}

void SB4RoQEncoder::PutLE16(lwmUInt8 **h, lwmUInt16 n)
{
	lwmUInt8 *ptr;
	ptr = *h;

	ptr[0] = static_cast<lwmUInt8>(n);
	ptr[1] = static_cast<lwmUInt8>((n >> 8) & 0xff);
	*h = ptr + 2;
}

void SB4RoQEncoder::PutByte(lwmUInt8 **h, lwmUInt8 n)
{
	lwmUInt8 *ptr;
	ptr = *h;

	*ptr = static_cast<lwmUInt8>(n);
	*h = ptr + 1;
}


void SB4RoQEncoder::UnpackAllCB2(const lwmUInt8 cb2[1536], SB4Block<2> *cb2unpacked)
{
	SB4Block<2> *block = cb2unpacked;
	const unsigned char *cb2ptr = cb2;
	for (lwmUInt32 i = 0; i<256; i++)
	{
		block->Row(0, 0)[0] = *cb2ptr++;
		block->Row(0, 0)[1] = *cb2ptr++;
		block->Row(0, 1)[0] = *cb2ptr++;
		block->Row(0, 1)[1] = *cb2ptr++;

		block->Row(1, 0)[0] =
			block->Row(1, 0)[1] =
			block->Row(1, 1)[0] =
			block->Row(1, 1)[1] = *cb2ptr++;

		block->Row(2, 0)[0] =
			block->Row(2, 0)[1] =
			block->Row(2, 1)[0] =
			block->Row(2, 1)[1] = *cb2ptr++;

		block++;
	}
}

void SB4RoQEncoder::UnpackSingleCB4(const SB4Block<2> *cb2unpacked, const lwmUInt8 *cb4, SB4Block<4> *block)
{
	const SB4Block<2> *ptr;
	static const lwmUInt32 offsets[4][2] = { { 0, 0 }, { 2, 0 }, { 0, 2 }, { 2, 2 } };

	for (lwmUInt32 p = 0; p<3; p++)
		for (lwmUInt32 i = 0; i<4; i++)
		{
			ptr = cb2unpacked + cb4[i];

			memcpy(block->Row(p, offsets[i][1] + 0) + offsets[i][0], ptr->Row(p, 0), 2);
			memcpy(block->Row(p, offsets[i][1] + 1) + offsets[i][0], ptr->Row(p, 1), 2);
		}
}

void SB4RoQEncoder::UnpackAllCB4(const SB4Block<2> cb2unpacked[256], const lwmUInt8 cb4[1024], SB4Block<4> *cb4unpacked)
{
	for (lwmUInt32 i = 0; i<256; i++)
		UnpackSingleCB4(cb2unpacked, cb4 + i * 4, cb4unpacked + i);
}

void SB4RoQEncoder::UnpackCB2(const int *cb2, SB4Block<2> *block)
{
	block->Row(0, 0)[0] = cb2[0];
	block->Row(0, 0)[1] = cb2[1];
	block->Row(0, 1)[0] = cb2[2];
	block->Row(0, 1)[1] = cb2[3];

	block->Row(1, 0)[0] =
		block->Row(1, 0)[1] =
		block->Row(1, 1)[0] =
		block->Row(1, 1)[1] = (cb2[4] + 2) / 2;

	block->Row(2, 0)[0] =
		block->Row(2, 0)[1] =
		block->Row(2, 1)[0] =
		block->Row(2, 1)[1] = (cb2[5] + 2) / 2;
}

SB4Block<8> SB4RoQEncoder::Enlarge4to8(const SB4Block<4> &b4)
{
	SB4Block<8> b8;

	for (lwmUInt32 p = 0; p<3; p++)
		for (lwmUInt32 y = 0; y<8; y++)
			for (lwmUInt32 x = 0; x<8; x++)
				b8.Row(p, y)[x] = b4.Row(p, y / 2)[x / 2];
	return b8;

}

lwmUInt32 SB4SizeCalc::CalculateSize() const
{
	lwmUInt32 cbSize;

	/** If all CB4 entries are used, all CB2 entries must be used too,
	*  or it'll encode it as zero entries */
	if (this->usedCB4 == 256)
		cbSize = 8 + 256 * 6 + 256 * 4;
	if (this->usedCB2 || this->usedCB4)
		cbSize = 8 + this->usedCB2 * 6 + this->usedCB4 * 4;
	else
		cbSize = 0;

	/* 1 code = 2 bits, round up to 2 byte divisible */
	lwmUInt32 numCodeWordsSpooled = ((this->numCodes + 7) >> 3);

	lwmUInt32 mainBlockSize = 8 + numCodeWordsSpooled * 2 + this->numArguments;

	return cbSize + mainBlockSize;
}

// Returns the size of just the main block with no headers
lwmUInt32 SB4SizeCalc::CalculateSizeMainBlock() const
{
	lwmUInt32 numCodeWordsSpooled;

	numCodeWordsSpooled = ((this->numCodes + 7) >> 3);

	return numCodeWordsSpooled * 2 + this->numArguments;
}
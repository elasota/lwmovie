#ifndef __SB4VIDEOENC_H__
#define __SB4VIDEOENC_H__

#include <vector>

#include "../common//lwmovie_coretypes.h"
#include "SB4Image.hpp"
#include "SB4Random.hpp"

struct SB4RoQTempData;
struct SB4PossibilityList;
struct SB4CelEvaluation;
struct SB4SubCelEvaluation;
struct SB4SortOption;
struct SB4Possibility;
struct SB4SizeCalc;

struct SB4MotionVector
{
	lwmSInt32 dx, dy;
};

enum SB4WriteType
{
	SB4WT_MovieHeader,
	SB4WT_Frame,
	SB4WT_MovieInfo,
};

typedef void(*SB4WriteCallback) (void *h, const void *bytes, lwmLargeUInt numBytes, SB4WriteType writeType);
typedef void(*SB4LogCallback) (void *h, const char *fmt, ...);

class SB4RoQEncoder
{
public:
	SB4RoQEncoder();
	bool Init(lwmUInt32 width, lwmUInt32 height, lwmUInt32 keyrate, lwmUInt32 iFrameSize, lwmUInt32 pFrameSize);
	bool EncodeFrame(const SB4Image *img, lwmUInt32 highThreshold, lwmUInt32 numThresholdPhases, void *streamH, SB4WriteCallback cb, void *logH, SB4LogCallback log);
	void WriteFileHeader(void *h, SB4WriteCallback cb);

private:
	void WriteVideoInfoChunk(void *h, SB4WriteCallback cb);
	bool EncodeVideo(lwmUInt32 highThreshold, lwmUInt32 numThresholdPhases, void *h, SB4WriteCallback cb, void *logH, SB4LogCallback logCB);
	bool CreateCelEvals(SB4RoQTempData &tempData);
	bool CreatePossibilityLists(SB4RoQTempData &tempData);
	void InitializeSinglePossibilityList(SB4PossibilityList *plist, lwmUInt32 fsk);
	bool VQEncode(lwmUInt32 numPhases, SB4RoQTempData &tempData, std::vector<lwmUInt8> &v, void *logH, SB4LogCallback logCB);
	bool GatherCelData(SB4RoQTempData &tempData, bool motionPass);
	void GatherDataForCel(SB4RoQTempData &tempData, SB4CelEvaluation *celEvals, bool motionPass);
	void GatherDataForSubCel(SB4RoQTempData &tempData, SB4SubCelEvaluation *subCelEval, lwmUInt32 x, lwmUInt32 y, bool motionPass);
	bool VQThresholdPass(SB4RoQTempData &tempData, std::vector<lwmUInt8> &v, lwmUInt32 threshold, SB4Image *outImage, lwmUInt64 *outDist);
	bool UnpackCodebooks(SB4RoQTempData &tempData);
	bool GatherPossibilityData(SB4PossibilityList *plists, SB4CelEvaluation *celEvals, lwmUInt32 numBlocks);
	static bool GatherPossibilityDataForBlock(SB4PossibilityList *plist, SB4CelEvaluation *celEval);
	static bool SortPossibilityData(SB4PossibilityList *plist);
	bool ReducePLists(SB4RoQTempData &tempData, SB4PossibilityList *plists, SB4CelEvaluation *celEvals, lwmUInt32 numBlocks,
		lwmUInt32 sizeLowerLimit, lwmUInt32 sizeLimit, lwmUInt64 *outDist);
	static void UpdateSortOption(SB4SortOption *sortOption);
	static void ModifySizeCalc(SB4Possibility *p, SB4CelEvaluation *eval, SB4SizeCalc *sizeCalc, int mod);
	bool ReconstructAndEncodeImage(SB4RoQTempData &tempData, lwmUInt32 w, lwmUInt32 h, lwmUInt32 numBlocks, void *handle, SB4Image *outImage, SB4WriteCallback cb);

	template<class T>
	static void SimpleSwap(T &a, T &b);

	template <lwmUInt32 TD>
	void MotionSearch(bool fullSearch);

	template <lwmUInt32 TD>
	lwmUInt32 EvalMotionDist(lwmUInt32 x, lwmUInt32 y, lwmSInt32 mx, lwmSInt32 my);

	template <lwmUInt32 TD>
	static lwmUInt32 BlockSSE(const SB4Image *img1, const SB4Image *img2, lwmUInt32 x1, lwmUInt32 y1, lwmUInt32 x2, lwmUInt32 y2);

	lwmSInt32 MidPred(lwmSInt32 a, lwmSInt32 b, lwmSInt32 c);

	static void PutLE32(lwmUInt8 **h, lwmUInt32 n);
	static void PutLE16(lwmUInt8 **h, lwmUInt16 n);
	static void PutByte(lwmUInt8 **h, lwmUInt8 n);

	static void UnpackAllCB2(const lwmUInt8 cb2[1536], SB4Block<2> *cb2unpacked);
	static void UnpackSingleCB4(const SB4Block<2> *cb2unpacked, const lwmUInt8 *cb4, SB4Block<4> *block);
	static void UnpackAllCB4(const SB4Block<2> cb2unpacked[256], const lwmUInt8 cb4[1024], SB4Block<4> *cb4unpacked);
	static void UnpackCB2(const int *cb2, SB4Block<2> *block);
	static SB4Block<8> Enlarge4to8(const SB4Block<4> &b4);

	void MakeCodebooks(lwmUInt8 *cb2, lwmUInt8 *cb4, SB4RandomState *rnd, const SB4Image *prevImg, const SB4Image *img, const lwmUInt32 *subcelPredDists, lwmUInt32 threshold);


	SB4Image m_frames[2];

	SB4Image *m_lastFrame;
	SB4Image *m_currentFrame;
	const SB4Image *m_frameToEnc;
	SB4Image *m_outputFrame;

	std::vector<SB4MotionVector> m_motion8[2];
	std::vector<SB4MotionVector> m_motion4[2];

	SB4MotionVector *m_thisMotion4;
	SB4MotionVector *m_lastMotion4;

	SB4MotionVector *m_thisMotion8;
	SB4MotionVector *m_lastMotion8;

	lwmUInt32 m_width, m_height;

	lwmUInt32 m_minBytes;
	lwmUInt32 m_maxBytes;

	lwmUInt32 m_framesSinceKeyframe;
	lwmUInt32 m_framesToKeyframe;
	lwmUInt32 m_keyrate;
	bool m_firstFrame;

	lwmUInt32 m_iframeSize;
	lwmUInt32 m_pframeSize;

	lwmUInt8 m_cb2[1536];
	lwmUInt8 m_cb4[1024];

	lwmUInt32 m_finalSize;

	lwmSInt32 m_slip;
	lwmUInt64 m_dist;
	lwmUInt64 m_targetDist;

	lwmSInt32 m_cbOmitThreshold;

	SB4RandomState m_rng;
};

#endif

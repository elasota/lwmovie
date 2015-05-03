#include "SB4Encoder.hpp"

SB4RoQEncoder::SB4RoQEncoder()
	: m_lastFrame(NULL)
	, m_currentFrame(NULL)
	, m_frameToEnc(NULL)
	, m_outputFrame(NULL)
	, m_thisMotion4(NULL)
	, m_lastMotion4(NULL)
	, m_thisMotion8(NULL)
	, m_lastMotion8(NULL)
	, m_width(0)
	, m_height(0)
	, m_minBytes(0)
	, m_maxBytes(0)
	, m_framesSinceKeyframe(0)
	, m_framesToKeyframe(0)
	, m_keyrate(0)
	, m_firstFrame(false)
	, m_iframeSize(0)
	, m_pframeSize(0)
	, m_finalSize(0)
	, m_slip(0)
	, m_dist(0)
	, m_targetDist(0)
	, m_cbOmitThreshold(0)
{
	memset(m_cb2, 0, sizeof(m_cb2));
	memset(m_cb4, 0, sizeof(m_cb4));
}

bool SB4RoQEncoder::Init(lwmUInt32 width, lwmUInt32 height, lwmUInt32 keyrate, lwmUInt32 irate, lwmUInt32 prate)
{
	SB4MotionVector blankVect;

	/* Clear out the context */
	m_rng.Init(1);

	m_framesToKeyframe = m_keyrate = keyrate;
	m_iframeSize = irate;
	m_pframeSize = prate;

	m_slip = 0;

	if ((width & 0xf) || (height & 0xf))
		return false;

	m_width = width;
	m_height = height;

	m_framesSinceKeyframe = 0;
	m_firstFrame = true;

	m_lastFrame = &m_frames[0];
	m_currentFrame = &m_frames[1];

	blankVect.dx = blankVect.dy = 0;

	m_motion4[0].resize(m_width*m_height / 16, blankVect);
	m_motion4[1].resize(m_width*m_height / 16, blankVect);
	m_motion8[0].resize(m_width*m_height / 64, blankVect);
	m_motion8[1].resize(m_width*m_height / 64, blankVect);

	m_thisMotion4 = &m_motion4[0][0];
	m_lastMotion4 = &m_motion4[1][0];
	m_thisMotion8 = &m_motion8[0][0];
	m_lastMotion8 = &m_motion8[1][0];

	m_frames[0].Init(width, height);
	m_frames[1].Init(width, height);

	return 0;
}

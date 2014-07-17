#ifndef __LWMOVIE_VIDEOTYPES_HPP__
#define __LWMOVIE_VIDEOTYPES_HPP__

#include "lwmovie_types.hpp"
#include "lwmovie_external_types.h"
#include "lwmovie_bitstream.hpp"
#include "lwmovie_constants.hpp"


namespace lwmovie
{
	struct lwmIM1VReconstructor;

	struct lwmDCTBLOCK
	{
		lwmSInt16 data[64];

		void FastZeroFill();
	};

	struct mpegGoP
	{
		bool drop_flag;			/* Flag indicating dropped frame. */
		lwmUInt32 tc_hours;		/* Hour component of time code.   */
		lwmUInt32 tc_minutes;	/* Minute component of time code. */
		lwmUInt32 tc_seconds;	/* Second component of time code. */
		lwmUInt32 tc_pictures;	/* Picture counter of time code.  */
		bool closed_gop;		/* Indicates no pred. vectors to
								previous group of pictures.    */
		bool broken_link;		/* B frame unable to be decoded.  */
	};

	struct mpegSequence
	{
		lwmUInt8 m_intra_quant_matrix[64];		/* Quantization matrix for
												intracoded frames.      */
		lwmUInt8 m_non_intra_quant_matrix[64];	/* Quanitization matrix for 
												non intracoded frames.  */
	};

	struct mpegPict
	{
		lwmUInt32 temp_ref;				/* Temporal reference.             */
		lwmUInt32 code_type;			/* Frame type: P, B, I             */
		lwmUInt32 vbv_delay;			/* Buffer delay.                   */
		bool full_pel_forw_vector;		/* Forw. vectors specified in full pixel values flag.              */
		lwmUInt32 forw_r_size;			/* Used for vector decoding.       */
		lwmUInt8 forw_f;				/* Used for vector decoding.       */
		bool full_pel_back_vector;		/* Back vectors specified in full pixel values flag.              */
		lwmUInt32 back_r_size;			/* Used in decoding.               */
		lwmUInt8 back_f;				/* Used in decoding.               */
	};

	struct mpegSlice
	{
		lwmUInt32 vert_pos;				/* Vertical position of slice. */
		lwmUInt8 quant_scale;			/* Quantization scale.         */
	};

	struct mpegBlock
	{
		lwmDCTBLOCK dct_recon;
		lwmSInt16 dct_dc_y_past;	/* Past lum. dc dct coefficient.   */
		lwmSInt16 dct_dc_cr_past;	/* Past cr dc dct coefficient.     */
		lwmSInt16 dct_dc_cb_past;	/* Past cb dc dct coefficient.     */
	};

	struct mpegMacroblock
	{
		lwmSInt32 mb_address;				/* Macroblock address.              */
		lwmSInt32 past_mb_addr;				/* Previous mblock address.         */
		lwmSInt32 motion_h_forw_code;		/* Forw. horiz. motion vector code. */
		lwmUInt8 motion_h_forw_r;			/* Used in decoding vectors.        */
		lwmSInt32 motion_v_forw_code;		/* Forw. vert. motion vector code.  */
		lwmUInt8 motion_v_forw_r;			/* Used in decdoinge vectors.       */
		lwmSInt32 motion_h_back_code;		/* Back horiz. motion vector code.  */
		lwmUInt8 motion_h_back_r;			/* Used in decoding vectors.        */
		lwmSInt32 motion_v_back_code;		/* Back vert. motion vector code.   */
		lwmUInt8 motion_v_back_r;			/* Used in decoding vectors.        */
		lwmUInt32 cbp;						/* Coded block pattern.             */
		bool mb_intra;						/* Intracoded mblock flag.          */
		bool bpict_past_forw;				/* Past B frame forw. vector flag.  */
		bool bpict_past_back;				/* Past B frame back vector flag.   */
		lwmSInt32 past_intra_addr;			/* Addr of last intracoded mblock.  */
		lwmSInt32 recon_right_for_prev;		/* Past right forw. vector.         */
		lwmSInt32 recon_down_for_prev;		/* Past down forw. vector.          */
		lwmSInt32 recon_right_back_prev;	/* Past right back vector.          */
		lwmSInt32 recon_down_back_prev;		/* Past down back vector.           */
	};

	class lwmPictImage
	{
		lwmUInt8 *luminance;	/* Luminance plane.   */
		lwmUInt8 *Cr;			/* Cr plane.          */
		lwmUInt8 *Cb;			/* Cb plane.          */

		lwmSAllocator alloc;

		bool createdOk;
		bool m_isReserved;

	public:
		lwmPictImage();
		lwmPictImage(const lwmSAllocator *alloc, lwmLargeUInt w, lwmLargeUInt h);
		~lwmPictImage();

		lwmUInt8 *GetLuminancePlane() const;
		lwmUInt8 *GetCrPlane() const;
		lwmUInt8 *GetCbPlane() const;

		bool IsCreated() const;
		bool IsReserved() const;
		void Reserve();
		void Unreserve();
	};

	struct lwmReconMBlock
	{
		bool skipped;
		bool mb_motion_forw;
		bool mb_motion_back;

		lwmSInt32 recon_right_for;
		lwmSInt32 recon_down_for;
		lwmSInt32 recon_right_back;
		lwmSInt32 recon_down_back;

		inline void SetReconInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back, lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, lwmSInt32 recon_right_back, lwmSInt32 recon_down_back)
		{
			this->skipped = skipped;
			this->mb_motion_forw = mb_motion_forw;
			this->mb_motion_back = mb_motion_back;
			this->recon_right_for = recon_right_for;
			this->recon_down_for = recon_down_for;
			this->recon_right_back = recon_right_back;
			this->recon_down_back = recon_down_back;
		}
	};

	struct lwmMBlockInfo
	{
		bool skipped;
		bool mb_intra;

		lwmSInt32 recon_right_for_prev;		/* Past right forw. vector.         */
		lwmSInt32 recon_down_for_prev;		/* Past down forw. vector.          */
		lwmSInt32 recon_right_back_prev;	/* Past right back vector.          */
		lwmSInt32 recon_down_back_prev;		/* Past down back vector.           */

		/*
		lwmSInt32 recon_right_for;
		lwmSInt32 recon_down_for;
		lwmSInt32 recon_right_back;
		lwmSInt32 recon_down_back;
		*/


		bool bpict_past_forw;				/* Past B frame forw. vector flag.  */
		bool bpict_past_back;				/* Past B frame back vector flag.   */

		bool mb_motion_forw;
		bool mb_motion_back;

		inline lwmMBlockInfo();

		inline lwmMBlockInfo(mpegMacroblock *mblock, bool p)
		{
			mb_intra = mblock->mb_intra;

			recon_right_for_prev = mblock->recon_right_for_prev;
			recon_down_for_prev = mblock->recon_down_for_prev;
			recon_right_back_prev = mblock->recon_right_back_prev;
			recon_down_back_prev = mblock->recon_down_back_prev;

			bpict_past_back = mblock->bpict_past_back;
			bpict_past_forw = mblock->bpict_past_forw;

			skipped = p;
			mb_motion_forw = mb_motion_back = false;
		}
	};

	struct lwmBlockInfo
	{
		bool needs_idct;
		bool sparse_idct;
		lwmUInt8 sparse_idct_index;
		lwmSInt16 sparse_idct_coef;

		bool zero_block_flag;

		inline void SetReconInfo(bool zero_block_flag)
		{
			this->zero_block_flag = zero_block_flag;
		}
	};

	enum lwmEReconSlot
	{
		lwmRECONSLOT_Unassigned,
		lwmRECONSLOT_FirstListed,

		lwmRECONSLOT_IP1 = lwmRECONSLOT_FirstListed,
		lwmRECONSLOT_IP2,
		lwmRECONSLOT_B,
	};

	class lwmDeslicerJob
	{
	public:
		lwmDeslicerJob(lwmUInt32 mbWidth, lwmUInt32 mbHeight);
		bool Digest(const mpegSequence *sequenceData, const mpegPict *pictData, const void *sliceData, lwmUInt32 sliceSize, lwmIM1VReconstructor *recon);

	private:
		bool ParseSliceHeader(lwmCBitstream *bitstream);
		lwmovie::constants::lwmEParseState ParseMacroBlock(lwmCBitstream *bitstream, lwmSInt32 max_mb_addr, lwmIM1VReconstructor *recon);
		void ComputeForwVector(lwmSInt32 *recon_right_for_ptr, lwmSInt32 *recon_down_for_ptr);
		void ComputeBackVector(lwmSInt32 *recon_right_back_ptr, lwmSInt32 *recon_down_back_ptr);
		bool ParseReconBlock(lwmCBitstream *bitstream, lwmSInt32 n, lwmIM1VReconstructor *recon);
		void DecodeDCTCoeffFirst(lwmCBitstream *bitstream, lwmUInt8 *outRun, lwmSInt16 *outLevel);
		void DecodeDCTCoeffNext(lwmCBitstream *bitstream, lwmUInt8 *outRun, lwmSInt16 *outLevel);
		void DecodeDCTCoeff(lwmCBitstream *bitstream, const lwmUInt16 *dct_coeff_tbl, lwmUInt8 *outRun, lwmSInt16 *outLevel);
		lwmUInt8 DecodeDCTDCSizeLum(lwmCBitstream *bitstream);
		lwmUInt8 DecodeDCTDCSizeChrom(lwmCBitstream *bitstream);
		lwmUInt8 DecodeMBAddrInc(lwmCBitstream *bitstream);
		void DecodeMBTypeI(lwmCBitstream *bitstream, lwmovie::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra);
		void DecodeMBTypeP(lwmCBitstream *bitstream, lwmovie::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra);
		void DecodeMBTypeB(lwmCBitstream *bitstream, lwmovie::constants::lwmEMBQuantType *mb_quant, bool *mb_motion_forw, bool *mb_motion_back, bool *mb_pattern, bool *mb_intra);
		lwmSInt32 DecodeMotionVectors(lwmCBitstream *bitstream);
		lwmUInt8 DecodeCBP(lwmCBitstream *bitstream);

		//lwmCBitstream		m_bitstream;
		lwmUInt32			m_mb_width;
		lwmUInt32			m_mb_height;
		mpegSlice			m_slice;
		mpegMacroblock		m_mblock;
		mpegBlock			m_block;
		const mpegPict		*m_picture;
		const mpegSequence	*m_sequence;
	};

	class lwmVidStream
	{
	public:
		void SignalIOFailure();
		void ReportError(const char *str);
		
		lwmVidStream(const lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, bool useThreadedDeslicer);

		bool DigestStreamParameters(const void *bytes, lwmUInt32 packetSize, lwmIM1VReconstructor *recon);
		bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, lwmIM1VReconstructor *recon, lwmUInt32 *outResult);

		bool CreatedOK() const;
		
		static void SkipExtraBitInfo(lwmCBitstream *m_bitstream);

	private:
		lwmUInt32 m_h_size;						/* Horiz. size in pixels.     */
		lwmUInt32 m_v_size;						/* Vert. size in pixels.      */
		lwmUInt8 m_aspect_ratio_code;			/* Code for aspect ratio.     */
		lwmUInt8 m_picture_rate_code;			/* Code for picture rate.     */
		lwmUInt32 m_bit_rate;					/* Bit rate.                  */
		lwmUInt32 m_vbv_buffer_size;			/* Minimum buffer size.       */
		bool m_const_param_flag;				/* Contrained parameter flag. */
		mpegPict m_picture;						/* Current picture.           */
		mpegSequence m_sequence;				/* Current sequence.          */

		lwmSInt32 m_right_for, m_down_for;		/* From ReconPMBlock, video.c */
		lwmSInt32 m_right_half_for, m_down_half_for;

		lwmEReconSlot		m_past;				/* Past predictive frame.         */
		lwmEReconSlot		m_future;			/* Future predictive frame.       */
		lwmEReconSlot		m_current;			/* Current frame.                 */

		lwmUInt32			m_mb_height;		/* Vert. size in mblocks.     */
		lwmUInt32			m_mb_width;			/* Horiz. size in mblocks.    */

		lwmSAllocator		m_alloc;
		lwmDeslicerJob		m_stDeslicerJob;

		bool m_createdOk;
		bool m_eof;
		bool m_useThreadedDeslicer;

	private:

		void FinishPicture();
		void OutputFinishedFrame();

		lwmovie::constants::lwmEParseState ParseSeqHead_MPEG(lwmCBitstream *bitstream);
		lwmovie::constants::lwmEParseState ParsePicture(lwmCBitstream *bitstream);
		bool ParseReconBlock(lwmSInt32 n, lwmIM1VReconstructor *recon);
		lwmovie::constants::lwmEParseState ParseMacroBlock(lwmSInt32 max_mb_addr, lwmIM1VReconstructor *recon);

		bool SetupPictImages(lwmUInt32 w, lwmUInt32 h);
		void ComputeForwVector(lwmSInt32 *recon_right_for, lwmSInt32 *recon_down_for);
		void ComputeBackVector(lwmSInt32 *recon_right_for, lwmSInt32 *recon_down_for);

		void ReconIMBlock( int bnum );
		void ReconPMBlock(int bnum, lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, bool zflag);
	};
}

#endif

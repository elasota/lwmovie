/********************************************************************
*                                                                  *
* THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
* USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
* GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
* IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
*                                                                  *
* THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
* by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
*                                                                  *
********************************************************************

function:
last mod: $Id$

********************************************************************/
/* lwmovie modifications (c)2017 Eric Lasota */

#include "../config.h"

#ifdef OC_X86_64_ASM

#include "x86int.h"

#include <emmintrin.h>
#include <string.h>

#ifdef _MSC_VER
#pragma intrinsic(memcpy)
#endif

void oc_frag_copy_sse2(unsigned char *_dst,const unsigned char *_src,int _ystride){
  int i;
  for(i=8;i-->0;){
    _mm_storel_epi64((__m128i*)_dst, _mm_loadl_epi64((const __m128i*)_src));
    _dst+=_ystride;
    _src+=_ystride;
  }
}



/*Copies the fragments specified by the lists of fragment indices from one
   frame to another.
  _dst_frame:     The reference frame to copy to.
  _src_frame:     The reference frame to copy from.
  _ystride:       The row stride of the reference frames.
  _fragis:        A pointer to a list of fragment indices.
  _nfragis:       The number of fragment indices to copy.
  _frag_buf_offs: The offsets of fragments in the reference frames.*/
void oc_frag_copy_list_sse2(unsigned char *_dst_frame,
 const unsigned char *_src_frame,int _ystride,
 const ptrdiff_t *_fragis,ptrdiff_t _nfragis,const ptrdiff_t *_frag_buf_offs){
  ptrdiff_t fragii;
  for(fragii=0;fragii<_nfragis;fragii++){
    ptrdiff_t frag_buf_off;
    frag_buf_off=_frag_buf_offs[_fragis[fragii]];
	oc_frag_copy_sse2(_dst_frame+frag_buf_off,
     _src_frame+frag_buf_off,_ystride);
  }
}

void oc_frag_recon_intra_sse2(unsigned char *_dst,int _ystride,
 const ogg_int16_t _residue[64]){
  int i;
  __m128i offset, residue, total, result;
  offset = _mm_set1_epi16(128);
  for(i=0;i<8;i++){
	residue = _mm_load_si128((const __m128i*)(_residue + i*8));
	total = _mm_add_epi16(offset, residue);
	result = _mm_packus_epi16(total, _mm_setzero_si128());
	_mm_storel_epi64((__m128i*)(_dst), result);
    _dst+=_ystride;
  }
}

void oc_frag_recon_inter_sse2(unsigned char *_dst,
 const unsigned char *_src,int _ystride,const ogg_int16_t _residue[64]){
  int i;
  __m128i srcrow, residue, total, result;
  for(i=0;i<8;i++){
    srcrow = _mm_loadl_epi64((const __m128i*)(_src));
	residue = _mm_load_si128((const __m128i*)(_residue + i*8));

	srcrow = _mm_unpacklo_epi8(srcrow, _mm_setzero_si128());
	total = _mm_add_epi16(srcrow, residue);
	result = _mm_packus_epi16(total, _mm_setzero_si128());
	_mm_storel_epi64((__m128i*)(_dst), result);
    _dst+=_ystride;
    _src+=_ystride;
  }
}

void oc_frag_recon_inter2_sse2(unsigned char *_dst,const unsigned char *_src1,
 const unsigned char *_src2,int _ystride,const ogg_int16_t _residue[64]){
  int i;
  __m128i src1row, src2row, avgsrc, residue, total, result;
  for(i=0;i<8;i++){
    src1row = _mm_loadl_epi64((const __m128i*)(_src1));
	src2row = _mm_loadl_epi64((const __m128i*)(_src2));
	residue = _mm_load_si128((const __m128i*)(_residue + i*8));

	src1row = _mm_unpacklo_epi8(src1row, _mm_setzero_si128());
	src2row = _mm_unpacklo_epi8(src2row, _mm_setzero_si128());
	avgsrc = _mm_srli_epi16(_mm_add_epi16(src1row, src2row), 1);
	total = _mm_add_epi16(avgsrc, residue);
	result = _mm_packus_epi16(total, _mm_setzero_si128());
	_mm_storel_epi64((__m128i*)(_dst), result);
    _dst+=_ystride;
    _src1+=_ystride;
    _src2+=_ystride;
  }
}

void oc_restore_fpu_sse2() {
}


void oc_state_frag_recon_sse2(const oc_theora_state *_state, ptrdiff_t _fragi,
	int _pli, ogg_int16_t _dct_coeffs[128], int _last_zzi, ogg_uint16_t _dc_quant)
{
	oc_state_frag_recon_c(_state, _fragi, _pli, _dct_coeffs, _last_zzi, _dc_quant);
}

#define LOOP_FILTER_LOCALS __m128i packed2sub1, packed2sub1mul3, f, clow, chigh

#define LOOP_FILTER_PACKED_SSE2(_packed0, _packed1, _packed2, _packed3, _2flimit, _out1, _out2)\
  packed2sub1 = _mm_sub_epi16(_packed2, _packed1);\
  packed2sub1mul3 = _mm_add_epi16(packed2sub1, _mm_slli_epi16(packed2sub1, 1));\
  f = _mm_add_epi16(_mm_sub_epi16(_packed0, _packed3), packed2sub1mul3);\
  f = _mm_add_epi16(f, _mm_set1_epi16(4));\
  f = _mm_srai_epi16(f, 3);\
  /* f=min(max(min(-_2flimit-f,0),f),max(_2flimit-f,0));*/\
  clow = _mm_min_epi16(_mm_sub_epi16(_mm_sub_epi16(_mm_setzero_si128(), _2flimit), f), _mm_setzero_si128());\
  chigh = _mm_max_epi16(_mm_sub_epi16(_2flimit, f), _mm_setzero_si128());\
  f = _mm_min_epi16(_mm_max_epi16(f, clow), chigh);\
  _out1 = _mm_add_epi16(_packed1, f);\
  _out2 = _mm_sub_epi16(_packed2, f)

static void loop_filter_h_sse2(unsigned char *_pix,int _ystride,const __m128i *_p2flimit){
  __m128i packed[2];
  __m128i packed01, packed23;
  __m128i packed0, packed1, packed2, packed3;
  __m128i filtered1, filtered2;
  __m128i finalInterleaved;
  __m128i flimit2;
  LOOP_FILTER_LOCALS;
  int y;
  flimit2 = *_p2flimit;
  _pix-=2;
  for(y=0;y<8;y++)
    memcpy(((char*)packed) + y * 4, _pix + _ystride*y, 4);

  // 0123 0123 0123 0123 | 0123 0123 0123 0123
  packed01 = _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(packed[0], 16), 16), _mm_srai_epi32(_mm_slli_epi32(packed[1], 16), 16));
  packed23 = _mm_packs_epi32(_mm_srai_epi32(packed[0], 16), _mm_srai_epi32(packed[1], 16));

  packed0 = _mm_srli_epi16(_mm_slli_epi16(packed01, 8), 8);
  packed1 = _mm_srli_epi16(packed01, 8);
  packed2 = _mm_srli_epi16(_mm_slli_epi16(packed23, 8), 8);
  packed3 = _mm_srli_epi16(packed23, 8);

  LOOP_FILTER_PACKED_SSE2(packed0, packed1, packed2, packed3, flimit2, filtered1, filtered2);

  filtered1 = _mm_packus_epi16(filtered1, _mm_setzero_si128());
  filtered2 = _mm_packus_epi16(filtered2, _mm_setzero_si128());

  finalInterleaved = _mm_unpacklo_epi8(filtered1, filtered2);
  
  for(y=0;y<8;y++)
    memcpy(_pix + _ystride*y + 1, ((char*)&finalInterleaved) + y*2, 2);
}

static void loop_filter_v_sse2(unsigned char *_pix,int _ystride,const __m128i *_p2flimit){
  __m128i packed0, packed1, packed2, packed3, filtered1, filtered2, filteredMerged;
  __m128i flimit2;
  LOOP_FILTER_LOCALS;
  _pix-=_ystride*2;
  flimit2 = *_p2flimit;

  packed0 = _mm_loadl_epi64((const __m128i*)(_pix));
  packed1 = _mm_loadl_epi64((const __m128i*)(_pix + _ystride*1));
  packed2 = _mm_loadl_epi64((const __m128i*)(_pix + _ystride*2));
  packed3 = _mm_loadl_epi64((const __m128i*)(_pix + _ystride*3));

  packed0 = _mm_unpacklo_epi8(packed0, _mm_setzero_si128());
  packed1 = _mm_unpacklo_epi8(packed1, _mm_setzero_si128());
  packed2 = _mm_unpacklo_epi8(packed2, _mm_setzero_si128());
  packed3 = _mm_unpacklo_epi8(packed3, _mm_setzero_si128());

  LOOP_FILTER_PACKED_SSE2(packed0, packed1, packed2, packed3, flimit2, filtered1, filtered2);

  filteredMerged = _mm_packus_epi16(filtered1, filtered2);
  _mm_storel_epi64((__m128i*)(_pix + _ystride), filteredMerged);
  _mm_storel_epi64((__m128i*)(_pix + _ystride*2), _mm_srli_si128(filteredMerged, 8));
}

/*Initialize the bounding values array used by the loop filter.
  _bv: Storage for the array.
  _flimit: The filter limit as defined in Section 7.10 of the spec.*/
void oc_loop_filter_init_sse2(signed char _bv[256],int _flimit){
  __m128i flimit2 = _mm_set1_epi16((short)_flimit);
  flimit2 = _mm_add_epi16(flimit2, flimit2);
  _mm_store_si128((__m128i*)_bv, flimit2);
}

/*Apply the loop filter to a given set of fragment rows in the given plane.
  The filter may be run on the bottom edge, affecting pixels in the next row of
   fragments, so this row also needs to be available.
  _bv:        The bounding values array.
  _refi:      The index of the frame buffer to filter.
  _pli:       The color plane to filter.
  _fragy0:    The Y coordinate of the first fragment row to filter.
  _fragy_end: The Y coordinate of the fragment row to stop filtering at.*/
void oc_state_loop_filter_frag_rows_sse2(const oc_theora_state *_state,
 signed char *_bv,int _refi,int _pli,int _fragy0,int _fragy_end){
  const oc_fragment_plane *fplane;
  const oc_fragment       *frags;
  const ptrdiff_t         *frag_buf_offs;
  unsigned char           *ref_frame_data;
  ptrdiff_t                fragi_top;
  ptrdiff_t                fragi_bot;
  ptrdiff_t                fragi0;
  ptrdiff_t                fragi0_end;
  __m128i                  flimit2;
  int                      ystride;
  int                      nhfrags;
  flimit2 = _mm_load_si128((const __m128i*)_bv);
  fplane=_state->fplanes+_pli;
  nhfrags=fplane->nhfrags;
  fragi_top=fplane->froffset;
  fragi_bot=fragi_top+fplane->nfrags;
  fragi0=fragi_top+_fragy0*(ptrdiff_t)nhfrags;
  fragi0_end=fragi_top+_fragy_end*(ptrdiff_t)nhfrags;
  ystride=_state->ref_ystride[_pli];
  frags=_state->frags;
  frag_buf_offs=_state->frag_buf_offs;
  ref_frame_data=_state->ref_frame_data[_refi];
  /*The following loops are constructed somewhat non-intuitively on purpose.
    The main idea is: if a block boundary has at least one coded fragment on
     it, the filter is applied to it.
    However, the order that the filters are applied in matters, and VP3 chose
     the somewhat strange ordering used below.*/
  while(fragi0<fragi0_end){
    ptrdiff_t fragi;
    ptrdiff_t fragi_end;
    fragi=fragi0;
    fragi_end=fragi+nhfrags;
    while(fragi<fragi_end){
      if(frags[fragi].coded){
        unsigned char *ref;
        ref=ref_frame_data+frag_buf_offs[fragi];
        if(fragi>fragi0)loop_filter_h_sse2(ref,ystride,&flimit2);
        if(fragi0>fragi_top)loop_filter_v_sse2(ref,ystride,&flimit2);
        if(fragi+1<fragi_end&&!frags[fragi+1].coded){
          loop_filter_h_sse2(ref+8,ystride,&flimit2);
        }
        if(fragi+nhfrags<fragi_bot&&!frags[fragi+nhfrags].coded){
          loop_filter_v_sse2(ref+(ystride<<3),ystride,&flimit2);
        }
      }
      fragi++;
    }
    fragi0+=nhfrags;
  }
}


#endif

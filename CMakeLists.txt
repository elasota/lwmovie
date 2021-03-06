cmake_minimum_required (VERSION 2.8.5)
project (lwmovie)
include (GenerateExportHeader)

option(NOTHEORA "Disable theora" OFF)

add_library(lwcelt STATIC
	lwcelt/celt/bands.c
	lwcelt/celt/celt.c
	lwcelt/celt/celt_encoder.c
	lwcelt/celt/celt_decoder.c
	lwcelt/celt/celt_lpc.c
	lwcelt/celt/cwrs.c
	lwcelt/celt/entcode.c
	lwcelt/celt/entdec.c
	lwcelt/celt/entenc.c
	lwcelt/celt/kiss_fft.c
	lwcelt/celt/laplace.c
	lwcelt/celt/mathops.c
	lwcelt/celt/mdct.c
	lwcelt/celt/modes.c
	lwcelt/celt/pitch.c
	lwcelt/celt/quant_bands.c
	lwcelt/celt/rate.c
	lwcelt/celt/vq.c
)

add_library(mp2dec2 STATIC
	mp2dec2/lwmovie_layer2_csf.cpp
	mp2dec2/lwmovie_layer2_decode.cpp
	mp2dec2/lwmovie_layer2_imdct_32.cpp
	mp2dec2/lwmovie_layer2_sbs.cpp
	mp2dec2/lwmovie_layer2_tables.cpp
)

add_library(theoracommon STATIC
	theora/lib/bitpack.c
	theora/lib/dequant.c
	theora/lib/fragment.c
	theora/lib/idct_b2.c
	theora/lib/info.c
	theora/lib/internal.c
	theora/lib/quant.c
	theora/lib/state.c
	theora/lib/x86intr/sse2frag.c
	theora/lib/x86intr/sse2idct.c
	theora/lib/x86intr/x86cpu.c
	theora/lib/x86intr/x86state.c
)

add_library(theoradec STATIC
	theora/lib/decinfo.c
	theora/lib/decode.c
	theora/lib/huffdec.c
)

add_library(ogg STATIC
	libogg-1.3.1/src/alloc.c
	libogg-1.3.1/src/bitwise.c
	libogg-1.3.1/src/framing.c
)

add_library(lwmovie MODULE
	lwmovie/idct/fastjrevdct.cpp
	lwmovie/lwmovie_adpcm.cpp
	lwmovie/lwmovie_adpcm_decoder.cpp
	lwmovie/lwmovie_audiobuffer.cpp
	lwmovie/lwmovie_audiocodec.cpp
	lwmovie/lwmovie_bitstream.cpp
	lwmovie/lwmovie_celt_decoder.cpp
	lwmovie/lwmovie_constants.cpp
	lwmovie/lwmovie_decodedct.cpp
	lwmovie/lwmovie_demux.cpp
	lwmovie/lwmovie_fp_systemmem.cpp
	lwmovie/lwmovie_idct.cpp
	lwmovie/lwmovie_init.cpp
	lwmovie/lwmovie_motionvectors.cpp
	lwmovie/lwmovie_m1v_idct.cpp
	lwmovie/lwmovie_mp2_decoder.cpp
	lwmovie/lwmovie_parseblock.cpp
	lwmovie/lwmovie_profile.cpp
	lwmovie/lwmovie_recon.cpp
	lwmovie/lwmovie_recon_m1vsw.cpp
	lwmovie/lwmovie_recon_roqsw.cpp
	lwmovie/lwmovie_roq.cpp
	lwmovie/lwmovie_theoradec.cpp
	lwmovie/lwmovie_vlc.cpp
	lwmovie/lwmovie_vlc_dct.cpp
	lwmovie/lwmovie_vlctables.cpp
	lwmovie/mpeg_deslicer.cpp
	lwmovie/mpeg_vidstream.cpp
)

add_executable(lwmux
	lwmovie/lwmovie_adpcm.cpp
	lwmux/lwmux.cpp
	lwmux/lwmux_adpcmenc.cpp
	lwmux/lwmux_celtenc.cpp
	lwmux/lwmux_convertm1v.cpp
	lwmux/lwmux_convertmp2.cpp
	lwmux/lwmux_escape.cpp
	lwmux/lwmux_mux.cpp
	lwmux/lwmux_osfile_x.cpp
	lwmux/lwmux_riff.cpp
	lwmux/lwmux_util.cpp
	lwmux/lwmux_wav.cpp
)

add_executable(lwroqenc
	lwmux/lwmux_osfile_x.cpp
	lwroqenc/lwroqenc.cpp
	lwroqenc/SB4ELBG2.cpp
	lwroqenc/SB4Encoder.cpp
	lwroqenc/SB4Encoder_EncodeFrame.cpp
	lwroqenc/SB4Encoder_MakeCodebooks.cpp
	lwroqenc/SB4Random.cpp
)

add_executable(lwrerange
	lwrerange/lwrerange.cpp
)

target_include_directories(theoracommon PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/theora/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libogg-1.3.1/include>
)

target_include_directories(theoradec PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/theora/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libogg-1.3.1/include>
)

target_include_directories(ogg PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libogg-1.3.1/include>
)

target_include_directories(lwmovie PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/theora/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libogg-1.3.1/include>
)

set_target_properties(lwmovie PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(mp2dec2 PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(lwcelt PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)



set(CMAKE_BUILD_TYPE Release)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_MODULE_LINKER_FLAGS "-s")
    set(CMAKE_EXE_LINKER_FLAGS "-s")
endif()

target_compile_definitions(lwcelt PUBLIC HAVE_CONFIG_H)

target_link_libraries(lwmovie lwcelt mp2dec2)
target_link_libraries(lwmux lwcelt)

if(NOT NOTHEORA)
  target_link_libraries(lwmovie theoradec theoracommon ogg)
  target_compile_definitions(lwmovie PRIVATE LWMOVIE_NO_THEORA)
endif()

set_target_properties(mp2dec2 PROPERTIES COMPILE_FLAGS "-msse4.1")
	
install (TARGETS lwmovie lwmux lwroqenc lwrerange DESTINATION bin)

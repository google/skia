/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_DEVICE_CL_12_H
#define SKC_ONCE_DEVICE_CL_12_H

//
// FIXME -- THERE ARE SOME DUPLICATED TYPEDEFS IN THIS FILE
//
// THESE WILL GO AWAY AS THE TYPING GET POLISHED AND SIMPLIFIED
//

#include "block.h"

//
// HOW TO SELECT A SUBBLOCK AND BLOCK SIZES:
//
// 1) The subblock size should match the natural SIMT/SIMD width of
//    the target device.
//
// 2) Either a square or rectangular (1:2) tile size is chosen.  The
//    tile size is usually determined by the amount of SMEM available
//    to a render kernel subgroup and desired multiprocessor
//    occupancy.
//
// 3) If the tile is rectangular then the block size must be at least
//    twice the size of the subblock size.
//
// 4) A large block size can decrease allocation overhead but there
//    will be diminishing returns as the block size increases.
//

#define SKC_DEVICE_BLOCK_WORDS_LOG2             6  // CHANGE "WORDS" TO "SIZE" ?
#define SKC_DEVICE_SUBBLOCK_WORDS_LOG2          3

#define SKC_TILE_WIDTH_LOG2                     SKC_DEVICE_SUBBLOCK_WORDS_LOG2
#define SKC_TILE_HEIGHT_LOG2                    (SKC_DEVICE_SUBBLOCK_WORDS_LOG2 + 1)

/////////////////////////////////////////////////////////////////
//
// BLOCK POOL INIT
//

#define SKC_BP_INIT_IDS_KERNEL_ATTRIBS
#define SKC_BP_INIT_ATOMICS_KERNEL_ATTRIBS      __attribute__((reqd_work_group_size(2,1,1)))

/////////////////////////////////////////////////////////////////
//
// PATHS ALLOC
//

#define SKC_PATHS_ALLOC_KERNEL_ATTRIBS          __attribute__((reqd_work_group_size(1,1,1)))

/////////////////////////////////////////////////////////////////
//
// PATHS COPY
//

#define SKC_PATHS_COPY_SUBGROUP_SIZE_LOG2       SKC_DEVICE_SUBBLOCK_WORDS_LOG2 // FIXME -- SUBGROUP OR THREADS PER BLOCK?
#define SKC_PATHS_COPY_ELEM_WORDS               1
#define SKC_PATHS_COPY_ELEM_EXPAND()            SKC_EXPAND_1()

#define SKC_PATHS_COPY_KERNEL_ATTRIBS           __attribute__((intel_reqd_sub_group_size(SKC_PATHS_COPY_SUBGROUP_SIZE)))

#define SKC_IS_NOT_PATH_HEAD(sg,I)              ((sg) + get_sub_group_local_id() >= SKC_PATH_HEAD_WORDS)

typedef skc_uint  skc_paths_copy_elem;
typedef skc_uint  skc_pb_idx_v;

/////////////////////////////////////////////////////////////////
//
// FILLS EXPAND
//

#define SKC_FILLS_EXPAND_SUBGROUP_SIZE_LOG2     SKC_DEVICE_SUBBLOCK_WORDS_LOG2
#define SKC_FILLS_EXPAND_ELEM_WORDS             1

#define SKC_FILLS_EXPAND_KERNEL_ATTRIBS         __attribute__((intel_reqd_sub_group_size(SKC_FILLS_EXPAND_SUBGROUP_SIZE)))

/////////////////////////////////////////////////////////////////
//
// RASTER ALLOC
//
// NOTE -- Intel subgroup shuffles aren't supported in SIMD32 which is
// why use of the subgroup broadcast produces a compiler error. So a
// subgroup of size 16 is this widest we can require.
//

#define SKC_RASTERS_ALLOC_GROUP_SIZE            16

#if (SKC_RASTERS_ALLOC_GROUP_SIZE <= 16)

#define SKC_RASTERS_ALLOC_KERNEL_ATTRIBS        __attribute__((intel_reqd_sub_group_size(SKC_RASTERS_ALLOC_GROUP_SIZE)))
#define SKC_RASTERS_ALLOC_LOCAL_ID()            get_sub_group_local_id()
#define SKC_RASTERS_ALLOC_INCLUSIVE_ADD(v)      sub_group_scan_inclusive_add(v)
#define SKC_RASTERS_ALLOC_BROADCAST(v,i)        sub_group_broadcast(v,i)

#else

#define SKC_RASTERS_ALLOC_KERNEL_ATTRIBS        __attribute__((reqd_work_group_size(SKC_RASTERS_ALLOC_GROUP_SIZE,1,1)))
#define SKC_RASTERS_ALLOC_LOCAL_ID()            get_local_id(0)
#define SKC_RASTERS_ALLOC_INCLUSIVE_ADD(v)      work_group_scan_inclusive_add(v)
#define SKC_RASTERS_ALLOC_BROADCAST(v,i)        work_group_broadcast(v,i)

#endif

/////////////////////////////////////////////////////////////////
//
// RASTERIZE
//

#define SKC_RASTERIZE_SUBGROUP_SIZE             SKC_DEVICE_SUBBLOCK_WORDS
#define SKC_RASTERIZE_VECTOR_SIZE_LOG2          0
#define SKC_RASTERIZE_WORKGROUP_SUBGROUPS       1

#define SKC_RASTERIZE_KERNEL_ATTRIBS                                    \
  __attribute__((intel_reqd_sub_group_size(SKC_RASTERIZE_SUBGROUP_SIZE))) \
  __attribute__((reqd_work_group_size(SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_WORKGROUP_SUBGROUPS, 1, 1)))

#define SKC_RASTERIZE_FLOAT                     float
#define SKC_RASTERIZE_UINT                      uint
#define SKC_RASTERIZE_INT                       int
#define SKC_RASTERIZE_PREDICATE                 bool
#define SKC_RASTERIZE_POOL                      uint

#define SKC_RASTERIZE_TILE_HASH_X_BITS          1
#define SKC_RASTERIZE_TILE_HASH_Y_BITS          2

typedef skc_block_id_t skc_block_id_v_t;
typedef skc_uint2      skc_ttsk_v_t;
typedef skc_uint2      skc_ttsk_s_t;

// SKC_STATIC_ASSERT(SKC_RASTERIZE_POOL_SIZE > SKC_RASTERIZE_SUBGROUP_SIZE);

/////////////////////////////////////////////////////////////////
//
// PREFIX
//

#define SKC_PREFIX_SUBGROUP_SIZE               8 // for now this had better be SKC_DEVICE_SUBBLOCK_WORDS
#define SKC_PREFIX_WORKGROUP_SUBGROUPS         1

#define SKC_PREFIX_KERNEL_ATTRIBS                                       \
  __attribute__((intel_reqd_sub_group_size(SKC_PREFIX_SUBGROUP_SIZE)))  \
  __attribute__((reqd_work_group_size(SKC_PREFIX_SUBGROUP_SIZE * SKC_PREFIX_WORKGROUP_SUBGROUPS, 1, 1)))

#define SKC_PREFIX_TTP_V                       skc_uint2
#define SKC_PREFIX_TTS_V_BITFIELD              skc_int

#define SKC_PREFIX_TTS_VECTOR_INT_EXPAND       SKC_EXPAND_1

#define SKC_PREFIX_SMEM_ZERO                   ulong
#define SKC_PREFIX_SMEM_ZERO_WIDTH             (sizeof(SKC_PREFIX_SMEM_ZERO) / sizeof(skc_ttp_t))
#define SKC_PREFIX_SMEM_COUNT_BLOCK_ID         8

#define SKC_PREFIX_BLOCK_ID_V_SIZE             SKC_PREFIX_SUBGROUP_SIZE

#define SKC_PREFIX_TTXK_V_SIZE                 SKC_PREFIX_SUBGROUP_SIZE
#define SKC_PREFIX_TTXK_V_MASK                 (SKC_PREFIX_TTXK_V_SIZE - 1)

typedef skc_uint       skc_bp_elem_t;

typedef skc_uint2      skc_ttrk_e_t;
typedef skc_uint2      skc_ttsk_v_t;
typedef skc_uint2      skc_ttsk_s_t;
typedef skc_uint2      skc_ttpk_s_t;
typedef skc_uint2      skc_ttxk_v_t;

typedef skc_int        skc_tts_v_t;

typedef skc_int        skc_ttp_t;

typedef skc_uint       skc_raster_yx_s;

typedef skc_block_id_t skc_block_id_v_t;
typedef skc_block_id_t skc_block_id_s_t;

/////////////////////////////////////////////////////////////////
//
// PLACE
//

#define SKC_PLACE_SUBGROUP_SIZE                16
#define SKC_PLACE_WORKGROUP_SUBGROUPS          1

#define SKC_PLACE_KERNEL_ATTRIBS                                       \
  __attribute__((intel_reqd_sub_group_size(SKC_PLACE_SUBGROUP_SIZE)))  \
  __attribute__((reqd_work_group_size(SKC_PLACE_SUBGROUP_SIZE * SKC_PLACE_WORKGROUP_SUBGROUPS, 1, 1)))

typedef skc_uint  skc_bp_elem_t;

typedef skc_uint  skc_ttsk_lo_t;
typedef skc_uint  skc_ttsk_hi_t;

typedef skc_uint  skc_ttpk_lo_t;
typedef skc_uint  skc_ttpk_hi_t;

typedef skc_uint  skc_ttxk_lo_t;
typedef skc_uint  skc_ttxk_hi_t;

typedef skc_uint2 skc_ttck_t;

typedef skc_bool  skc_pred_v_t;
typedef skc_int   skc_int_v_t;

/////////////////////////////////////////////////////////////////
//
// RENDER
//

#define SKC_ARCH_GEN9

#if defined(__OPENCL_C_VERSION__)
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif

#define SKC_RENDER_SUBGROUP_SIZE               8
#define SKC_RENDER_WORKGROUP_SUBGROUPS         1

#define SKC_RENDER_KERNEL_ATTRIBS                                       \
  __attribute__((intel_reqd_sub_group_size(SKC_RENDER_SUBGROUP_SIZE)))  \
  __attribute__((reqd_work_group_size(SKC_RENDER_SUBGROUP_SIZE * SKC_RENDER_WORKGROUP_SUBGROUPS, 1, 1)))

#define SKC_RENDER_SCANLINE_VECTOR_SIZE        2

#define SKC_RENDER_REGS_COLOR_R                2
#define SKC_RENDER_REGS_COVER_R                3

#define SKC_RENDER_TTSB_EXPAND()               SKC_EXPAND_1()

#define SKC_RENDER_TTS_V                       skc_int
#define SKC_RENDER_TTS_V_BITFIELD              skc_int

#define SKC_RENDER_TTP_V                       skc_int2
#define SKC_RENDER_AREA_V                      skc_int2

#define SKC_RENDER_TILE_COLOR_PAIR             half2
#define SKC_RENDER_TILE_COLOR_PAIR_LOAD(x,v)   vload2(x,v)

#define SKC_RENDER_SURFACE_COLOR               half4
#define SKC_RENDER_SURFACE_WRITE               write_imageh

// #define SKC_RENDER_TTXB_VECTOR_INT             int2
// #define SKC_RENDER_TTXB_VECTOR_UINT            uint2

#define SKC_RENDER_WIDE_AA                     ulong // SLM = 64 bytes/clock

#define SKC_RENDER_TILE_COLOR                  half2
#define SKC_RENDER_TILE_COVER                  half2

#define SKC_RENDER_ACC_COVER_INT               int2
#define SKC_RENDER_ACC_COVER_UINT              uint2

#define SKC_RENDER_GRADIENT_FLOAT              float2
#define SKC_RENDER_GRADIENT_INT                int2
#define SKC_RENDER_GRADIENT_STOP               int2
#define SKC_RENDER_GRADIENT_FRAC               half2
#define SKC_RENDER_GRADIENT_COLOR_STOP         half

#define SKC_RENDER_SURFACE_U8_RGBA             uint2

#define SKC_RENDER_TILE_COLOR_VECTOR           uint16
#define SKC_RENDER_TILE_COLOR_VECTOR_COMPONENT uint
#define SKC_RENDER_TILE_COLOR_VECTOR_COUNT     ((sizeof(SKC_RENDER_TILE_COLOR) * 4 * SKC_TILE_WIDTH) / sizeof(SKC_RENDER_TILE_COLOR_VECTOR))

/////////////////////////////////////////////////////////////////
//
// PATHS & RASTERS RECLAIM
//
// FIXME -- investigate enabling the stride option for a smaller grid
// that iterates over a fixed number of threads.  Since reclamation is
// a low-priority task, it's probably reasonable to trade longer
// reclamation times for lower occupancy of the device because it
// might delay the fastpath of the pipeline.
//

#define SKC_RECLAIM_ARRAY_SIZE                  (7 * 8 / 2) // 8 EUs with 7 hardware threads divided by 2 is half a sub-slice

/////////////////////////////////////////////////////////////////
//
// PATHS RECLAIM
//

#define SKC_PATHS_RECLAIM_SUBGROUP_SIZE_LOG2    SKC_DEVICE_SUBBLOCK_WORDS_LOG2 // FIXME -- SUBGROUP OR THREADS PER BLOCK?
#define SKC_PATHS_RECLAIM_LOCAL_ELEMS           1
#define SKC_PATHS_RECLAIM_KERNEL_ATTRIBS        __attribute__((intel_reqd_sub_group_size(SKC_PATHS_RECLAIM_SUBGROUP_SIZE)))

/////////////////////////////////////////////////////////////////
//
// RASTERS RECLAIM
//

#define SKC_RASTERS_RECLAIM_SUBGROUP_SIZE_LOG2  SKC_DEVICE_SUBBLOCK_WORDS_LOG2 // FIXME -- SUBGROUP OR THREADS PER BLOCK?
#define SKC_RASTERS_RECLAIM_LOCAL_ELEMS         1
#define SKC_RASTERS_RECLAIM_KERNEL_ATTRIBS      __attribute__((intel_reqd_sub_group_size(SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)))

//
// COMMON -- FIXME -- HOIST THESE ELSEWHERE
//

#define SKC_DEVICE_BLOCK_WORDS                 (1u << SKC_DEVICE_BLOCK_WORDS_LOG2)
#define SKC_DEVICE_SUBBLOCK_WORDS              (1u << SKC_DEVICE_SUBBLOCK_WORDS_LOG2)

#define SKC_DEVICE_BLOCK_DWORDS                (SKC_DEVICE_BLOCK_WORDS / 2)

#define SKC_DEVICE_BLOCK_WORDS_MASK            SKC_BITS_TO_MASK(SKC_DEVICE_BLOCK_WORDS_LOG2)
#define SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK    SKC_BITS_TO_MASK(SKC_DEVICE_BLOCK_WORDS_LOG2 - SKC_DEVICE_SUBBLOCK_WORDS_LOG2)

#define SKC_DEVICE_SUBBLOCKS_PER_BLOCK         (SKC_DEVICE_BLOCK_WORDS / SKC_DEVICE_SUBBLOCK_WORDS)

#define SKC_TILE_RATIO                         (SKC_TILE_HEIGHT / SKC_TILE_WIDTH)

//
//
//

#define SKC_PATHS_COPY_SUBGROUP_SIZE           (1 << SKC_PATHS_COPY_SUBGROUP_SIZE_LOG2)
#define SKC_PATHS_RECLAIM_SUBGROUP_SIZE        (1 << SKC_PATHS_RECLAIM_SUBGROUP_SIZE_LOG2)
#define SKC_RASTERS_RECLAIM_SUBGROUP_SIZE      (1 << SKC_RASTERS_RECLAIM_SUBGROUP_SIZE_LOG2)
#define SKC_FILLS_EXPAND_SUBGROUP_SIZE         (1 << SKC_FILLS_EXPAND_SUBGROUP_SIZE_LOG2)

//
//
//

#endif

//
//
//

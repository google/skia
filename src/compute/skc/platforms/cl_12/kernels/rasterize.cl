/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "tile.h"
#include "common.h"
#include "atomic_cl.h"
#include "block_pool_cl.h"
#include "raster_builder_cl_12.h"
#include "kernel_cl_12.h"

// #define SKC_ARCH_AVX2
// #define SKC_RASTERIZE_SIMD_USES_SMEM

#define PRINTF_ENABLE       0
#define PRINTF_BLOCK_COUNT  0

//
// NOTE:
//
// ON SIMD DEVICES THE BIN COUNT MUST BE POW2 SO THAT WE CAN LOAD IT
// AS A VECTOR AND PERFORM A SWIZZLE/SHUFFLE
//
// NOTE:
//
// IGNORE FOR NOW ANY AVX2 CODE SNIPPETS.  THEY WILL BE MOVED ASAP.
//
//

#if 0 // SKC_ARCH_AVX2

// #define SKC_RASTERIZE_SUBGROUP_SIZE              1
// #define SKC_RASTERIZE_VECTOR_SIZE_LOG2           3
// #define SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP   1

// #define SKC_TTXB_WORDS                           8

// #define SKC_RASTERIZE_FLOAT                      float8
// #define SKC_RASTERIZE_UINT                       uint8
// #define SKC_RASTERIZE_INT                        int8
// #define SKC_RASTERIZE_PREDICATE                  int8

// #define SKC_RASTERIZE_BIN_BLOCK                  uint16
// #define SKC_RASTERIZE_BIN                        uint8

// #define SKC_RASTERIZE_POOL                       uint8
// #define SKC_RASTERIZE_POOL_SCALE                 6

// #define SKC_RASTERIZE_TILE_HASH_X_BITS           1
// #define SKC_RASTERIZE_TILE_HASH_Y_BITS           2

// #define SKC_RASTERIZE_VECTOR_EXPAND()            SKC_EXPAND_8()

#endif

//
// SIMT
//

#define SKC_RASTERIZE_BLOCK_ID_V_SIZE        SKC_RASTERIZE_SUBGROUP_SIZE
#define SKC_RASTERIZE_TTSK_V_SIZE            SKC_RASTERIZE_SUBGROUP_SIZE
#define SKC_RASTERIZE_TTSK_V_MASK            (SKC_RASTERIZE_TTSK_V_SIZE - 1)

//
//
//

#define SKC_RASTERIZE_VECTOR_SIZE            (1 << SKC_RASTERIZE_VECTOR_SIZE_LOG2)
#define SKC_RASTERIZE_ELEMS_PER_SUBGROUP     (SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_VECTOR_SIZE)

//
//
//

#define SKC_RASTERIZE_YX_INIT                0x7FFF7FFF  // { +32767, +32767 }
#define SKC_RASTERIZE_YX_INVALID             0x80008000  // { -32768, -32768 }

//
//
//

#define SKC_RASTERIZE_TILE_HASH_X_MASK       SKC_BITS_TO_MASK(SKC_RASTERIZE_TILE_HASH_X_BITS)
#define SKC_RASTERIZE_TILE_HASH_Y_MASK       SKC_BITS_TO_MASK(SKC_RASTERIZE_TILE_HASH_Y_BITS)
#define SKC_RASTERIZE_TILE_HASH_BITS         (SKC_RASTERIZE_TILE_HASH_X_BITS + SKC_RASTERIZE_TILE_HASH_Y_BITS)
#define SKC_RASTERIZE_TILE_HASH_BIN_COUNT    (1 << SKC_RASTERIZE_TILE_HASH_BITS)
#define SKC_RASTERIZE_TILE_HASH_BIN_BITS     (SKC_RASTERIZE_TILE_HASH_BITS + 1) // FIXME -- LOG2_RU(BIN_COUNT)
#define SKC_RASTERIZE_TILE_HASH_BIN_MASK     SKC_BITS_TO_MASK(SKC_RASTERIZE_TILE_HASH_BIN_BITS)

//
// Norbert Juffa notes: "GPU Pro Tip: Lerp Faster in C++"
//
// https://devblogs.nvidia.com/parallelforall/lerp-faster-cuda/
//
// Lerp in two fma/mad ops:
//
//    t * b + ((-t) * a + a)
//
// Note: OpenCL documents mix() as being implemented as:
//
//    a + (b - a) * t
//
// But this may be a native instruction on some devices. For example,
// on GEN9 there is an LRP "linear interoplation" opcode but it
// doesn't appear to support half floats.
//
// Feel free to toggle this option and then benchmark and inspect the
// generated code.  We really want the double FMA to be generated when
// there isn't support for a LERP/MIX operation.
//

#if 1
#define SKC_LERP(a,b,t)      mad(t,b,mad(-(t),a,a))
#else
#define SKC_LERP(a,b,t)      mix(a,b,t)
#endif

//
// There is no integer MAD in OpenCL with "don't care" overflow
// semantics.
//
// FIXME -- verify if the platform needs explicit MAD operations even
// if a "--fastmath" option is available at compile time.  It might
// make sense to explicitly use MAD calls if the platform requires it.
//

#if 1
#define SKC_MAD_UINT(a,b,c)  ((a) * (b) + (c))
#else
#define SKC_MAD_UINT(a,b,c)  mad_sat(a,b,c)
#endif

//
//
//

#define SKC_RASTERIZE_SEGMENT(id) (id * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane())

//
//
//

union skc_bp_elem
{
  skc_uint              u32;
  skc_tagged_block_id_t tag_id;
  skc_float             coord;
};

//
//
//

struct skc_subgroup_smem
{
  //
  // SIMT subgroup scratchpad for max scan -- also shared with 'winner' member
  //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 ) || defined ( SKC_RASTERIZE_SIMD_USES_SMEM )
  struct {
    union {

      skc_uint                winner;

      struct {
        skc_uint              scratch[SKC_RASTERIZE_SUBGROUP_SIZE];
      } aN;

      struct {
        SKC_RASTERIZE_UINT    scratch[SKC_RASTERIZE_SUBGROUP_SIZE];
      } vN;
    };
  } subgroup;
#endif

  //
  // work-in-progress TTSB blocks and associated YX keys
  //
  union {
    struct {
      // FIXME -- some typedefs are valid here
      skc_uint                ttsb [SKC_RASTERIZE_TILE_HASH_BIN_COUNT][SKC_DEVICE_SUBBLOCK_WORDS];
      skc_uint                yx   [SKC_RASTERIZE_TILE_HASH_BIN_COUNT];
      skc_uint                id   [SKC_RASTERIZE_TILE_HASH_BIN_COUNT];
      skc_uint                count[SKC_RASTERIZE_TILE_HASH_BIN_COUNT];
    } aN;
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
    struct {
      SKC_RASTERIZE_BIN_BLOCK ttsb[SKC_RASTERIZE_TILE_HASH_BIN_COUNT];
      SKC_RASTERIZE_BIN       yx;
      SKC_RASTERIZE_BIN       id;
      SKC_RASTERIZE_BIN       count;
    } vN;
#endif
  } bin;
};

//
//
//

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
#define skc_subgroup_lane()  0
#else
#define skc_subgroup_lane()  get_sub_group_local_id()
#endif

//
// replenish block ids
//
// note that you can't overrun the block id pool since it's a ring
//

static
void
skc_blocks_replenish(skc_uint                           * const blocks_next,
                     skc_block_id_v_t                   * const blocks,
                     __global SKC_ATOMIC_UINT  volatile * const bp_atomics,
                     skc_uint                             const bp_mask, // pow2 modulo mask for block pool ring
                     __global skc_block_id_t   const    * const bp_ids)
{
  //
  // get a new vector of block ids -- this is kind of a narrow
  // allocation but subblocks help stretch out the pool.
  //
  // FIXME -- there is now plenty of SMEM to allocate a LOT of block ids
  //
  skc_uint bp_idx = 0;

  if (skc_subgroup_lane() == 0)
    {
      bp_idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(bp_atomics+SKC_BP_ATOMIC_OFFSET_READS,
                                                    SKC_RASTERIZE_BLOCK_ID_V_SIZE); // ring_reads
#if 0
      printf("r+: %8u + %u\n",bp_idx,SKC_RASTERIZE_BLOCK_ID_V_SIZE);
#endif
    }

  bp_idx       = (sub_group_broadcast(bp_idx,0) + skc_subgroup_lane()) & bp_mask;
  *blocks      = bp_ids[bp_idx];
  *blocks_next = 0;
}

//
//
//

static
skc_block_id_t
skc_blocks_get_next(skc_uint                           * const blocks_next,
                    skc_block_id_v_t                   * const blocks,
                    __global SKC_ATOMIC_UINT  volatile * const bp_atomics,
                    skc_uint                             const bp_mask, // pow2 modulo mask for block pool ring
                    __global skc_block_id_t   const    * const bp_ids)
{
  // replenish?
  if (*blocks_next == SKC_RASTERIZE_BLOCK_ID_V_SIZE)
    {
      skc_blocks_replenish(blocks_next,blocks,bp_atomics,bp_mask,bp_ids);
    }

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 )
  //
  // SIMT
  //
  skc_block_id_t id = sub_group_broadcast(*blocks,*blocks_next);

#else
  //
  // SIMD
  //
  skc_block_id_t id = blocks->s0;

  skc_shuffle_down_1(*blocks);

#endif

  *blocks_next += 1;

  return id;
}

//
// subblock allocator
//

#if SKC_DEVICE_BLOCK_WORDS_LOG2 > SKC_DEVICE_SUBBLOCK_WORDS_LOG2

static
skc_block_id_t
skc_subblocks_get_next(skc_block_id_t                     * const subblocks,
                       skc_uint                           * const blocks_next,
                       skc_block_id_v_t                   * const blocks,
                       __global SKC_ATOMIC_UINT  volatile * const bp_atomics,
                       skc_uint                             const bp_mask, // pow2 modulo mask for block pool ring
                       __global skc_block_id_t   const    * const bp_ids)
{
  if ((*subblocks & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK) == 0)
    {
      *subblocks = skc_blocks_get_next(blocks_next,blocks,bp_atomics,bp_mask,bp_ids);
    }

  skc_block_id_t const sb_id = *subblocks;

  *subblocks += 1;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("= %u\n",sb_id);
#endif

  return sb_id;
}


#define SKC_SUBBLOCKS_BLOCKS_PROTO() skc_block_id_t * const subblocks, skc_block_id_t * const blocks
#define SKC_SUBBLOCKS_BLOCKS_ARGS()  subblocks, blocks

#else

#define SKC_SUBBLOCKS_BLOCKS_PROTO() skc_block_id_t * const blocks
#define SKC_SUBBLOCKS_BLOCKS_ARGS()  blocks

#endif

//
//
//

static
skc_block_id_t
skc_ttsk_v_append(SKC_SUBBLOCKS_BLOCKS_PROTO(),
                  skc_uint                           * const blocks_next,
                  __global SKC_ATOMIC_UINT  volatile * const bp_atomics,
                  skc_uint                             const bp_mask, // pow2 modulo mask for block pool ring
                  __global skc_block_id_t   const    * const bp_ids,
                  __global SKC_ATOMIC_UINT  volatile * const cohort_atomics,
                  skc_ttsk_v_t                       * const sk_v,
                  skc_uint                           * const sk_v_next,
                  __global skc_ttsk_s_t              * const sk_extent,
                  skc_uint                             const new_yx)
{
#if SKC_DEVICE_BLOCK_WORDS_LOG2 > SKC_DEVICE_SUBBLOCK_WORDS_LOG2
  skc_block_id_t const new_id = skc_subblocks_get_next(subblocks,
                                                       blocks_next,
                                                       blocks,
                                                       bp_atomics,
                                                       bp_mask,
                                                       bp_ids);
#else
  skc_block_id_t const new_id = skc_blocks_get_next(blocks_next,
                                                    blocks,
                                                    bp_atomics,
                                                    bp_mask, // pow2 modulo mask for block pool ring
                                                    bp_ids);
#endif

  if (get_sub_group_local_id() == (*sk_v_next & SKC_RASTERIZE_TTSK_V_MASK))
    {
      sk_v->lo = new_id;
      sk_v->hi = (sk_v->hi & SKC_TTRK_HI_MASK_COHORT) | new_yx;
#if 0
      printf("@ ( %3u, %3u ) %u\n",
             (new_yx >> 12) & 0xFFF,
             (new_yx      ) & 0xFFF,
             new_id);
#endif
    }

  *sk_v_next += 1;

  if (*sk_v_next == SKC_RASTERIZE_TTSK_V_SIZE)
    {
      *sk_v_next = 0;

      skc_uint sk_idx = 0;

      if (skc_subgroup_lane() == 0)
        {
          sk_idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE
            (cohort_atomics+SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS,SKC_RASTERIZE_TTSK_V_SIZE);
#if 0
          printf("+ %u\n",sk_idx);
#endif
        }

      sk_idx = sub_group_broadcast(sk_idx,0) + skc_subgroup_lane();

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > SKC_RASTERIZE_TTSK_V_SIZE )
      if (skc_subgroup_lane() < SKC_RASTERIZE_TTSK_V_SIZE)
#endif
        {
          sk_extent[sk_idx] = *sk_v;
#if 0
          printf("> %u : %v2u\n",sk_idx,*sk_v);
#endif
        }
    }

  return new_id;
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_subgroup_scan_inclusive_add_float(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  //  0 +
  // --
  // 01
  SKC_RASTERIZE_FLOAT const w = mad(v.s10,(SKC_RASTERIZE_FLOAT)(0,1),v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  //  012 +
  // ----
  // 0123
  //   01 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.s3012,(SKC_RASTERIZE_FLOAT)(0,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.s2301,(SKC_RASTERIZE_FLOAT)(0,0,1,1),w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  //  0123456 +
  // --------
  // 01234567
  //   012345 +
  // --------
  // 01234567
  //     0123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.s70123456,(SKC_RASTERIZE_FLOAT)(0,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.s67012345,(SKC_RASTERIZE_FLOAT)(0,0,1,1,1,1,1,1),w);
  SKC_RASTERIZE_FLOAT const y = mad(x.s45670123,(SKC_RASTERIZE_FLOAT)(0,0,0,0,1,1,1,1),x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  //  0123456789abcde +
  // ----------------
  // 0123456789abcdef
  //   0123456789abcd +
  // ----------------
  // 0123456789abcdef
  //     0123456789ab +
  // ----------------
  // 0123456789abcdef
  //         01234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.sf0123456789abcde,(SKC_RASTERIZE_FLOAT)(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.sef0123456789abcd,(SKC_RASTERIZE_FLOAT)(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1),w);
  SKC_RASTERIZE_FLOAT const y = mad(x.scdef0123456789ab,(SKC_RASTERIZE_FLOAT)(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),x);
  SKC_RASTERIZE_FLOAT const z = mad(y.s89abcdef01234567,(SKC_RASTERIZE_FLOAT)(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_add(v);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_subgroup_scan_inclusive_add_uint(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  //  0 +
  // --
  // 01
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s10,(SKC_RASTERIZE_UINT)(0,1),v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  //  012 +
  // ----
  // 0123
  //   01 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s3012,(SKC_RASTERIZE_UINT)(0,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.s2301,(SKC_RASTERIZE_UINT)(0,0,1,1),w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  //  0123456 +
  // --------
  // 01234567
  //   012345 +
  // --------
  // 01234567
  //     0123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s70123456,(SKC_RASTERIZE_UINT)(0,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.s67012345,(SKC_RASTERIZE_UINT)(0,0,1,1,1,1,1,1),w);
  SKC_RASTERIZE_UINT const y = SKC_MAD_UINT(x.s45670123,(SKC_RASTERIZE_UINT)(0,0,0,0,1,1,1,1),x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  //  0123456789abcde +
  // ----------------
  // 0123456789abcdef
  //   0123456789abcd +
  // ----------------
  // 0123456789abcdef
  //     0123456789ab +
  // ----------------
  // 0123456789abcdef
  //         01234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.sf0123456789abcde,(SKC_RASTERIZE_UINT)(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.sef0123456789abcd,(SKC_RASTERIZE_UINT)(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1),w);
  SKC_RASTERIZE_UINT const y = SKC_MAD_UINT(x.scdef0123456789ab,(SKC_RASTERIZE_UINT)(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),x);
  SKC_RASTERIZE_UINT const z = SKC_MAD_UINT(y.s89abcdef01234567,(SKC_RASTERIZE_UINT)(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_add(v);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_subgroup_scan_inclusive_max(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  // 00 max
  // --
  // 01
  SKC_RASTERIZE_UINT const w = max(v.s00,v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  // 0012 +
  // ----
  // 0123
  // 0101 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_UINT const w = max(v.s0012,v);
  SKC_RASTERIZE_UINT const x = max(w.s0101,w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  // 00123456 +
  // --------
  // 01234567
  // 01012345 +
  // --------
  // 01234567
  // 01230123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_UINT const w = max(v.s00123456,v);
  SKC_RASTERIZE_UINT const x = max(w.s01012345,w);
  SKC_RASTERIZE_UINT const y = max(x.s01230123,x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  // 00123456789abcde +
  // ----------------
  // 0123456789abcdef
  // 010123456789abcd +
  // ----------------
  // 0123456789abcdef
  // 01230123456789ab +
  // ----------------
  // 0123456789abcdef
  // 0123456701234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_UINT const w = max(v.s00123456789abcde,v);
  SKC_RASTERIZE_UINT const x = max(w.s010123456789abcd,w);
  SKC_RASTERIZE_UINT const y = max(x.s01230123456789ab,x);
  SKC_RASTERIZE_UINT const z = max(y.s0123456701234567,y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_max(v);

#endif
}

//
//
//

static
float
skc_subgroup_last_float(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return v.s1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return v.s3;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return v.s7;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return v.sf;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,SKC_RASTERIZE_SUBGROUP_SIZE-1);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_subgroup_last_uint(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return v.s1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return v.s3;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return v.s7;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return v.sf;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,SKC_RASTERIZE_SUBGROUP_SIZE-1);

#endif
}

//
//
//

static
float
skc_subgroup_first(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#else
  return v.s0;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,0);

#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_subgroup_shuffle(SKC_RASTERIZE_FLOAT const v,
                      SKC_RASTERIZE_UINT  const i)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#else
  return shuffle(v,i);
#endif

#else
  //
  // SIMT
  //
  return intel_sub_group_shuffle(v,i);

#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_subgroup_shuffle_up_1(SKC_RASTERIZE_FLOAT const p, // previous
                          SKC_RASTERIZE_FLOAT const c) // current
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // FIXME -- there are alternative formulations here:
  //
  // Option 1:
  //
  //   select(c.rotate(+1),p.rotate(-1),(1,0,0,...))
  //
  // Option 2:
  //
  //   p is a scalar
  //   t    = c.rotate(+1)
  //   t.s0 = p;
  //
  // Option 3: ...
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return p;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return shuffle2(p,c,(uint2)(1,2));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return shuffle2(p,c,(uint4)(3,4,5,6));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return shuffle2(p,c,(uint8)(7,8,9,10,11,12,13,14));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return shuffle2(p,c,(uint16)(15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30));
#endif

#else
  //
  // SIMT
  //
  return intel_sub_group_shuffle_up(p,c,1);

#endif
}

//
//
//

static
bool
skc_is_lane_first()
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
  //
  // SIMD
  //
  return true;
#else
  //
  // SIMT
  //
  return get_sub_group_local_id() == 0;
#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_delta_offset()
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return 1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4, 5, 6, 7, 8 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 );
#endif

#else
  //
  // SIMT
  //
  return 1.0f + get_sub_group_local_id();

#endif

}

//
//
//

static
int
skc_subgroup_any(SKC_RASTERIZE_PREDICATE const p)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  return any(p);
#else
  //
  // SIMT
  //
  return sub_group_any(p);
#endif
}

//
//
//

#define SKC_PATH_NODEWORD_IS_LAST(n)  (((n) & SKC_DEVICE_BLOCK_WORDS_MASK) == SKC_DEVICE_BLOCK_WORDS_MASK)

void
skc_segment_next(__global union skc_bp_elem * const bp_elems,
                 skc_uint                   * const nodeword,
                 skc_block_id_t             * const id)
{
  if ((++*id & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK) == 0)
    {
      if (SKC_PATH_NODEWORD_IS_LAST(++*nodeword))
        {
          *nodeword = SKC_TAGGED_BLOCK_ID_GET_ID(bp_elems[*nodeword].tag_id) * SKC_DEVICE_SUBBLOCK_WORDS;
        }

      skc_tagged_block_id_t const tag_id = bp_elems[*nodeword].tag_id;

      *id = SKC_TAGGED_BLOCK_ID_GET_ID(tag_id);
    }
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_native_length(SKC_RASTERIZE_FLOAT const x, SKC_RASTERIZE_FLOAT const y)
{
  return native_sqrt(x * x + y * y);
}

//
// Wang's Formula (1985)
//

#define SKC_WANG_PIXEL_RESL   0.25f // <-- this can be tuned

#define SKC_WANG_EPSILON      (SKC_WANG_PIXEL_RESL * SKC_SUBPIXEL_RESL_X_F32)

#define SKC_WANG_CUBIC        ((3.0f * 2.0f) / (8.0f * SKC_WANG_EPSILON))
#define SKC_WANG_QUADRATIC    ((2.0f       ) / (8.0f * SKC_WANG_EPSILON))

#define SKC_WANG_LENGTH(x,y)  skc_native_length(x,y)
#define SKC_WANG_SQRT(x)      native_sqrt(x)

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_cubic(SKC_RASTERIZE_FLOAT const t0x, SKC_RASTERIZE_FLOAT const t0y,
                        SKC_RASTERIZE_FLOAT const t1x, SKC_RASTERIZE_FLOAT const t1y,
                        SKC_RASTERIZE_FLOAT const t2x, SKC_RASTERIZE_FLOAT const t2y,
                        SKC_RASTERIZE_FLOAT const t3x, SKC_RASTERIZE_FLOAT const t3y)
{
  //
  // Return the number of evenly spaced (in the parametric sense) line
  // segments that are guaranteed to be within "epsilon" error of the
  // curve.
  //
  // We're then going to take multiples of the reciprocal of this
  // number so that the segmentation can be distributed across the
  // subgroup.
  //
  // Note, this can probably be slightly optimized per architecture
  // but it's probably far from being a hotspot since it's all
  // straight-line unpredicated code.
  //
  // The result is an integer ranging from [1.0,#segments]
  //
  // Note that even if all of the control points are coincident, the
  // max(1.0f) will categorize this as a line of 1 segment.
  //
  // This is what we want!  We want to convert cubics to lines as
  // easily as possible and *then* cull lines that are either
  // horizontal or zero length.
  //
  return max(1.0f,
             ceil(SKC_WANG_SQRT(SKC_WANG_CUBIC *
                                SKC_WANG_LENGTH(max(fabs(t2x - 2.0f * t1x + t0x),
                                                    fabs(t3x - 2.0f * t2x + t1x)),
                                                max(fabs(t2y - 2.0f * t1y + t0y),
                                                    fabs(t3y - 2.0f * t2y + t1y))))));
}

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_quadratic(SKC_RASTERIZE_FLOAT const t0x, SKC_RASTERIZE_FLOAT const t0y,
                            SKC_RASTERIZE_FLOAT const t1x, SKC_RASTERIZE_FLOAT const t1y,
                            SKC_RASTERIZE_FLOAT const t2x, SKC_RASTERIZE_FLOAT const t2y)
{
  return max(1.0f,
             ceil(SKC_WANG_SQRT(SKC_WANG_QUADRATIC *
                                SKC_WANG_LENGTH(t2x - 2.0f * t1x + t0x,
                                                t2y - 2.0f * t1y + t0y))));
}

//
// rational curves
//

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_cubic_rat()
{
  return 0.0f;
}

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_quad_rat()
{
  return 0.0f;
}

//
// flush any work-in-progress blocks and return unused block ids
//

static
void
skc_finalize(__global SKC_ATOMIC_UINT          volatile * const bp_atomics,
             __global union skc_bp_elem                 * const bp_elems,
             __global uint                              * const bp_ids,
             skc_uint                                     const bp_mask,
             __global SKC_ATOMIC_UINT          volatile * const cohort_atomics,
             skc_block_id_v_t                           * const blocks,
             skc_uint                                     const blocks_next,
             skc_ttsk_v_t                               * const sk_v,
             skc_uint                                     const sk_v_next,
             __global skc_ttsk_s_t                      * const sk_extent,
             __local  struct skc_subgroup_smem volatile * const smem)
{
  //
  // flush non-empty bins
  //
  // FIXME -- accelerate this iteration/search with a subgroup operation
  //
  for (skc_uint ii=0; ii<SKC_RASTERIZE_TILE_HASH_BIN_COUNT; ii++)
    {
      if (smem->bin.aN.count[ii] > 0)
        {
          skc_block_id_v_t const id  = smem->bin.aN.id[ii];
          skc_uint         const idx = id * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane();
          skc_uint         const tts = smem->bin.aN.ttsb[ii][skc_subgroup_lane()];
#if 0
          printf("???????? : [ %10u = %10u : %08X ]\n",id,idx,tts);
#endif
          bp_elems[idx].u32 = tts;
        }

      //
      // FIXME -- vectorize with vstoreN()
      //
    }

  //
  // return remaining block ids back to the pool
  //
  skc_uint const blocks_rem = SKC_RASTERIZE_BLOCK_ID_V_SIZE - blocks_next;

  if (blocks_rem > 0)
    {
      skc_uint bp_idx = 0;

      if (skc_subgroup_lane() == 0)
        {
          bp_idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(bp_atomics+SKC_BP_ATOMIC_OFFSET_WRITES,blocks_rem);

#if 0
          printf("r-: %8u + %u\n",bp_idx,blocks_rem);
#endif
        }

      bp_idx = (sub_group_broadcast(bp_idx,0) + skc_subgroup_lane() - blocks_next) & bp_mask;

      if (skc_subgroup_lane() >= blocks_next)
        {
          bp_ids[bp_idx] = *blocks;
        }
    }

  //
  // flush work-in-progress ryx keys
  //
  if (sk_v_next > 0)
    {
      skc_uint sk_idx = 0;

      if (skc_subgroup_lane() == 0)
        {
          sk_idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE
            (cohort_atomics+SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS,sk_v_next);
#if 0
          printf("* %u\n",sk_idx);
#endif
        }

      sk_idx = sub_group_broadcast(sk_idx,0) + skc_subgroup_lane();

      if (skc_subgroup_lane() < sk_v_next)
        {
          sk_extent[sk_idx] = *sk_v;
        }
    }
}

//
// If there are lanes that were unable to append to a bin because
// their hashes collided with a bin's current ryx key then those bins
// must be ejected.
//
// Note that we do not eject "full" bins because lazily waiting for a
// collision results in simpler code.
//

static
void
skc_flush(__global SKC_ATOMIC_UINT          volatile * const bp_atomics,
          __global union skc_bp_elem                 * const bp_elems,
          __global uint                              * const bp_ids,
          skc_uint                                     const bp_mask,
          __global SKC_ATOMIC_UINT          volatile * const cohort_atomics,
          skc_block_id_t                             * const subblocks,
          skc_block_id_v_t                           * const blocks,
          skc_uint                                   * const blocks_next,
          skc_ttsk_v_t                               * const sk_v,
          skc_uint                                   * const sk_v_next,
          __global skc_ttsk_s_t                      * const sk_extent,
          __local  struct skc_subgroup_smem volatile * const smem,
          SKC_RASTERIZE_UINT                           const hash,
          SKC_RASTERIZE_UINT                           const yx,
          SKC_RASTERIZE_PREDICATE                            is_collision) // pass by value
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //

  //
  // FIXME -- this code is now stale with the changes to the
  // subblock/block allocation strategy
  //

  //
  // get local TTSB ID queue count
  //
  skc_uint ttsb_id_count  = smem->pool.count; // scalar

  // init hash bit mask
  skc_uint component_mask = 0;

  for (int cc=0; cc<SKC_RASTERIZE_VECTOR_SIZE; cc++)
    {
      // if no collision continue
      if (((int*)&is_collision)[cc] == 0)
        continue;

      uint const winner        = ((uint*)&hash)[cc];
      uint const component_bit = 1u << winner;

      // if already processed this hash then continue
      if (component_mask & component_bit)
        continue;

      // update component mask
      component_mask |= component_bit;

      //
      // new winner requires ejecting the old TTSB
      //
      if (smem->bin.aN.count[winner] > 0)
        {
          skc_uint const elem_idx = smem->bin.aN.id[winner] * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane();

          bp_elems[elem_idx].u32 = smem->bin.aN.ttsb[winner][skc_subgroup_lane()];
        }

        //
        // ensure there is at least one TTSK and TTSB ID
        //
        if (ttsb_id_count == SKC_RASTERIZE_POOL_SIZE)
          {
            //
            // update remaining count
            //
            ttsb_id_count = 0;

            //
            // flush accumulated ttsk_ryx keys
            //
            uint const idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE
              (cohort_atomics+SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS,SKC_RASTERIZE_POOL_SIZE); // ttsk_ryx_count

#if 0
            printf("# %u\n",idx);
#endif

            for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
              {
                ttsk_ryx[idx + ii] = skc_make_ttsk_ryx(smem,SKC_CMD_RASTERIZE_GET_COHORT(cmd),ii);
              }

            //
            // allocate more ttsb ids from pool
            //
            uint const id = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(bp_atomics+0,SKC_RASTERIZE_POOL_SIZE); // ring_reads

            for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
              smem->pool.aN.id[ii] = bp_ids[id + ii];
          }

      //
      // invalidate the winning block
      //

      //
      // update bin with winning yx, new ttsb id and zero count
      //
      // all lanes are loading/storing from/to the same index
      //
      smem->bin.vN.ttsb [winner] = ( SKC_TTS_INVALID );
      smem->bin.aN.id   [winner] = smem->pool.aN.id[ttsb_id_count];
      smem->bin.aN.yx   [winner] = smem->pool.aN.yx[ttsb_id_count] = ((uint*)&yx)[cc];
      smem->bin.aN.count[winner] = 0;

      //
      // update count
      //
      ttsb_id_count += 1;
    }

  //
  // save count
  //
  smem->pool.count = ttsb_id_count;

#else
  //
  // SIMT
  //

  do {
    //
    // only one lane will win!
    //
    if (is_collision)
      smem->subgroup.winner = hash;

    barrier(CLK_LOCAL_MEM_FENCE);

    //
    // which bin is being ejected?
    //
    skc_uint const winner = smem->subgroup.winner;

    //
    // which colliding hash is taking over the bin?
    //
    SKC_RASTERIZE_PREDICATE const is_winner = is_collision && (hash == winner);

    //
    // all lanes with the same hash will try to store but only one
    // lane will win
    //
    if (is_winner)
      smem->subgroup.winner = yx;

    barrier(CLK_LOCAL_MEM_FENCE);

    //
    // flush this block to the pool
    //
    if (smem->bin.aN.count[winner] > 0)
      {
        skc_block_id_v_t const id  = smem->bin.aN.id[winner];
        skc_uint         const idx = id * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane();
        skc_uint         const tts = smem->bin.aN.ttsb[winner][skc_subgroup_lane()];
#if 0
        printf("%08X : [ %10u = %10u : %08X ]\n",yx,id,idx,tts);
#endif
        bp_elems[idx].u32 = tts;
      }

    //
    // append new ttsk
    //
    skc_uint       const new_yx = smem->subgroup.winner;
    skc_block_id_t const new_id = skc_ttsk_v_append(SKC_SUBBLOCKS_BLOCKS_ARGS(),
                                                    blocks_next,
                                                    bp_atomics,
                                                    bp_mask, // pow2 modulo mask for block pool ring
                                                    bp_ids,
                                                    cohort_atomics,
                                                    sk_v,
                                                    sk_v_next,
                                                    sk_extent,
                                                    new_yx);

#if 0
    if (get_sub_group_local_id() == 0) {
      printf(">>> %9u\n",new_id);
    }
#endif

    //
    // update bin with winning yx, new ttsb id and zero count
    //
    smem->bin.aN.ttsb [winner][skc_subgroup_lane()] = SKC_TTS_INVALID;
    smem->bin.aN.yx   [winner]                      = new_yx;
    smem->bin.aN.id   [winner]                      = new_id;
    smem->bin.aN.count[winner]                      = 0;

    //
    // remove all lanes matching this hash
    //
    is_collision = is_collision && !is_winner;

    //
    // exit if nothing left to do
    //
  } while (sub_group_any(is_collision));

#endif
}

//
// scatter scan max
//
static
SKC_RASTERIZE_UINT
skc_scatter_scan_max(__local struct skc_subgroup_smem volatile * const smem,
                     SKC_RASTERIZE_FLOAT                         const iss,
                     SKC_RASTERIZE_FLOAT                         const ess)
{
  //
  // prefix sums determine which lanes we're going to work on next
  //
  SKC_RASTERIZE_PREDICATE const is_scratch_store = (iss > 0.0f) && (ess < (float)SKC_RASTERIZE_ELEMS_PER_SUBGROUP);
  SKC_RASTERIZE_UINT      const scratch_idx      = SKC_CONVERT(SKC_RASTERIZE_UINT)(max(ess,0.0f));

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#ifdef SKC_RASTERIZE_SIMD_USES_SMEM
  //
  // SIMD APPROACH 1: SIMT'ISH
  //

  // zero the volatile smem scratchpad using vector syntax
  smem->subgroup.vN.scratch[0] = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                         \
  if (is_scratch_store C)                               \
    smem->subgroup.aN.scratch[scratch_idx C] = I;

  SKC_RASTERIZE_VECTOR_EXPAND();

  // propagate lanes to right using max scan
  SKC_RASTERIZE_UINT const scratch = smem->subgroup.vN.scratch[0];
  SKC_RASTERIZE_UINT const source  = skc_subgroup_scan_inclusive_max(scratch);

#else
  //
  // SIMD APPROACH 2: SCALAR'ISH
  //

  SKC_RASTERIZE_UINT source = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                 \
  if (is_scratch_store C)                       \
    ((uint *)&source)[scratch_idx C] = I;

  SKC_RASTERIZE_VECTOR_EXPAND();

  for (uint ii=1; ii<SKC_RASTERIZE_ELEMS_PER_SUBGROUP; ii++)
    ((uint *)&source)[ii] = max(((uint *)&source)[ii-1],((uint *)&source)[ii]);
#endif

#else
  //
  // SIMT
  //

  //
  // zero the volatile smem scratchpad using vector syntax
  //
  smem->subgroup.vN.scratch[skc_subgroup_lane()] = ( 0 );

  //
  // store source lane at starting lane
  //
  if (is_scratch_store)
    smem->subgroup.aN.scratch[scratch_idx] = skc_subgroup_lane();

  //
  // propagate lanes to right using max scan
  //
  SKC_RASTERIZE_UINT const scratch = smem->subgroup.vN.scratch[skc_subgroup_lane()];
  SKC_RASTERIZE_UINT const source  = skc_subgroup_scan_inclusive_max(scratch);
#endif

  return source;
}

//
// sliver lines into subpixels
//

static
void
skc_sliver(__global SKC_ATOMIC_UINT          volatile * const bp_atomics,
           __global union skc_bp_elem                 * const bp_elems,
           __global uint                              * const bp_ids,
           skc_uint                                     const bp_mask,
           __global SKC_ATOMIC_UINT          volatile * const cohort_atomics,
           skc_block_id_t                             * const subblocks,
           skc_block_id_v_t                           * const blocks,
           skc_uint                                   * const blocks_next,
           skc_ttsk_v_t                               * const sk_v,
           skc_uint                                   * const sk_v_next,
           __global skc_ttsk_s_t                      * const sk_extent,
           __local  struct skc_subgroup_smem volatile * const smem,
           SKC_RASTERIZE_FLOAT                          const l0x,
           SKC_RASTERIZE_FLOAT                          const l0y,
           SKC_RASTERIZE_FLOAT                          const l1x,
           SKC_RASTERIZE_FLOAT                          const l1y)
{
  //
  // Y-SLIVERING
  // -----------
  //
  // immediately sliver all multi-pixel lines in into 1-pixel high
  // lines
  //
  // note this implicitly squelches horizontal lines
  //
  // there is another test for horizontal lines after x-slivering
  // is complete
  //

  //
  // will we need to flip the sign of y_delta ?
  //
  SKC_RASTERIZE_PREDICATE const y_lt   = (l0y <= l1y);
  SKC_RASTERIZE_UINT      const dy_xor = y_lt ? 0 : 0x80000000;

  //
  // save 1/dy
  //
  SKC_RASTERIZE_FLOAT const y_denom = native_recip(l1y - l0y);

  //
  // how many non-horizontal subpixel y-axis slivers are there?
  //
  SKC_RASTERIZE_FLOAT const y_min   = floor(fmin(l0y,l1y) * SKC_SUBPIXEL_Y_SCALE_DOWN);
  SKC_RASTERIZE_FLOAT const y_max   = ceil (fmax(l0y,l1y) * SKC_SUBPIXEL_Y_SCALE_DOWN);
  SKC_RASTERIZE_FLOAT const y_base  = y_lt ? y_min : y_max;
  SKC_RASTERIZE_FLOAT       y_segs  = y_max - y_min;

  //
  // inclusive subgroup scan of y_segs
  //
  SKC_RASTERIZE_FLOAT       y_iss   = skc_subgroup_scan_inclusive_add_float(y_segs);
  SKC_RASTERIZE_FLOAT       y_ess   = y_iss - y_segs;
  float                     y_rem   = skc_subgroup_last_float(y_iss);

  //
  // if this is a horizontal line then tweak y_iss so "is_scratch_store" always fails
  //
  if (y_segs == 0.0f)
    y_iss = 0.0f;

#if 0
  printf("{ { %5.0f, %5.0f }, { %5.0f, %5.0f } (* %5.0f / %5.0f / %5.0f / %5.0f *) }, \n",a0x,a0y,a1x,a1y,y_segs,y_iss,y_ess,y_rem);
#endif

  //
  // these values don't matter on first iteration
  //
  SKC_RASTERIZE_FLOAT n1x_prev = 0;
  SKC_RASTERIZE_FLOAT n1y_prev = 0;

  //
  // loop until done
  //
  while (y_rem > 0.0f)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const y_source = skc_scatter_scan_max(smem,y_iss,y_ess);

      //
      // get line at y_source line
      //
      SKC_RASTERIZE_FLOAT const m0x = skc_subgroup_shuffle(l0x,y_source);
      SKC_RASTERIZE_FLOAT const m0y = skc_subgroup_shuffle(l0y,y_source);
      SKC_RASTERIZE_FLOAT const m1x = skc_subgroup_shuffle(l1x,y_source);
      SKC_RASTERIZE_FLOAT const m1y = skc_subgroup_shuffle(l1y,y_source);

      //
      // every lane will create a 1 pixel tall line "sliver"
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if numerator == 1 then this is the first lane
      // if numerator == s then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const y_delta    = skc_delta_offset() - skc_subgroup_shuffle(y_ess,y_source);
      SKC_RASTERIZE_FLOAT     const y_count    = skc_subgroup_shuffle(y_segs,y_source);

      SKC_RASTERIZE_PREDICATE const is_y_first = (y_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_y_last  = (y_delta >= y_count);

      // toggle y_delta sign
      SKC_RASTERIZE_FLOAT     const y_offset   = as_float((as_uint(y_delta) ^ intel_sub_group_shuffle(dy_xor,y_source)));

      //
      // calculate "right" line segment endpoint
      //
      SKC_RASTERIZE_FLOAT       n1y = (y_offset + skc_subgroup_shuffle(y_base,y_source)) * SKC_SUBPIXEL_Y_SCALE_UP;
      SKC_RASTERIZE_FLOAT const n_t = (n1y - m0y) * skc_subgroup_shuffle(y_denom,y_source);
      SKC_RASTERIZE_FLOAT       n1x = round(SKC_LERP(m0x,m1x,n_t));

      //
      // override c1 if this is last point
      //
      n1y = select(n1y,m1y,is_y_last);
      n1x = select(n1x,m1x,is_y_last);

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT n0y = skc_subgroup_shuffle_up_1(n1y_prev,n1y);
      SKC_RASTERIZE_FLOAT n0x = skc_subgroup_shuffle_up_1(n1x_prev,n1x);

      //
      // override shuffle up if this is the first line segment
      //
      n0y = select(n0y,m0y,is_y_first);
      n0x = select(n0x,m0x,is_y_first);

      //
      // save previous right endpoint
      //
      n1x_prev = n1x;
      n1y_prev = n1y;

      //
      // decrement by subgroup size
      //
      y_iss -= (float)SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      y_ess -= (float)SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      y_rem -= (float)SKC_RASTERIZE_ELEMS_PER_SUBGROUP;

#if 0
      //
      // debug
      //
      if (n0y != n1y) {
        printf("{ { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",n0x,n0y,n1x,n1y);
      }
#endif

      //
      // X-SLIVERING
      // -----------
      //
      // now sliver 1-pixel high lines into at either vertical or
      // 1-pixel wide lines
      //
      // save original direction and work with increasing x
      //
      SKC_RASTERIZE_PREDICATE const x_lt   = (n0x <= n1x);
      SKC_RASTERIZE_UINT      const dx_xor = x_lt ? 0 : 0x80000000;

      //
      // save 1/dy
      //
      SKC_RASTERIZE_FLOAT const x_denom  = native_recip(n1x - n0x);

      //
      // how many non-horizontal subpixel y-axis slivers are there?
      //
      SKC_RASTERIZE_FLOAT const x_min    = floor(fmin(n0x,n1x) * SKC_SUBPIXEL_X_SCALE_DOWN);
      SKC_RASTERIZE_FLOAT const x_max    = ceil (fmax(n0x,n1x) * SKC_SUBPIXEL_X_SCALE_DOWN);
      SKC_RASTERIZE_FLOAT const x_base   = x_lt ? x_min : x_max;
      SKC_RASTERIZE_FLOAT const x_segs   = fmax(x_max - x_min,1.0f);

      //
      // inclusive subgroup scan of y_segs
      //
      SKC_RASTERIZE_FLOAT       x_iss    = skc_subgroup_scan_inclusive_add_float(x_segs);
      SKC_RASTERIZE_FLOAT       x_ess    = x_iss - x_segs;
      float                     x_rem    = skc_subgroup_last_float(x_iss);

      //
      // if this is a horizontal line then tweak x_iss so "is_scratch_store" always fails
      //
      //if (x_segs == 0.0f)
      // x_iss = 0.0f;

      //
      // these values don't matter on first iteration
      //
      SKC_RASTERIZE_FLOAT       p1x_prev = 0;
      SKC_RASTERIZE_FLOAT       p1y_prev = 0;

      //
      // loop until done
      //
      while (x_rem > 0)
        {
          //
          // distribute work across lanes
          //
          SKC_RASTERIZE_UINT const x_source = skc_scatter_scan_max(smem,x_iss,x_ess);

          //
          // get line at y_source line
          //
          SKC_RASTERIZE_FLOAT const o0x = skc_subgroup_shuffle(n0x,x_source);
          SKC_RASTERIZE_FLOAT const o0y = skc_subgroup_shuffle(n0y,x_source);
          SKC_RASTERIZE_FLOAT const o1x = skc_subgroup_shuffle(n1x,x_source);
          SKC_RASTERIZE_FLOAT const o1y = skc_subgroup_shuffle(n1y,x_source);

          //
          // every lane will create a 1 pixel tall line "sliver"
          //
          // FIXME -- this gets expanded on SIMD
          //
          // if numerator == 1 then this is the first lane
          // if numerator == s then this is the last  lane
          //
          SKC_RASTERIZE_FLOAT     const x_delta    = skc_delta_offset() - skc_subgroup_shuffle(x_ess,x_source);
          SKC_RASTERIZE_FLOAT     const x_count    = skc_subgroup_shuffle(x_segs,x_source);

          SKC_RASTERIZE_PREDICATE const is_x_first = (x_delta == 1.0f);
          SKC_RASTERIZE_PREDICATE const is_x_last  = (x_delta >= x_count);

          // toggle x_delta sign
          SKC_RASTERIZE_FLOAT     const x_offset   = as_float((as_uint(x_delta) ^ intel_sub_group_shuffle(dx_xor,x_source)));

          //
          // calculate "right" line segment endpoint
          //
          SKC_RASTERIZE_FLOAT       p1x = (x_offset + skc_subgroup_shuffle(x_base,x_source)) * SKC_SUBPIXEL_X_SCALE_UP;
          SKC_RASTERIZE_FLOAT const p_t = (p1x - o0x) * skc_subgroup_shuffle(x_denom,x_source);
          SKC_RASTERIZE_FLOAT       p1y = round(SKC_LERP(o0y,o1y,p_t));

          //
          // override c1 if this is last point
          //
          p1x = select(p1x,o1x,is_x_last);
          p1y = select(p1y,o1y,is_x_last);

          //
          // shuffle up "left" line segment endpoint
          //
          // NOTE: Intel's shuffle_up is unique with its elegant
          // "previous" argument so don't get used to it
          //
          SKC_RASTERIZE_FLOAT p0x = skc_subgroup_shuffle_up_1(p1x_prev,p1x);
          SKC_RASTERIZE_FLOAT p0y = skc_subgroup_shuffle_up_1(p1y_prev,p1y);

          //
          // override shuffle up if this is the first line segment
          //
          p0x = select(p0x,o0x,is_x_first);
          p0y = select(p0y,o0y,is_x_first);

          //
          // save previous right endpoint
          //
          p1x_prev = p1x;
          p1y_prev = p1y;

          //
          // decrement by subgroup size
          //
          x_iss -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
          x_ess -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
          x_rem -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;

          //
          // only non-horizontal subpixel lines are valid
          //
          SKC_RASTERIZE_PREDICATE is_active = (p0y != p1y);

          //
          // if no lanes are active then continue
          //
          // FIXME -- THIS SIMPLE SUB_GROUP_ANY TEST SIGNIFICANTLY
          // IMPACTS PERFORMANCE (+12% ?)
          //
          // IT SHOULDN'T !!!
          //
#if 0
          if (!skc_subgroup_any(is_active))
            continue;
#endif

          //
          // Option 1: use SLM for explicitly managed coalesced stores
          //
          // 1. which tile does this line belong?
          // 2. hash tile coordinates
          // 3. lookup hash
          // 4. if tile matches then SLM append keys
          // 5. if tile doesn't match
          //   a. flush
          //   b. create new TTSK_RYX
          //   c. obtain TTSB block from pool
          //   d. goto 3.
          //

          //
          // Option 2: rely on L1/L2/L3 to mitigate non-coalesced stores
          //
          // 1. which tile does this line belong?
          // 2. hash tile coordinates
          // 3. lookup hash
          // 4. if tile matches then GMEM append keys
          // 5. if tile doesn't match
          //   a. flush (and invalidate empty elems)
          //   b. create new TTSK_RYX
          //   c. obtain TTSB block from pool
          //   d. goto 3.
          //

          //
          // The virtual rasterization surface is very large and
          // signed: +/- ~64K-256K, depending on the architecture.
          //
          // Rasters must be clipped to the virtual surface and,
          // optionally, clipped even further on a per raster
          // basis.
          //

          //
          // Clip to the per-raster clip
          //

          /*

            CLIP HERE

          */

          //
          // Hash the tile coordinates
          //
          // This table lists nominal values for each architecture.
          // We want to choose values that are naturally fit the
          // "width" of the architecture.
          //
          //   SIMD   RANGE   BITS  MAX RANGE  MAX BINS  HASH BITS
          //   ----  -------  ----  ---------  --------  ---------
          //     4   [0,  4]    3    [0,  7]      10      mod(10)  <-- SSE42, ?
          //     8   [0,  8]    4    [0, 15]       8         3     <-- GEN*,AVX*
          //    16   [0, 16]    5    [0, 31]       6      mod(6)   <-- GEN*,?
          //    32   [0, 32]    6    [0, 63]       5      mod(5)   <-- CUDA,PowerVR,Adreno,GEN*
          //    64   [0, 64]    7    [0,127]       4         2     <-- AMD Radeon
          //
          // NOTE: When possible, bias the hash toward using more y
          // bits because of:
          //
          //   1. the 90 degree counter-clockwise rotation that we put
          //      in place to offset the render-time clockwise
          //      rotation
          //
          //   2. the likely presence of left-to-right or
          //      right-to-left glyphs.
          //
          // For power-of-two bins, the hash is easy.
          //
          // For non-power-of-two, we may want to either implement a
          // fast mod (compiler should do this for us... hahahaha) or
          // drop down to the next power-of-two.
          //

          //
          // FIXME -- this snarl is not good -- can probably reduce
          // some of the sign casting but some is there to vectorize a
          // scalar
          //
          SKC_RASTERIZE_INT       const z0y    = SKC_CONVERT(SKC_RASTERIZE_INT)(p0y);
          SKC_RASTERIZE_INT       const z1y    = SKC_CONVERT(SKC_RASTERIZE_INT)(p1y);

          SKC_RASTERIZE_INT       const z0x    = SKC_CONVERT(SKC_RASTERIZE_INT)(p0x);
          SKC_RASTERIZE_INT       const z1x    = SKC_CONVERT(SKC_RASTERIZE_INT)(p1x);

          SKC_RASTERIZE_INT       const min_y  = min(z0y,z1y);
          SKC_RASTERIZE_INT       const max_y  = max(z0y,z1y);

          SKC_RASTERIZE_INT       const tile_y = min_y >> SKC_SUBTILE_RESL_Y_LOG2;

          SKC_RASTERIZE_UINT      const ty     = SKC_AS(SKC_RASTERIZE_UINT)(min_y) & SKC_SUBTILE_MASK_Y;
          SKC_RASTERIZE_INT             dy     = SKC_AS(SKC_RASTERIZE_INT)(z1y - z0y);

          //
          // map [+1,+32] to [ 0,+31]
          // map [-1,-32] to [-1,-32]
          //
          SKC_RASTERIZE_INT             dys    = (dy + (~dy >> 31)) << 26;

          SKC_RASTERIZE_INT       const min_x  = min(z0x,z1x);
          SKC_RASTERIZE_INT       const max_x  = max(z0x,z1x);
          SKC_RASTERIZE_INT       const tile_x = min_x >> SKC_SUBTILE_RESL_X_LOG2;

          SKC_RASTERIZE_UINT      const tx     = SKC_AS(SKC_RASTERIZE_UINT)(min_x) & SKC_SUBTILE_MASK_X;
          SKC_RASTERIZE_UINT      const sx     = SKC_AS(SKC_RASTERIZE_UINT)(max_x - min_x);

          SKC_RASTERIZE_UINT      const tts    = dys | (ty << 16) | (sx << 10) | tx;

          SKC_RASTERIZE_UINT      const hash   = (((SKC_AS(SKC_RASTERIZE_UINT)(tile_y) & SKC_RASTERIZE_TILE_HASH_Y_MASK) << SKC_RASTERIZE_TILE_HASH_X_BITS) |
                                                   (SKC_AS(SKC_RASTERIZE_UINT)(tile_x) & SKC_RASTERIZE_TILE_HASH_X_MASK));

          SKC_RASTERIZE_UINT      const yx     = (((SKC_AS(SKC_RASTERIZE_UINT)(tile_y) & 0xFFF) << 12) | (SKC_AS(SKC_RASTERIZE_UINT)(tile_x) & 0xFFF));

#if 0
          printf("(%3u, %3u)\n",tile_y,tile_x);
#endif

#if 0
          if (is_active)
            printf("( %3u, %3u ) : [ %3u, %3u, %3d, %3d, %3u ]\n",tile_y,tile_x,ty,tx,dy,((int)dys)>>26,sx);
#endif

          //
          // debug
          //
#if 0 // PRINTF_ENABLE

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                         \
          if (is_active C)                                              \
            printf("{ { %5d, %5d }, { %5d, %5d } (* %2u *) },\n",z0x C,z0y C,z1x C,z1y C,hash C);

          SKC_RASTERIZE_VECTOR_EXPAND();
#else
          if (is_active)
            printf("{ { %5d, %5d }, { %5d, %5d } } (* %2u *),\n",z0x,z0y,z1x,z1y,hash);
#endif

#endif
          //
          // flush all active lanes
          //
          while (true)
            {
              //
              // either gather load or vector load+shuffle the yx keys
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
              SKC_RASTERIZE_BIN       const yx_bin     = smem->bin.vN.yx;
              SKC_RASTERIZE_UINT      const yx_cur     = shuffle(yx_bin,hash);
#else
              SKC_RASTERIZE_UINT      const yx_cur     = smem->bin.aN.yx[hash];
#endif

              //
              // does yx for lane match yx for hash?
              //
              SKC_RASTERIZE_UINT      const active_yx  = is_active ? yx : SKC_RASTERIZE_YX_INVALID;
              SKC_RASTERIZE_PREDICATE const is_match   = (yx_cur == active_yx);

              //
              // OpenCL spec: "When casting a bool to a vector integer
              // data type, the vector components will be set to -1
              // (i.e. all bits set) if the vector bool value is true
              // and 0 otherwise.
              //
#if ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
              SKC_RASTERIZE_UINT      const h_match    = (SKC_RASTERIZE_UINT)is_match;
#else
              SKC_RASTERIZE_UINT      const h_match    = abs(is_match); // {-1,0} -> {+1,0}
#endif
              //
              // how many new elements for each matching hash bin?
              //
              SKC_RASTERIZE_UINT      const h_shl      = hash * SKC_RASTERIZE_TILE_HASH_BIN_BITS;
              SKC_RASTERIZE_UINT      const h          = h_match << h_shl;

              //
              // prefix sum all of the bins in parallel
              //
              SKC_RASTERIZE_UINT      const h_iss      = skc_subgroup_scan_inclusive_add_uint(h);
              SKC_RASTERIZE_UINT      const h_total    = skc_subgroup_last_uint(h_iss);

              //
              // current bin counts
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
              SKC_RASTERIZE_BIN       const count_bin  = smem->bin.vN.count;
              SKC_RASTERIZE_UINT      const count_cur  = shuffle(count_bin,hash);
#else
              SKC_RASTERIZE_UINT      const count_cur  = smem->bin.aN.count[hash];
#endif

              //
              // calculate where each cache-hit and in-bounds tts should be stored
              //
              SKC_RASTERIZE_UINT      const ttsb_index = (h_iss   >> h_shl & SKC_RASTERIZE_TILE_HASH_BIN_MASK) + count_cur - 1;
              SKC_RASTERIZE_UINT      const count_new  = (h_total >> h_shl & SKC_RASTERIZE_TILE_HASH_BIN_MASK) + count_cur;

              //
              // which lanes can append to a matching bin?
              //
              SKC_RASTERIZE_PREDICATE const is_append  = is_match && (ttsb_index < SKC_DEVICE_SUBBLOCK_WORDS);

              //
              // scatter append tts elements to bin blocks
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
              //
              // SIMD
              //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                         \
              if (is_append C)                                          \
                {                                                       \
                  smem->bin.aN.ttsb [hash C][ttsb_index C] = tts       C; \
                  smem->bin.aN.count[hash C]               = count_new C; \
                }

              SKC_RASTERIZE_VECTOR_EXPAND();
#else
              //
              // SIMT
              //
              if (is_append)
                {
                  smem->bin.aN.ttsb [hash][ttsb_index] = tts;
                  smem->bin.aN.count[hash]             = count_new; // it's ok if this is > SKC_DEVICE_SUBBLOCK_WORDS
                }
#endif
              //
              // try to keep predicate updates SIMD-friendly and
              // outside of predicated code paths -- this is not
              // always how we would normally do things on SIMT but
              // either approach is acceptable
              //

              //
              // mask off lanes/components that successfully appended
              //
              is_active = is_active && !is_append;

              //
              // are there any active lanes left?
              //
              if (!skc_subgroup_any(is_active))
                break;

              //
              // There are active lanes that couldn't be appended to a
              // bin because their hashes collided with the bin's
              // current ryx key then those bins must be ejected.
              //
              // Note that we do not eject "full" bins because lazily
              // waiting for a collision results in simpler code.
              //
              skc_flush(bp_atomics,
                        bp_elems,
                        bp_ids,
                        bp_mask,
                        cohort_atomics,
                        subblocks,
                        blocks,
                        blocks_next,
                        sk_v,
                        sk_v_next,
                        sk_extent,
                        smem,
                        hash,
                        yx,
                        is_active);
            }
        }
    }
}

//
// INITIALIZE SMEM
//
// Note that SIMD/SIMT have nearly the same syntax.
//
static
void
skc_smem_init(__local struct skc_subgroup_smem volatile * const smem)
{
  //
  // initialize smem bins
  //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  smem->bin.vN.yx    = ( SKC_RASTERIZE_YX_INIT );
  smem->bin.vN.count = ( 0 );
#else
  //
  // SIMT
  //
  int idx = skc_subgroup_lane();

#if   ( SKC_RASTERIZE_TILE_HASH_BIN_COUNT < SKC_RASTERIZE_ELEMS_PER_SUBGROUP )
  if (idx < SKC_RASTERIZE_TILE_HASH_BIN_COUNT)
#elif ( SKC_RASTERIZE_TILE_HASH_BIN_COUNT > SKC_RASTERIZE_ELEMS_PER_SUBGROUP )
  for (; idx<SKC_RASTERIZE_TILE_HASH_BIN_COUNT; idx+=SKC_RASTERIZE_SUBGROUP_SIZE)
#endif
    {
      smem->bin.aN.yx   [idx] = ( SKC_RASTERIZE_YX_INIT );
      smem->bin.aN.count[idx] = ( 0 );
    }
#endif
}

//
// RASTERIZE CUBIC KERNEL
//

static
void
skc_rasterize_cubics(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                     __global union skc_bp_elem                * const bp_elems,
                     __global uint                             * const bp_ids,
                     skc_uint                                    const bp_mask,

                     __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                     __global skc_ttsk_s_t                     * const sk_extent,

                     __local struct skc_subgroup_smem volatile * const smem,

                     skc_uint                                  * const nodeword,
                     skc_block_id_t                            * const id,

                     union skc_transform              const    * const tv,
                     union skc_path_clip              const    * const cv,
                     skc_uint                                    const cohort)
{
  //
  // the initial segment idx and segments-per-block constant determine
  // how many block ids will need to be loaded
  //
  SKC_RASTERIZE_FLOAT const c0x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c0y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c2x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c2y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c3x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c3y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only support perspective later
  //
  // the affine transformation requires 8 FMA + 2 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const b0x = round(c0x * tv->sx  + c0y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const b0y = round(c0x * tv->shy + c0y * tv->sy  + tv->ty);

  SKC_RASTERIZE_FLOAT const t1x = c1x * tv->sx  + c1y * tv->shx + tv->tx;
  SKC_RASTERIZE_FLOAT const t1y = c1x * tv->shy + c1y * tv->sy  + tv->ty;

  SKC_RASTERIZE_FLOAT const t2x = c2x * tv->sx  + c2y * tv->shx + tv->tx;
  SKC_RASTERIZE_FLOAT const t2y = c2x * tv->shy + c2y * tv->sy  + tv->ty;

  SKC_RASTERIZE_FLOAT const t3x = round(c3x * tv->sx  + c3y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const t3y = round(c3x * tv->shy + c3y * tv->sy  + tv->ty);

  //
  //
  //
#if PRINTF_ENABLE

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                         \
  printf("{ { %.02f, %.02f }, { %.02f, %.02f },"                        \
         "  { %.02f, %.02f }, { %.02f, %.02f } },\n",                   \
         b0x C,b0y C,t1x C,t1y C,                                       \
         t2x C,t2y C,t3x C,t3y C);

  SKC_RASTERIZE_VECTOR_EXPAND();

#else

  printf("{ { %.02f, %.02f }, { %.02f, %.02f }, { %.02f, %.02f }, { %.02f, %.02f } },\n",
         b0x,b0y,t1x,t1y,t2x,t2y,t3x,t3y);

#endif

#endif

  //
  // OLD APPROACH
  // ------------
  //
  // The Spinel CUDA rasterizer was significantly more complex and
  // performed a few different tasks that are probably best kept
  // separate.
  //
  // The Spinel rasterizer Bezier held 4-element x and y coordinates
  // in adjacent lanes. This simplified intermingling of single lane
  // 4-coordinate line segments with two-lane cubic Beziers.
  //
  // After transformation of the input segments, the Spinel rasterizer
  // would test cubics for flatness and, if flat, collapse the
  // adjacent lanes into a single line lane and an empty lane.
  //
  // Any lines would then be appended to a line queue.
  //
  // Any cubics would then be subdivided.
  //
  // The reclassification process would be repeated.
  //
  // NEW APPROACH
  // ------------
  //
  // Assume we're only working with cubics in this kernel.
  //
  // Optimization: if the line segment is a special case -- a cusp,
  // has 1+ inflections, or a loop -- it might be beneficial to
  // subdivide the control cage 1+ times in order to separate the
  // flatter segments the high-velocity region(s).
  //
  // This means we want to split using [a,b] formulation to _directly_
  // subdivide producing a new control cage.
  //
  // Wang's Formula is still useful even if we subdivide once or twice
  // as it's so cheap that it might give some useful hints about where
  // the high-velocity sections of curve reside.
  //
  // But it seems like using Wang's and directly flattening to line
  // segments without any subdivision is good enough for the limited
  // set of test cases that I've tried.
  //
  // So... use Wang's Formula to estimate how many line segment are
  // required to properly flatten the cubics.
  //
  // Then use inclusive/exclusive scans to put all the lanes to work:
  //
  //   1. segmenting cubics to line segments
  //
  //   2. slivering line segments into 1-pixel high line segments
  //
  //   3. slivering 1-pixel high line segments into 1-pixel wide line
  //      segments
  //
  // MORE BACKGROUND ON NEW APPROACH
  // -------------------------------
  //
  // Two options for handling line segments:
  //
  // 1. append the line segments onto an SLM array until enough
  //    work has been accrued (Spinel does this)
  //
  // 2. immediately sliver the potentially multi-pixel line
  //    segments into subpixel lines
  //
  // The advantage of (1) is that it guarantees the slivering
  // process will, on average, always be emitting a full subgroup
  // of subpixel lines.
  //
  // The advantage of (2) is that it reduces code complexity and
  // leaves more room for SLM tile bins. The difference between Spinel
  // and Skia Compute is that Wang's Formula guarantees there will be
  // a full subgroup of multi-pixel lines unless this is the final
  // iteration of the warp of multi-pixel lines.
  //
  // Note that wider GPU architectures might benefit from (1) and
  // other work accumulation strategies because it will minimize
  // partial warp workloads in the final iteration of each stage.  It
  // also minimizes the sunk cost of the uniform control logic steps.
  //
  // So let's implement (2) for now...
  //

  //
  // And... begin!
  //
  // Estimate how many line segments are in quad/cubic curve.
  //
  // Wang's Formula will return zero if the control points are
  // collinear but we bump it up to 1.0f.
  //
  SKC_RASTERIZE_FLOAT const s_segs  = skc_wangs_formula_cubic(b0x,b0y,t1x,t1y,t2x,t2y,t3x,t3y);

  //
  // if there are free registers then precalculate the reciprocal for
  // each estimated segments since it will never change
  //
  SKC_RASTERIZE_FLOAT const s_denom = native_recip(s_segs);


  //
  // inclusive add scan of estimated line segments
  // exclusive add scan of estimated line segments
  // total number       of estimated line segments
  //
  SKC_RASTERIZE_FLOAT       s_iss   = skc_subgroup_scan_inclusive_add_float(s_segs);
  SKC_RASTERIZE_FLOAT       s_ess   = s_iss - s_segs;
  float                     s_rem   = skc_subgroup_last_float(s_iss); // scalar

  //
  // Precompute cubic polynomial coefficients from transformed control
  // cage so we can shuffle them in on each iteration of the outer
  // loop and then evaluate the polynomial in Horner form.
  //
  //                            |  1  0  0  0 | | c0 |
  //                            |             | |    |
  //                            | -3  3  0  0 | | c1 |
  //   B(t) = [ 1 t^1 t^2 t^3 ] |             | |    |
  //                            |  3 -6  3  0 | | c2 |
  //                            |             | |    |
  //                            | -1  3 -3  1 | | c3 |
  //
  //
  SKC_RASTERIZE_FLOAT const b1x = mad(-3.0f,b0x,3.0f*t1x);                // 2 - 1 MAD + MUL
  SKC_RASTERIZE_FLOAT const b1y = mad(-3.0f,b0y,3.0f*t1y);                // 2 - 1 MAD + MUL

  SKC_RASTERIZE_FLOAT const b2x = mad(3.0f,b0x,mad(-6.0f,t1x,3.0f*t2x));  // 3 - 2 MAD + MUL
  SKC_RASTERIZE_FLOAT const b2y = mad(3.0f,b0y,mad(-6.0f,t1y,3.0f*t2y));  // 3 - 2 MAD + MUL

  SKC_RASTERIZE_FLOAT const b3x = mad(3.0f,t1x,mad(-3.0f,t2x,t3x)) - b0x; // 3 - 2 MAD + SUB
  SKC_RASTERIZE_FLOAT const b3y = mad(3.0f,t1y,mad(-3.0f,t2y,t3y)) - b0y; // 3 - 2 MAD + SUB

  //
  // these values don't matter on the first iteration
  //
  SKC_RASTERIZE_FLOAT l1x_prev  = 0;
  SKC_RASTERIZE_FLOAT l1y_prev  = 0;

  //
  // allocate and init in-register TTSK keys
  //
  skc_uint     sk_v_next = 0;
  skc_ttsk_v_t sk_v; 

  sk_v.hi = cohort;

  //
  // initialize smem
  //
  skc_smem_init(smem);

  //
  // initialize blocks / subblocks
  //
  skc_block_id_v_t blocks;
  skc_uint         blocks_next = SKC_RASTERIZE_BLOCK_ID_V_SIZE;

#if SKC_DEVICE_BLOCK_WORDS_LOG2 > SKC_DEVICE_SUBBLOCK_WORDS_LOG2
  skc_block_id_t   subblocks   = 0;
#endif

  //
  // loop until done
  //
  while (s_rem > 0)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const s_source = skc_scatter_scan_max(smem,s_iss,s_ess);

      //
      // every lane has a fraction to work off of
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if delta == 1      then this is the first lane
      // if count == s_segs then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const s_delta    = skc_delta_offset() - skc_subgroup_shuffle(s_ess,s_source);
      SKC_RASTERIZE_FLOAT     const s_count    = skc_subgroup_shuffle(s_segs,s_source);

      SKC_RASTERIZE_PREDICATE const is_s_first = (s_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_s_last  = (s_delta >= s_count);

      //
      // init parametric t
      //
      SKC_RASTERIZE_FLOAT s_t = s_delta * skc_subgroup_shuffle(s_denom,s_source); // faster than native_recip(s_count)?

      //
      // if last then override to a hard 1.0f
      //
      s_t    = is_s_last ? 1.0f : s_t;

      //
      // decrement by subgroup size
      //
      s_iss -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      s_ess -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      s_rem -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;

      //
      // now every lane knows what to do and the following lines will
      // pump out up to SUBGROUP_SIZE line segments
      //
      // obtain the src vertices through shared or via a shuffle
      //

      //
      // shuffle in the polynomial coefficients their source lane
      //
      SKC_RASTERIZE_FLOAT const s0x = skc_subgroup_shuffle(b0x,s_source);
      SKC_RASTERIZE_FLOAT const s0y = skc_subgroup_shuffle(b0y,s_source);

      SKC_RASTERIZE_FLOAT const s1x = skc_subgroup_shuffle(b1x,s_source);
      SKC_RASTERIZE_FLOAT const s1y = skc_subgroup_shuffle(b1y,s_source);

      SKC_RASTERIZE_FLOAT const s2x = skc_subgroup_shuffle(b2x,s_source);
      SKC_RASTERIZE_FLOAT const s2y = skc_subgroup_shuffle(b2y,s_source);

      SKC_RASTERIZE_FLOAT const s3x = skc_subgroup_shuffle(b3x,s_source);
      SKC_RASTERIZE_FLOAT const s3y = skc_subgroup_shuffle(b3y,s_source);

      //
      // calculate "right" line segment endpoint using Horner form
      //
      SKC_RASTERIZE_FLOAT       l1x = round(mad(mad(mad(s3x,s_t,s2x),s_t,s1x),s_t,s0x)); // 3 MAD + ROUND
      SKC_RASTERIZE_FLOAT       l1y = round(mad(mad(mad(s3y,s_t,s2y),s_t,s1y),s_t,s0y)); // 3 MAD + ROUND

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT       l0x = skc_subgroup_shuffle_up_1(l1x_prev,l1x);
      SKC_RASTERIZE_FLOAT       l0y = skc_subgroup_shuffle_up_1(l1y_prev,l1y);

      //
      // save previous right endpoint
      //
      l1x_prev = l1x;
      l1y_prev = l1y;

      //
      // override shuffle up if this is the first line segment
      //
      l0x = select(l0x,s0x,is_s_first);
      l0y = select(l0y,s0y,is_s_first);

      //
      // sliver lines
      //
      skc_sliver(bp_atomics,
                 bp_elems,
                 bp_ids,
                 bp_mask,
                 cohort_atomics,
                 &subblocks,
                 &blocks,
                 &blocks_next,
                 &sk_v,
                 &sk_v_next,
                 sk_extent,
                 smem,
                 l0x,l0y,l1x,l1y);
    }

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(bp_atomics,
               bp_elems,
               bp_ids,
               bp_mask,
               cohort_atomics,
               &blocks,
               blocks_next,
               &sk_v,
               sk_v_next,
               sk_extent,
               smem);
}

//
// RASTERIZE QUAD KERNEL
//

static
void
skc_rasterize_quads(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                    __global union skc_bp_elem                * const bp_elems,
                    __global uint                             * const bp_ids,
                    skc_uint                                    const bp_mask,

                    __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                    __global skc_ttsk_s_t                     * const sk_extent,

                    __local struct skc_subgroup_smem volatile * const smem,
                    
                    skc_uint                                  * const nodeword,
                    skc_block_id_t                            * const id,

                    union skc_transform              const    * const tv,
                    union skc_path_clip              const    * const cv,
                    skc_uint                                    const cohort)
{
  //
  // the initial segment idx and segments-per-block constant determine
  // how many block ids will need to be loaded
  //
  SKC_RASTERIZE_FLOAT const c0x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c0y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c2x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c2y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only support perspective later
  //
  // the affine transformation requires 8 FMA + 2 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const b0x = round(c0x * tv->sx  + c0y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const b0y = round(c0x * tv->shy + c0y * tv->sy  + tv->ty);

  SKC_RASTERIZE_FLOAT const t1x = c1x * tv->sx  + c1y * tv->shx + tv->tx;
  SKC_RASTERIZE_FLOAT const t1y = c1x * tv->shy + c1y * tv->sy  + tv->ty;

  SKC_RASTERIZE_FLOAT const t2x = round(c2x * tv->sx  + c2y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const t2y = round(c2x * tv->shy + c2y * tv->sy  + tv->ty);

  //
  // Estimate how many line segments are in quad/cubic curve.
  //
  // Wang's Formula will return zero if the control points are
  // collinear but we bump it up to 1.0f.
  //
  SKC_RASTERIZE_FLOAT const s_segs  = skc_wangs_formula_quadratic(b0x,b0y,t1x,t1y,t2x,t2y);

  //
  // if there are free registers then precalculate the reciprocal for
  // each estimated segments since it will never change
  //
  SKC_RASTERIZE_FLOAT const s_denom = native_recip(s_segs);


  //
  // inclusive add scan of estimated line segments
  // exclusive add scan of estimated line segments
  // total number       of estimated line segments
  //
  SKC_RASTERIZE_FLOAT       s_iss   = skc_subgroup_scan_inclusive_add_float(s_segs);
  SKC_RASTERIZE_FLOAT       s_ess   = s_iss - s_segs;
  float                     s_rem   = skc_subgroup_last_float(s_iss); // scalar

  //
  // Precompute quadratic polynomial coefficients from control cage so
  // we can shuffle them in on each iteration of the outer loop and
  // then evaluate the polynomial in Horner form.
  //

  //                        |  1  0  0  | | c0 |
  //                        |           | |    |
  //   B(t) = [ 1 t^1 t^2 ] | -2  2  0  | | c1 |
  //                        |           | |    |
  //                        |  1 -2  1  | | c2 |
  //
  //
  SKC_RASTERIZE_FLOAT const b1x = mad(-2.0f,b0x,2.0f*t1x); // 2 - 1 MAD + MUL
  SKC_RASTERIZE_FLOAT const b1y = mad(-2.0f,b0y,2.0f*t1y); // 2 - 1 MAD + MUL

  SKC_RASTERIZE_FLOAT const b2x = mad(-2.0f,t1x,b0x+t2x);  // 2 - 1 MAD + ADD
  SKC_RASTERIZE_FLOAT const b2y = mad(-2.0f,t1y,b0y+t2y);  // 2 - 1 MAD + ADD

  //
  // these values don't matter on the first iteration
  //
  SKC_RASTERIZE_FLOAT l1x_prev  = 0;
  SKC_RASTERIZE_FLOAT l1y_prev  = 0;

  //
  // allocate and init in-register TTSK keys
  //
  skc_uint     sk_v_next = 0;
  skc_ttsk_v_t sk_v; 

  sk_v.hi = cohort;

  //
  // initialize smem
  //
  skc_smem_init(smem);

  //
  // initialize blocks / subblocks
  //
  skc_block_id_v_t blocks;
  skc_uint         blocks_next = SKC_RASTERIZE_BLOCK_ID_V_SIZE;

#if SKC_DEVICE_BLOCK_WORDS_LOG2 > SKC_DEVICE_SUBBLOCK_WORDS_LOG2
  skc_block_id_t   subblocks   = 0;
#endif

  //
  // loop until done
  //
  while (s_rem > 0)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const s_source = skc_scatter_scan_max(smem,s_iss,s_ess);

      //
      // every lane has a fraction to work off of
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if delta == 1      then this is the first lane
      // if count == s_segs then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const s_delta    = skc_delta_offset() - skc_subgroup_shuffle(s_ess,s_source);
      SKC_RASTERIZE_FLOAT     const s_count    = skc_subgroup_shuffle(s_segs,s_source);

      SKC_RASTERIZE_PREDICATE const is_s_first = (s_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_s_last  = (s_delta >= s_count);

      //
      // init parametric t
      //
      SKC_RASTERIZE_FLOAT s_t = s_delta * skc_subgroup_shuffle(s_denom,s_source); // faster than native_recip(s_count)?

      //
      // if last then override to a hard 1.0f
      //
      s_t    = is_s_last ? 1.0f : s_t;

      //
      // decrement by subgroup size
      //
      s_iss -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      s_ess -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;
      s_rem -= SKC_RASTERIZE_ELEMS_PER_SUBGROUP;

      //
      // now every lane knows what to do and the following lines will
      // pump out up to SUBGROUP_SIZE line segments
      //
      // obtain the src vertices through shared or via a shuffle
      //

      //
      // shuffle in the polynomial coefficients their source lane
      //
      SKC_RASTERIZE_FLOAT const s0x = skc_subgroup_shuffle(b0x,s_source);
      SKC_RASTERIZE_FLOAT const s0y = skc_subgroup_shuffle(b0y,s_source);

      SKC_RASTERIZE_FLOAT const s1x = skc_subgroup_shuffle(b1x,s_source);
      SKC_RASTERIZE_FLOAT const s1y = skc_subgroup_shuffle(b1y,s_source);

      SKC_RASTERIZE_FLOAT const s2x = skc_subgroup_shuffle(b2x,s_source);
      SKC_RASTERIZE_FLOAT const s2y = skc_subgroup_shuffle(b2y,s_source);

      //
      // calculate "right" line segment endpoint using Horner form
      //
      SKC_RASTERIZE_FLOAT       l1x = round(mad(mad(s2x,s_t,s1x),s_t,s0x)); // 2 MAD + ROUND
      SKC_RASTERIZE_FLOAT       l1y = round(mad(mad(s2y,s_t,s1y),s_t,s0y)); // 2 MAD + ROUND

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT       l0x = skc_subgroup_shuffle_up_1(l1x_prev,l1x);
      SKC_RASTERIZE_FLOAT       l0y = skc_subgroup_shuffle_up_1(l1y_prev,l1y);

      //
      // save previous right endpoint
      //
      l1x_prev = l1x;
      l1y_prev = l1y;

      //
      // override shuffle up if this is the first line segment
      //
      l0x = select(l0x,s0x,is_s_first);
      l0y = select(l0y,s0y,is_s_first);

      //
      // sliver lines
      //
      skc_sliver(bp_atomics,
                 bp_elems,
                 bp_ids,
                 bp_mask,
                 cohort_atomics,
                 &subblocks,
                 &blocks,
                 &blocks_next,
                 &sk_v,
                 &sk_v_next,
                 sk_extent,
                 smem,
                 l0x,l0y,l1x,l1y);
    }

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(bp_atomics,
               bp_elems,
               bp_ids,
               bp_mask,
               cohort_atomics,
               &blocks,
               blocks_next,
               &sk_v,
               sk_v_next,
               sk_extent,
               smem);
}

//
// RASTERIZE LINE KERNEL
//

static
void
skc_rasterize_lines(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                    __global union skc_bp_elem                * const bp_elems,
                    __global uint                             * const bp_ids,
                    skc_uint                                    const bp_mask,

                    __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                    __global skc_ttsk_s_t                     * const sk_extent,

                    __local struct skc_subgroup_smem volatile * const smem,
                    
                    skc_uint                                  * const nodeword,
                    skc_block_id_t                            * const id,

                    union skc_transform              const    * const tv,
                    union skc_path_clip              const    * const cv,
                    skc_uint                                    const cohort)
{
  //
  // the initial segment idx and segments-per-block constant determine
  // how many block ids will need to be loaded
  //
  SKC_RASTERIZE_FLOAT const c0x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c0y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1x = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

  skc_segment_next(bp_elems,nodeword,id);

  SKC_RASTERIZE_FLOAT const c1y = bp_elems[SKC_RASTERIZE_SEGMENT(*id)].coord;

#if 0
  // printf("%5u : { { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",(skc_uint)get_global_id(0),c0x,c0y,c1x,c1y);
  printf("{ { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",c0x,c0y,c1x,c1y);
#endif

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only
  // FIXME -- support perspective later
  //
  // the affine transformation requires 8 FMA + 4 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const l0x = round(c0x * tv->sx  + c0y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const l0y = round(c0x * tv->shy + c0y * tv->sy  + tv->ty);

  SKC_RASTERIZE_FLOAT const l1x = round(c1x * tv->sx  + c1y * tv->shx + tv->tx);
  SKC_RASTERIZE_FLOAT const l1y = round(c1x * tv->shy + c1y * tv->sy  + tv->ty);

#if 0
  printf("{ { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",l0x,l0y,l1x,l1y);
#endif

  //
  // allocate and init in-register TTSK keys
  //
  skc_uint     sk_v_next = 0;
  skc_ttsk_v_t sk_v; 

  sk_v.hi = cohort;

  //
  // initialize smem
  //
  skc_smem_init(smem);

  //
  // initialize blocks / subblocks
  //
  skc_block_id_v_t blocks;
  skc_uint         blocks_next = SKC_RASTERIZE_BLOCK_ID_V_SIZE;

#if SKC_DEVICE_BLOCK_WORDS_LOG2 > SKC_DEVICE_SUBBLOCK_WORDS_LOG2
  skc_block_id_t   subblocks   = 0;
#endif

  //
  // sliver lines
  //
  skc_sliver(bp_atomics,
             bp_elems,
             bp_ids,
             bp_mask,
             cohort_atomics,
             &subblocks,
             &blocks,
             &blocks_next,
             &sk_v,
             &sk_v_next,
             sk_extent,
             smem,
             l0x,l0y,l1x,l1y);

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(bp_atomics,
               bp_elems,
               bp_ids,
               bp_mask,
               cohort_atomics,
               &blocks,
               blocks_next,
               &sk_v,
               sk_v_next,
               sk_extent,
               smem);
}

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_all(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                         __global union skc_bp_elem                * const bp_elems,
                         __global uint                             * const bp_ids,
                         skc_uint                                    const bp_mask,

                         __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                         __global skc_ttsk_s_t                     * const sk_extent,

                         __global float8                  const    * const transforms, // FIXME -- __constant
                         __global float4                  const    * const clips,      // FIXME -- __constant
                         __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                         skc_uint                                    const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  __local struct skc_subgroup_smem volatile                smem[1];
#else
  __local struct skc_subgroup_smem volatile                smem_wg[SKC_RASTERIZE_WORKGROUP_SUBGROUPS];
  __local struct skc_subgroup_smem volatile * const smem = smem_wg + get_sub_group_id();
#endif
  
  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

#if 0
  if (get_sub_group_local_id() == 0)
    printf("+cmd_idx = %u\n",cmd_idx);
#endif

  //
  // if worksgroups are multi-subgroup then there may be excess
  // subgroups in the final workgroup
  //
  if (cmd_idx >= count)
    return;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("-cmd_idx = %u\n",cmd_idx);
#endif

  //
  // load a single command for this subgroup
  //
  union skc_cmd_rasterize const cmd = cmds[cmd_idx];

#if 0
  if (get_sub_group_local_id() == 0)
    printf("[ %u ]< %u, %u, %u, %u >\n",
           cmd_idx,
           cmd.nodeword,
           SKC_CMD_RASTERIZE_GET_TRANSFORM(cmd),
           SKC_CMD_RASTERIZE_GET_CLIP(cmd),
           SKC_CMD_RASTERIZE_GET_COHORT(cmd));
#endif

  //
  // get first block node command word and its subblock
  //
  skc_uint              nodeword = cmd.nodeword; // nodeword has word-addressing
  skc_tagged_block_id_t tag_id   = bp_elems[nodeword].tag_id;
  skc_block_id_tag      tag      = SKC_TAGGED_BLOCK_ID_GET_TAG(tag_id);
  skc_block_id_t        id       = SKC_TAGGED_BLOCK_ID_GET_ID(tag_id);

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  // FIXME -- horizontal load might be better than this broadcast load
  //
  union skc_transform const tv     = { .f32v8 = transforms[SKC_CMD_RASTERIZE_GET_TRANSFORM(cmd)] }; // uniform load
  union skc_path_clip const cv     = { .f32v4 = clips     [SKC_CMD_RASTERIZE_GET_CLIP(cmd)     ] }; // uniform load
  skc_uint            const cohort = SKC_CMD_RASTERIZE_MASK_COHORT(cmd); // shifted

  switch (tag)
    {
    case SKC_BLOCK_ID_TAG_PATH_LINE:
      skc_rasterize_lines(bp_atomics,
                          bp_elems,
                          bp_ids,
                          bp_mask,
                          cohort_atomics,
                          sk_extent,
                          smem,
                          &nodeword,&id,
                          &tv,&cv,cohort);
      break;

    case SKC_BLOCK_ID_TAG_PATH_QUAD:
      skc_rasterize_quads(bp_atomics,
                          bp_elems,
                          bp_ids,
                          bp_mask,
                          cohort_atomics,
                          sk_extent,
                          smem,
                          &nodeword,&id,
                          &tv,&cv,cohort);
      break;

    case SKC_BLOCK_ID_TAG_PATH_CUBIC:
      skc_rasterize_cubics(bp_atomics,
                           bp_elems,
                           bp_ids,
                           bp_mask,
                           cohort_atomics,
                           sk_extent,
                           smem,
                           &nodeword,&id,
                           &tv,&cv,cohort);
      break;

    case SKC_BLOCK_ID_TAG_PATH_RAT_QUAD:
      break;
    case SKC_BLOCK_ID_TAG_PATH_RAT_CUBIC:
      break;

    default:
      break;
    }
}

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_lines(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                           __global union skc_bp_elem                * const bp_elems,
                           __global uint                             * const bp_ids,
                           skc_uint                                    const bp_mask,

                           __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                           __global skc_ttsk_s_t                     * const sk_extent,

                           __global float8                  const    * const transforms, // FIXME -- __constant
                           __global float4                  const    * const clips,      // FIXME -- __constant
                           __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                           skc_uint                                    const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  __local struct skc_subgroup_smem volatile                smem[1];
#else
  __local struct skc_subgroup_smem volatile                smem_wg[SKC_RASTERIZE_WORKGROUP_SUBGROUPS];
  __local struct skc_subgroup_smem volatile * const smem = smem_wg + get_sub_group_id();
#endif
  
  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  //
  // if worksgroups are multi-subgroup then there may be excess
  // subgroups in the final workgroup
  //
  if (cmd_idx >= count)
    return;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("cmd_idx = %u\n",cmd_idx);
#endif

  //
  // load a single command for this subgroup
  //
  union skc_cmd_rasterize const cmd = cmds[cmd_idx];

  //
  // get first block node command word and its subblock
  //
  skc_uint              nodeword = cmd.nodeword; // nodeword has word-addressing
  skc_tagged_block_id_t tag_id   = bp_elems[nodeword].tag_id;
  skc_block_id_t        id       = SKC_TAGGED_BLOCK_ID_GET_ID(tag_id);

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  // FIXME -- horizontal load might be better than this broadcast load
  //
  union skc_transform const tv     = { .f32v8 = transforms[SKC_CMD_RASTERIZE_GET_TRANSFORM(cmd)] }; // uniform load
  union skc_path_clip const cv     = { .f32v4 = clips     [SKC_CMD_RASTERIZE_GET_CLIP(cmd)     ] }; // uniform load
  skc_uint            const cohort = SKC_CMD_RASTERIZE_MASK_COHORT(cmd); // shifted

  skc_rasterize_lines(bp_atomics,
                      bp_elems,
                      bp_ids,
                      bp_mask,
                      cohort_atomics,
                      sk_extent,
                      smem,
                      &nodeword,&id,
                      &tv,&cv,cohort);
}

//
//
//

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_quads(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                           __global union skc_bp_elem                * const bp_elems,
                           __global uint                             * const bp_ids,
                           skc_uint                                    const bp_mask,

                           __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                           __global skc_ttsk_s_t                     * const sk_extent,

                           __global float8                  const    * const transforms, // FIXME -- __constant
                           __global float4                  const    * const clips,      // FIXME -- __constant
                           __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                           skc_uint                                    const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  __local struct skc_subgroup_smem volatile                smem[1];
#else
  __local struct skc_subgroup_smem volatile                smem_wg[SKC_RASTERIZE_WORKGROUP_SUBGROUPS];
  __local struct skc_subgroup_smem volatile * const smem = smem_wg + get_sub_group_id();
#endif
  
  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  //
  // if worksgroups are multi-subgroup then there may be excess
  // subgroups in the final workgroup
  //
  if (cmd_idx >= count)
    return;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("cmd_idx = %u\n",cmd_idx);
#endif

  //
  // load a single command for this subgroup
  //
  union skc_cmd_rasterize const cmd = cmds[cmd_idx];

  //
  // get first block node command word and its subblock
  //
  skc_uint              nodeword = cmd.nodeword; // nodeword has word-addressing
  skc_tagged_block_id_t tag_id   = bp_elems[nodeword].tag_id;
  skc_block_id_t        id       = SKC_TAGGED_BLOCK_ID_GET_ID(tag_id);

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  // FIXME -- horizontal load might be better than this broadcast load
  //
  union skc_transform const tv     = { .f32v8 = transforms[SKC_CMD_RASTERIZE_GET_TRANSFORM(cmd)] }; // uniform load
  union skc_path_clip const cv     = { .f32v4 = clips     [SKC_CMD_RASTERIZE_GET_CLIP(cmd)     ] }; // uniform load
  skc_uint            const cohort = SKC_CMD_RASTERIZE_MASK_COHORT(cmd); // shifted

  skc_rasterize_quads(bp_atomics,
                      bp_elems,
                      bp_ids,
                      bp_mask,
                      cohort_atomics,
                      sk_extent,
                      smem,
                      &nodeword,&id,
                      &tv,&cv,cohort);
}

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_cubics(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                            __global union skc_bp_elem                * const bp_elems,
                            __global uint                             * const bp_ids,
                            skc_uint                                    const bp_mask,

                            __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                            __global skc_ttsk_s_t                     * const sk_extent,

                            __global float8                  const    * const transforms, // FIXME -- __constant
                            __global float4                  const    * const clips,      // FIXME -- __constant
                            __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                            skc_uint                                    const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  __local struct skc_subgroup_smem volatile                smem[1];
#else
  __local struct skc_subgroup_smem volatile                smem_wg[SKC_RASTERIZE_WORKGROUP_SUBGROUPS];
  __local struct skc_subgroup_smem volatile * const smem = smem_wg + get_sub_group_id();
#endif
  
  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_SUBGROUPS == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  //
  // if worksgroups are multi-subgroup then there may be excess
  // subgroups in the final workgroup
  //
  if (cmd_idx >= count)
    return;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("cmd_idx = %u\n",cmd_idx);
#endif

  //
  // load a single command for this subgroup
  //
  union skc_cmd_rasterize const cmd = cmds[cmd_idx];

  //
  // get first block node command word and its subblock
  //
  skc_uint              nodeword = cmd.nodeword; // nodeword has word-addressing
  skc_tagged_block_id_t tag_id   = bp_elems[nodeword].tag_id;
  skc_block_id_t        id       = SKC_TAGGED_BLOCK_ID_GET_ID(tag_id);

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  // FIXME -- horizontal load might be better than this broadcast load
  //
  union skc_transform const tv     = { .f32v8 = transforms[SKC_CMD_RASTERIZE_GET_TRANSFORM(cmd)] }; // uniform load
  union skc_path_clip const cv     = { .f32v4 = clips     [SKC_CMD_RASTERIZE_GET_CLIP(cmd)     ] }; // uniform load
  skc_uint            const cohort = SKC_CMD_RASTERIZE_MASK_COHORT(cmd); // shifted

  skc_rasterize_cubics(bp_atomics,
                       bp_elems,
                       bp_ids,
                       bp_mask,
                       cohort_atomics,
                       sk_extent,
                       smem,
                       &nodeword,&id,
                       &tv,&cv,cohort);
}

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_rat_quads(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                               __global union skc_bp_elem                * const bp_elems,
                               __global uint                             * const bp_ids,
                               skc_uint                                    const bp_mask,

                               __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                               __global skc_ttsk_s_t                     * const sk_extent,

                               __global float8                  const    * const transforms, // FIXME -- __constant
                               __global float4                  const    * const clips,      // FIXME -- __constant
                               __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                               skc_uint                                    const count)
{
  ;
}

//
//
//

__kernel
SKC_RASTERIZE_KERNEL_ATTRIBS
void
skc_kernel_rasterize_rat_cubics(__global SKC_ATOMIC_UINT         volatile * const bp_atomics,
                                __global union skc_bp_elem                * const bp_elems,
                                __global uint                             * const bp_ids,
                                skc_uint                                    const bp_mask,

                                __global SKC_ATOMIC_UINT         volatile * const cohort_atomics,
                                __global skc_ttsk_s_t                     * const sk_extent,

                                __global float8                  const    * const transforms, // FIXME -- __constant
                                __global float4                  const    * const clips,      // FIXME -- __constant
                                __global union skc_cmd_rasterize const    * const cmds,       // FIXME -- __constant
                                skc_uint                                    const count)
{
  ;
}

//
//
//

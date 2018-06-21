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
#include "raster.h"
#include "atomic_cl.h"
#include "kernel_cl_12.h"

//
//
//

#define SKC_PLACE_SUBGROUP_MASK      (SKC_PLACE_SUBGROUP_SIZE - 1)
#define SKC_PLACE_SUBGROUP_LAST      (SKC_PLACE_SUBGROUP_SIZE - 1)

//
//
//

#define SKC_PLACE_SMEM_COUNT_TTSK    SKC_MAX_MACRO(SKC_RASTER_NODE_MAX_TTSK,SKC_PLACE_SUBGROUP_SIZE)
#define SKC_PLACE_SMEM_COUNT_TTPK    SKC_RASTER_NODE_MAX_TTPK

//
//
//

#define SKC_PLACE_X                  (SKC_DEVICE_BLOCK_DWORDS / SKC_PLACE_SUBGROUP_SIZE)

//
//
//

#if   ( SKC_PLACE_X == 1 )
#define SKC_PLACE_EXPAND()           SKC_EXPAND_1()
#define SKC_PLACE_EXPAND_I_LAST      0

#elif ( SKC_PLACE_X == 2 )
#define SKC_PLACE_EXPAND()           SKC_EXPAND_2()
#define SKC_PLACE_EXPAND_I_LAST      1

#elif ( SKC_PLACE_X == 4 )
#define SKC_PLACE_EXPAND()           SKC_EXPAND_4()
#define SKC_PLACE_EXPAND_I_LAST      3

#elif ( SKC_PLACE_X == 8 )
#define SKC_PLACE_EXPAND()           SKC_EXPAND_8()
#define SKC_PLACE_EXPAND_I_LAST      7

#elif ( SKC_PLACE_X == 16)
#define SKC_PLACE_EXPAND()           SKC_EXPAND_16()
#define SKC_PLACE_EXPAND_I_LAST      15
#endif

//
// PREFIX STORES THE 64-BIT KEYS WITH TWO 32-BIT SUBGROUP-WIDE
// COALESCED WRITES.  LO FIRST, FOLLOWED BY HI.
//
// THIS SLIGHTLY COMPLICATES LOADING BY THE PLACE KERNEL IF THE
// KERNELS USE DIFFERENT SUBGROUP SIZES.
//
// THE BENEFIT IS THAT THE RASTER RECLAIM KERNEL ONLY HAS TO LOAD THE
// LO WORD OF THE KEY SINCE IT CONTAINS THE BLOCK ID.
//
// NOTE: AT THIS POINT, ONLY INTEL'S HD GRAPHICS ARCHITECTURE UNDER
// OPENCL SUPPORTS SELECTING A SUBGROUP SIZE (8/16/32). VULKAN MAY
// ONLY SUPPORT A SUBGROUP SIZE OF 16.
//

#if    ( SKC_PREFIX_SUBGROUP_SIZE == SKC_PLACE_SUBGROUP_SIZE )

#define SKC_PLACE_STRIDE_H(L)              (L)
#define SKC_PLACE_STRIDE_V_LO(I)           (I * 2 * SKC_PLACE_SUBGROUP_SIZE)
#define SKC_PLACE_STRIDE_V_HI(I)           (SKC_PLACE_STRIDE_V_LO(I) + SKC_PLACE_SUBGROUP_SIZE)

#elif  ( SKC_PREFIX_SUBGROUP_SIZE >  SKC_PLACE_SUBGROUP_SIZE ) // same as above when ratio equals 1

#define SKC_PLACE_SUBGROUP_RATIO           (SKC_PREFIX_SUBGROUP_SIZE / SKC_PLACE_SUBGROUP_SIZE)
#define SKC_PLACE_SUBGROUP_RATIO_MASK      (SKC_PLACE_SUBGROUP_RATIO - 1)
#define SKC_PLACE_SUBGROUP_RATIO_SCALE(I)  ((I / SKC_PLACE_SUBGROUP_RATIO) * 2 * SKC_PLACE_SUBGROUP_RATIO + (I & SKC_PLACE_SUBGROUP_RATIO_MASK))

#define SKC_PLACE_STRIDE_H(L)              (L)
#define SKC_PLACE_STRIDE_V_LO(I)           (SKC_PLACE_SUBGROUP_RATIO_SCALE(I) * SKC_PLACE_SUBGROUP_SIZE)
#define SKC_PLACE_STRIDE_V_HI(I)           (SKC_PLACE_STRIDE_V_LO(I) + SKC_PLACE_SUBGROUP_RATIO * SKC_PLACE_SUBGROUP_SIZE)

#elif  ( SKC_PREFIX_SUBGROUP_SIZE <  SKC_PLACE_SUBGROUP_SIZE ) // same as above when ratio equals 1

#define SKC_PLACE_SUBGROUP_RATIO           (SKC_PLACE_SUBGROUP_SIZE / SKC_PREFIX_SUBGROUP_SIZE)
#define SKC_PLACE_SUBGROUP_RATIO_MASK      (SKC_PLACE_SUBGROUP_SIZE / SKC_PLACE_SUBGROUP_RATIO - 1) // equal to prefix subgroup mask

#define SKC_PLACE_STRIDE_H(L)              (((L) & ~SKC_PLACE_SUBGROUP_RATIO_MASK) * 2 + ((L) & SKC_PLACE_SUBGROUP_RATIO_MASK))
#define SKC_PLACE_STRIDE_V_LO(I)           (I * 2 * SKC_PLACE_SUBGROUP_SIZE)
#define SKC_PLACE_STRIDE_V_HI(I)           (SKC_PLACE_STRIDE_V_LO(I) + SKC_PLACE_SUBGROUP_SIZE / SKC_PLACE_SUBGROUP_RATIO)

#endif

//
// A COARSE COMPILE-TIME GUARD -- WILL ONLY MATTER WHEN SUBGROUP SIZE
// IS EQUAL TO THE RASTER HEADER SIZE (CURRENTLY 8)
//

#define SKC_PLACE_IS_ALL_HEADER_ROW(i)   (((i)+1) * SKC_PLACE_SUBGROUP_SIZE <= SKC_RASTER_HEAD_DWORDS)

#define SKC_PLACE_IS_NOT_HEADER_ROW(i)   ( (i)    * SKC_PLACE_SUBGROUP_SIZE >= SKC_RASTER_HEAD_DWORDS)

#define SKC_PLACE_IS_TRAILING_ROW(i)     (((i)+1) * SKC_PLACE_SUBGROUP_SIZE == SKC_DEVICE_BLOCK_DWORDS)

#define SKC_PLACE_IS_HEADER_ROW_KEY(i)   ((i) * SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id() - SKC_RASTER_HEAD_DWORDS < (k))


//
// Note: HEADER_LESS_THAN purposefully wraps unsigned integer to ~UINT_MAX
//
#define SKC_PLACE_HEADER_LESS_THAN(i,k) ((i) * SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id() - SKC_RASTER_HEAD_DWORDS < (k))
#define SKC_PLACE_NODE_LESS_THAN(i,k)   ((i) * SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id()                          < (k))

//
// TTSK v2:
//
//  0                                       63
//  | TTSB ID | PREFIX |  SPAN   |  X  |  Y  |
//  +---------+--------+---------+-----+-----+
//  |    27   | 1 (=0) | 12 (=0) | 12  | 12  |
//
//
// TTPK v2:
//
//  0                                    63
//  | TTPB ID | PREFIX | SPAN |  X  |  Y  |
//  +---------+--------+------+-----+-----+
//  |    27   | 1 (=1) |  12  | 12  | 12  |
//
//

//
// TTCK (32-BIT COMPARE) v1:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   18  |  7  |  7  |
//
//
// TTCK (32-BIT COMPARE) v2:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   15  |  9  |  8  |
//
//
// TTCK (64-BIT COMPARE) -- achieves 4K x 4K with an 8x16 tile:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          27          |    1   |    1   |   18  |  9  |  8  |
//

union skc_subgroup_smem
{
  skc_uint scratch[SKC_PLACE_SUBGROUP_SIZE]; // will only use SKC_PLACE_SUBGROUP_SIZE

  struct {
    struct {
      skc_ttsk_lo_t sk[SKC_PLACE_SMEM_COUNT_TTSK];
      skc_ttpk_lo_t pk[SKC_PLACE_SMEM_COUNT_TTPK];
    } lo;

    struct {
      skc_ttsk_hi_t sk[SKC_PLACE_SMEM_COUNT_TTSK];
      skc_ttpk_hi_t pk[SKC_PLACE_SMEM_COUNT_TTPK];
    } hi;

    // skc_uint span[SKC_PLACE_SMEM_COUNT_TTPK];
  };

};

//
// scatter scan max
//
static
skc_int_v_t
skc_scatter_scan_max(__local union skc_subgroup_smem  volatile * const smem,
                     skc_int_v_t                                 const iss,
                     skc_int_v_t                                 const ess)
{
  //
  // prefix sums determine which lanes we're going to work on next
  //
  skc_pred_v_t const is_scratch_store = (iss > 0) && (ess < SKC_PLACE_SUBGROUP_SIZE);
  skc_int_v_t  const scratch_idx      = max(ess,0);

  //
  // SIMT
  //

  //
  // zero the volatile smem scratchpad using vector syntax
  //
  smem->scratch[get_sub_group_local_id()] = ( 0 );

  //
  // store source lane at starting lane
  //
  if (is_scratch_store) {
    smem->scratch[scratch_idx] = get_sub_group_local_id();
  }

  //
  // propagate lanes to right using max scan
  //
  skc_int_v_t const scratch = smem->scratch[get_sub_group_local_id()];
  skc_int_v_t const source  = sub_group_scan_inclusive_max(scratch);

  return source;
}

//
//
//

static
skc_bool
skc_xk_clip(union skc_tile_clip const * const tile_clip,
            skc_ttxk_t                * const xk)
{
  //
  // clip the sk and pk keys
  //
  // if fully clipped then return false
  //
  // alternatively -- we can expand all these keys in place
  //
  // alternatively -- keep sk and pk keys segregated because sk
  // represents the vast majority of keys and are easier to process.
  // don't mess with the fastpath!
  //
  return false;
}

//
//
//

static
skc_ttck_t
skc_sk_to_ck(__local union skc_subgroup_smem  volatile * const smem,
             union skc_cmd_place              const    * const cmd,
             skc_uint                                    const sk_idx)
{
  skc_uint const lo = smem->lo.sk[sk_idx]; // assumes prefix bit is 0
  skc_uint const hi = smem->hi.sk[sk_idx];

  skc_ttck_t ck;

  ck.lo = lo | (cmd->layer_id << SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE); // FIXME -- preshift the layer id

  // FIXME -- x and y should already be clipped and shifted
  skc_uint const x = (cmd->tx + SKC_BFE(hi,SKC_TTXK_HI_BITS_X,SKC_TTXK_HI_OFFSET_X)) << SKC_TTCK_HI_OFFSET_X;
  skc_uint const y = (cmd->ty + SKC_BFE(hi,SKC_TTXK_HI_BITS_Y,SKC_TTXK_HI_OFFSET_Y)) << SKC_TTCK_HI_OFFSET_Y;

  ck.hi = (cmd->layer_id >> SKC_TTCK_HI_SHR_LAYER) | x | y;

  return ck;
}

static
skc_ttck_t
skc_pk_to_ck(__local union skc_subgroup_smem  volatile * const smem,
             union skc_cmd_place              const    * const cmd,
             skc_uint                                    const pk_idx,
             skc_uint                                    const dx)
{
  skc_uint const lo = smem->lo.pk[pk_idx] & SKC_TTXK_LO_MASK_ID_PREFIX; // assumes prefix bit is 1
  skc_uint const hi = smem->hi.pk[pk_idx];

  skc_ttck_t ck;

  ck.lo = lo | (cmd->layer_id << SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE); // FIXME -- preshift the layer id

  // FIXME -- x and y should already be clipped and shifted
  skc_uint const x = (cmd->tx + dx + SKC_BFE(hi,SKC_TTXK_HI_BITS_X,SKC_TTXK_HI_OFFSET_X)) << SKC_TTCK_HI_OFFSET_X;
  skc_uint const y = (cmd->ty +      SKC_BFE(hi,SKC_TTXK_HI_BITS_Y,SKC_TTXK_HI_OFFSET_Y)) << SKC_TTCK_HI_OFFSET_Y;

  ck.hi = (cmd->layer_id >> SKC_TTCK_HI_SHR_LAYER) | x | y;

  return ck;
}

//
//
//

static
void
skc_ttsk_flush(__global SKC_ATOMIC_UINT         volatile * const place_atomics,
               __global skc_ttck_t                       * const ck_extent,
               __local union skc_subgroup_smem  volatile * const smem,
               union skc_cmd_place              const    * const cmd,
               skc_uint                         const            sk)
{
  //
  // Pretty sure you can never ever have an sk count equal to 0
  //
  skc_uint ck_base = 0;

  // last lane performs the block pool allocation with an atomic increment
  if (get_sub_group_local_id() == 0) {
    ck_base = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(place_atomics,sk);
  }

  // broadcast base to all lanes
  ck_base = sub_group_broadcast(ck_base,0);

  // convert sk keys to ck keys
  for (skc_uint ii=get_sub_group_local_id(); ii<sk; ii+=SKC_PLACE_SUBGROUP_SIZE)
    {
      ck_extent[ck_base+ii] = skc_sk_to_ck(smem,cmd,ii);
    }
}

//
//
//

static
skc_int
skc_ttpk_get_span(__local union skc_subgroup_smem  volatile * const smem,
                  skc_uint                                    const idx)
{
  skc_uint const lo      = smem->lo.pk[idx];
  skc_uint const hi      = smem->hi.pk[idx];

  skc_uint const span_lo = lo >> SKC_TTXK_LO_OFFSET_SPAN;
  skc_uint const span_hi = (hi & SKC_BITS_TO_MASK(SKC_TTXK_HI_BITS_SPAN)) << SKC_TTXK_LO_BITS_SPAN;

  return (span_lo | span_hi) + 1;
}

//
//
//

static
void
skc_ttpk_flush(__global SKC_ATOMIC_UINT         volatile * const place_atomics,
               __global skc_ttck_t                       * const ck_extent,
               __local union skc_subgroup_smem  volatile * const smem,
               union skc_cmd_place              const    * const cmd,
               skc_uint                         const            pk)
{
  // bail out if pk queue is empty
  if (pk == 0)
    return;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("%u\n",pk);
#endif

  //
  // FIXME -- this nested loop iterates over the queue processing a
  // subgroup of 64-bit keys at a time.  This is probably not the most
  // efficient approach so investigate how to store and iterate over a
  // wider than subgroup (node-sized) queue of keys.
  //

  // round up so we work with full subgroups
  skc_uint const pk_ru = (pk + SKC_PLACE_SUBGROUP_SIZE - 1) & ~SKC_PLACE_SUBGROUP_MASK;
  skc_uint       ii    = 0;

  // nested loop that expands all ttpk keys
#if (SKC_PLACE_SMEM_COUNT_TTPK > SKC_PLACE_SUBGROUP_SIZE)
  for (; ii<pk_ru; ii+=SKC_PLACE_SUBGROUP_SIZE)
#endif
    {
      skc_uint idx  = ii + get_sub_group_local_id();
      skc_int  span = 0;

      // how many tiles does this ttpk span?
      if (idx < pk)
        span = skc_ttpk_get_span(smem,idx);

      // we need inclusive, exclusive and total
      skc_int iss = sub_group_scan_inclusive_add(span);
      skc_int ess = iss - span;
      skc_int rem = sub_group_broadcast(iss,SKC_PLACE_SUBGROUP_SIZE-1);

      // printf("%u : %u\n",span,iss);
      // continue;

      // atomically allocate space for the pk keys
      skc_uint ck_base = 0;

      // last lane performs the block pool allocation with an atomic increment
      if (get_sub_group_local_id() == 0) {
        ck_base = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(place_atomics,rem);
      }

      // broadcast atomically allocated extent base to all lanes
      skc_uint ck_idx = sub_group_broadcast(ck_base,0) + get_sub_group_local_id();

      //
      // FIXME -- this loop would probably be faster if the ttpk keys
      // were held in registers and accessed with shuffles instead of
      // SMEM loads
      //

      //
      // loop until there are no more expanded pk keys
      //
      while (true)
        {
          skc_int const source = skc_scatter_scan_max(smem,iss,ess);
          skc_int const dx     = get_sub_group_local_id() - intel_sub_group_shuffle(ess,source);

          // store valid ck keys to gmem
          if (get_sub_group_local_id() < rem) {
            ck_extent[ck_idx] = skc_pk_to_ck(smem,cmd,ii+source,dx);
          }

          // decrement remainder
          rem -= SKC_PLACE_SUBGROUP_SIZE;

          if (rem <= 0)
            break;

          // increment/decrement indices
          ck_idx += SKC_PLACE_SUBGROUP_SIZE;
          iss    -= SKC_PLACE_SUBGROUP_SIZE;
          ess    -= SKC_PLACE_SUBGROUP_SIZE;
        }
    }
}

//
//
//

static
skc_uint
skc_ballot(skc_uint * const xk, skc_uint const is_xk)
{
#if 0
  //
  // FIXME -- when available, this should use the idiom:
  //
  //   ballot() + lane_mask_less_than_or_equal + popcount()
  //
  // Supported by:
  //
  //   - Vulkan 1.1 / SPIR-V 1.3
  //   - CUDA
  //   - AVX2 (SSE*?)
  //
#else
  //
  // otherwise, emulate with an inclusive scan (yuk)
  //
  skc_uint const prefix = sub_group_scan_inclusive_add(is_xk);

  skc_uint const xk_idx = *xk + prefix - is_xk;

  *xk += sub_group_broadcast(prefix,SKC_PLACE_SUBGROUP_LAST);

#if 0
  printf("< %3u >\n",xk_idx);
#endif

  return xk_idx;
#endif
}

//
//
//
__kernel
SKC_PLACE_KERNEL_ATTRIBS
void
skc_kernel_place(__global skc_bp_elem_t                * const bp_elems,
                 __global SKC_ATOMIC_UINT     volatile * const place_atomics,
                 __global skc_ttck_t                   * const ck_extent,
                 __global union skc_cmd_place const    * const cmds,
                 __global skc_block_id_t               * const map,
                 skc_uint4                               const clip,
                 skc_uint                                const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_PLACE_WORKGROUP_SUBGROUPS == 1 )
  __local union skc_subgroup_smem  volatile                smem[1];
#else
  __local union skc_subgroup_smem  volatile                smem_wg[SKC_PLACE_WORKGROUP_SUBGROUPS];
  __local union skc_subgroup_smem  volatile * const smem = smem_wg + get_sub_group_id();
#endif

  //
  // This is a subgroup-centric kernel
  //
  // Which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
  // Test the raster's translated bounds against the composition's
  // tile clip
  //
  // There are 3 cases:
  //
  //   - the raster is completely clipped -> return
  //   - the raster is partially  clipped -> all keys must clipped
  //   - the raster is not        clipped -> no keys are tested
  //
  //
  // There are at least 4 implementations of place and we want to
  // special-case them as much as possible so that, at the least, the
  // fastpath remains fast.
  //
  //  - implement NO CLIP + NO TRANSLATION fastpath -- CAN ATOMICALLY ALLOCATE SK+PK KEYS IN ONE STEP
  //
  //  - implement CLIPPED + NO TRANSLATION path
  //
  //  - implement NO CLIP +    TRANSLATION path
  //
  //  - implement CLIPPED +    TRANSLATION path
  //
  //
  // FIXME/OPTIMIZATION: split scan accumulator into a triple-bin
  // 12:12:8 integer where:
  //
  //  12: ttsk
  //  12: ttpk
  //   8: /dev/null -- clipped or invalid key
  //
  // Three kinds of nodes in a raster's list:
  //
  //  - the head node
  //  - an internal node
  //  - the final node
  //

#if ( SKC_PLACE_WORKGROUP_SUBGROUPS == 1 )
  skc_uint const cmd_idx = get_group_id(0);
#else
  skc_uint const cmd_idx = get_group_id(0) * SKC_PLACE_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  // load command
  union skc_cmd_place const cmd = cmds[cmd_idx];

  // get the raster header from the raster host id -- scalar
  skc_block_id_t            id  = map[cmd.raster_h];

  //
  // load all of the head block ttxk keys into registers
  //
  // FIXME -- this pattern lends itself to using the higher
  // performance Intel GEN block load instructions
  //
  skc_uint const head_id = id * SKC_DEVICE_SUBBLOCK_WORDS + SKC_PLACE_STRIDE_H(get_sub_group_local_id());

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                 \
  union skc_raster_node_elem const h##I = {                     \
    .u32v2 = { bp_elems[head_id + SKC_PLACE_STRIDE_V_LO(I)],    \
               bp_elems[head_id + SKC_PLACE_STRIDE_V_HI(I)]  }  \
  };

  SKC_PLACE_EXPAND();

  //
  // load raster header counts -- we only need the "nodes" and "keys"
  // words but the keys we loaded are doublewords.
  //
  // FIXME -- this can be made portable with compile-time macro expansion
  //
  skc_uint nodes = sub_group_broadcast(h0.u32v2.lo,1); // SKC_RASTER_HEAD_OFFSET_COUNTS_NODES
  skc_uint keys  = sub_group_broadcast(h0.u32v2.hi,1); // SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS

  //
  //
  //
#if 0
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                 \
  printf("%5u :  %6u : %3u : %08X . %08X - %08X\n",             \
         nodes,keys,                                            \
         I*SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id(),  \
         h##I.u32v2.hi,h##I.u32v2.lo,                           \
         h##I.u32v2.lo & SKC_TTXK_LO_MASK_PREFIX);

  SKC_PLACE_EXPAND();
#endif

  //
#if 0
  if (get_sub_group_local_id() == 0) {
    printf("place: %u / %u / %u\n",head_id,nodes,keys);
  }
#endif

  {
    //
    // classify every key in the header
    //
    // keys: 0 is not a key / 1 is a key
    // skpk: 0 is sk        / 1 is pk
    //
    skc_uint bits_keys = 0;
    skc_uint bits_skpk = 0;

    //
    // calculate bits_keys
    //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
    if (!SKC_PLACE_IS_ALL_HEADER_ROW(I)) {                              \
      skc_uint const idx = I * SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id() - SKC_RASTER_HEAD_DWORDS; \
      if (idx < keys) {                                                 \
        bits_keys |= (1u << I);                                         \
      }                                                                 \
      if (SKC_PLACE_IS_TRAILING_ROW(I)) {                               \
        if (keys > SKC_RASTER_HEAD_COUNT_KEYS) {                        \
          if (get_sub_group_local_id() == SKC_PLACE_SUBGROUP_LAST) {    \
            bits_keys &= ~(1u << I);                                    \
          }                                                             \
        }                                                               \
      }                                                                 \
    }

    SKC_PLACE_EXPAND();

    //
    // blindly calculate bits_skpk
    //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
    if (!SKC_PLACE_IS_ALL_HEADER_ROW(I)) {                              \
      bits_skpk |= (h##I.xk.lo & SKC_TTXK_LO_MASK_PREFIX) >> (SKC_TTXK_LO_OFFSET_PREFIX - I); \
    }

    SKC_PLACE_EXPAND();

#if 0
    printf("%2X : %2X\n",bits_keys,bits_skpk);
#endif

    //
    // next pointer is last element of last row.  save it now because
    // this might be recognized as a subgroup-uniform/scalar.
    //
    id = sub_group_broadcast(SKC_CONCAT(h,SKC_PLACE_EXPAND_I_LAST).next.node,SKC_PLACE_SUBGROUP_LAST);

    //
    // append SK keys first
    //
    skc_uint const bits_sk = bits_keys & ~bits_skpk;
    skc_uint       sk      = 0;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                 \
    if (!SKC_PLACE_IS_ALL_HEADER_ROW(I)) {      \
      skc_uint is_sk  = (bits_sk >> I) & 1;     \
      skc_uint sk_idx = skc_ballot(&sk,is_sk);  \
      if (is_sk) {                              \
        smem->lo.sk[sk_idx] = h##I.xk.lo;       \
        smem->hi.sk[sk_idx] = h##I.xk.hi;       \
      }                                         \
    }

    SKC_PLACE_EXPAND();

    //
    // append PK keys next
    //
    skc_uint const bits_pk = bits_keys & bits_skpk;
    skc_uint       pk      = 0;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                 \
    if (!SKC_PLACE_IS_ALL_HEADER_ROW(I)) {      \
      skc_uint is_pk  = (bits_pk >> I) & 1;     \
      skc_uint pk_idx = skc_ballot(&pk,is_pk);  \
      if (is_pk) {                              \
        smem->lo.pk[pk_idx] = h##I.xk.lo;       \
        smem->hi.pk[pk_idx] = h##I.xk.hi;       \
      }                                         \
    }

    SKC_PLACE_EXPAND();

#if 0
    printf("%2u * %2u\n",sk,pk);
#endif
    //
    // flush the keys
    //
    skc_ttsk_flush(place_atomics,ck_extent,smem,&cmd,sk);
    skc_ttpk_flush(place_atomics,ck_extent,smem,&cmd,pk);
  }

  //
  // we're done if there was only a head node
  //
  if (nodes == 0)
    return;

  //
  // decrement keys
  //
  keys -= SKC_RASTER_HEAD_COUNT_KEYS;

  //
  // otherwise, append keys in trailing nodes to smem
  //
  while (true)
    {
      //
      // load all of the node block ttxk keys into registers
      //
      // FIXME -- this pattern lends itself to using the higher
      // performance Intel GEN block load instructions
      //
      skc_uint const node_id = id * SKC_DEVICE_SUBBLOCK_WORDS + SKC_PLACE_STRIDE_H(get_sub_group_local_id());

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      union skc_raster_node_elem const n##I = {                         \
        .u32v2 = { bp_elems[node_id + SKC_PLACE_STRIDE_V_LO(I)],        \
                   bp_elems[node_id + SKC_PLACE_STRIDE_V_HI(I)]  }      \
      };

      SKC_PLACE_EXPAND();

#if 0
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      printf("%5u :  %6u : %3u : %08X . %08X - %08X\n",                 \
             nodes,keys,                                                \
             I*SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id(),      \
             n##I.u32v2.hi,n##I.u32v2.lo,                               \
             n##I.u32v2.lo & SKC_TTXK_LO_MASK_PREFIX);

      SKC_PLACE_EXPAND();
#endif

      //
      // classify every key in the header
      //
      // keys: 0 is not a key / 1 is a key
      // skpk: 0 is sk        / 1 is pk
      //
      skc_uint bits_keys = 0;
      skc_uint bits_skpk = 0;

      //
      // calculate bits_keys
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                                       \
        skc_uint const idx = I * SKC_PLACE_SUBGROUP_SIZE + get_sub_group_local_id(); \
        if (idx < keys) {                                               \
          bits_keys |= (1u << I);                                       \
        }                                                               \
        if (SKC_PLACE_IS_TRAILING_ROW(I)) {                             \
          if (keys > SKC_RASTER_NODE_COUNT_KEYS) {                      \
            if (get_sub_group_local_id() == SKC_PLACE_SUBGROUP_LAST) {  \
              bits_keys &= ~(1u << I);                                  \
            }                                                           \
          }                                                             \
        }                                                               \
      }

      SKC_PLACE_EXPAND();

      //
      // blindly calculate bits_skpk
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                                       \
        bits_skpk |= (n##I.xk.lo & SKC_TTXK_LO_MASK_PREFIX) >> (SKC_TTXK_LO_OFFSET_PREFIX - I); \
      }

      SKC_PLACE_EXPAND();

#if 0
      printf("%2X : %2X\n",bits_keys,bits_skpk);
#endif

      //
      // next pointer is last element of last row.  save it now because
      // this might be recognized as a subgroup-uniform/scalar.
      //
      id = sub_group_broadcast(SKC_CONCAT(n,SKC_PLACE_EXPAND_I_LAST).next.node,SKC_PLACE_SUBGROUP_LAST);

      //
      // append SK keys first
      //
      skc_uint const bits_sk = bits_keys & ~bits_skpk;
      skc_uint       sk      = 0;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                       \
        skc_uint is_sk  = (bits_sk >> I) & 1;           \
        skc_uint sk_idx = skc_ballot(&sk,is_sk);        \
        if (is_sk) {                                    \
          smem->lo.sk[sk_idx] = n##I.xk.lo;             \
          smem->hi.sk[sk_idx] = n##I.xk.hi;             \
        }                                               \
      }

      SKC_PLACE_EXPAND();

      //
      // append PK keys next
      //
      skc_uint const bits_pk = bits_keys & bits_skpk;
      skc_uint       pk      = 0;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                       \
        skc_uint is_pk  = (bits_pk >> I) & 1;           \
        skc_uint pk_idx = skc_ballot(&pk,is_pk);        \
        if (is_pk) {                                    \
          smem->lo.pk[pk_idx] = n##I.xk.lo;             \
          smem->hi.pk[pk_idx] = n##I.xk.hi;             \
        }                                               \
      }

      SKC_PLACE_EXPAND();

#if 0
    printf("%2u * %2u\n",sk,pk);
#endif
      //
      // if total for either the sk or pk queue reaches the
      // highwater mark then flush it to the extent
      //
      skc_ttsk_flush(place_atomics,ck_extent,smem,&cmd,sk);
      skc_ttpk_flush(place_atomics,ck_extent,smem,&cmd,pk);

      //
      // if this was the last node then we're done
      //
      if (--nodes == 0)
        return;

      //
      // otherwise decrement keys
      //
      keys -= SKC_RASTER_NODE_COUNT_KEYS;
    }
}

//
//
//

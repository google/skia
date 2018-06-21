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
#include "block.h"
#include "raster.h"
#include "atomic_cl.h"
#include "raster_builder_cl_12.h"
#include "kernel_cl_12.h"

//
// INPUT:
//
//   TTRK (64-BIT COMPARE)
//
//    0                                  63
//    | TTSB ID |   X  |   Y  | COHORT ID |
//    +---------+------+------+-----------+
//    |    27   |  12  |  12  |     13    |
//
//
//   TTRK (32-BIT COMPARE)
//
//    0                                        63
//    | TTSB ID | N/A |   X  |   Y  | COHORT ID |
//    +---------+-----+------+------+-----------+
//    |    27   |  5  |  12  |  12  |     8     |
//
//
// OUTPUT:
//
//   TTSK v2:
//
//    0                                     63
//    | TTSB ID | PREFIX |  N/A |  X |  Y |
//    +---------+--------+------+----+----+
//    |    27   | 1 (=0) |  12  | 12 | 12 |
//
//
//   TTPK v1:
//
//    0                                        63
//    | TTPB ID | ALL ZEROES | SPAN |  X  |  Y  |
//    +---------+------------+------+-----+-----+
//    |    27   |      1     |  12  | 12  | 12  |
//
//
//   TTPK v2:
//
//    0                                       63
//    | TTPB ID | PREFIX | SPAN |  X  |  Y  |
//    +---------+--------+------+-----+-----+
//    |    27   | 1 (=1) |  12  | 12  | 12  |
//

#define SKC_PREFIX_SUBGROUP_MASK  (SKC_PREFIX_SUBGROUP_SIZE - 1)

//
// smem accumulator
//

union skc_subgroup_accum
{
  struct {
    SKC_ATOMIC_INT        ttp[SKC_TILE_HEIGHT];
  } atomic;

  struct {
    skc_ttp_t             ttp[SKC_TILE_HEIGHT];
  } aN;

  struct {
    SKC_PREFIX_TTP_V      ttp[SKC_PREFIX_SUBGROUP_SIZE];
  } vN;

  struct {
    SKC_PREFIX_SMEM_ZERO  ttp[SKC_TILE_HEIGHT / SKC_PREFIX_SMEM_ZERO_WIDTH];
  } zero;
};

//
//
//

struct skc_subgroup_smem
{
  // prefix accumulator
  union skc_subgroup_accum accum;
};

//
//
//

static
skc_uint
skc_subgroup_lane()
{
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  return get_sub_group_local_id();
#else
  return 0;
#endif
}

//
//
//

static
SKC_PREFIX_TTS_V_BITFIELD
skc_tts_get_dy(skc_tts_v_t const ttsv)
{
  // tts.dy is packed to fit in range [-32,31] and unpacked to [-32..-1,+1..+32]
  SKC_PREFIX_TTS_V_BITFIELD const dy = ttsv >> SKC_TTS_OFFSET_DY;

  return dy - (~ttsv >> 31);
}

static
SKC_PREFIX_TTS_V_BITFIELD
skc_tts_get_py(skc_tts_v_t const ttsv)
{
  return SKC_BFE(ttsv,SKC_TTS_BITS_TY-SKC_SUBPIXEL_RESL_Y_LOG2,SKC_TTS_OFFSET_TY+SKC_SUBPIXEL_RESL_Y_LOG2);
}

//
//
//

static
void
skc_accum_scatter(__local struct skc_subgroup_smem * const smem, skc_tts_v_t const tts_v)
{
  // get "altitude"
  SKC_PREFIX_TTS_V_BITFIELD dy = skc_tts_get_dy(tts_v);

  // get the y pixel coordinate
  SKC_PREFIX_TTS_V_BITFIELD py = skc_tts_get_py(tts_v);

  //
  // FIXME -- benchmark performance of setting dy to 0 if tts_v is invalid?
  //
  // FIXME -- consider making TTS_INVALID a dy/py/etc. that's a no-op
  //

#if 0
  if (tts_v != SKC_TTS_INVALID)
    printf("< %08X = %u : %d >\n",tts_v,py,dy); 
#endif

  //
  // scatter-add the "altitude" to accumulator
  //
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // GPU/SIMT -- IMPLIES SUPPORT FOR ATOMIC SCATTER-ADD
  //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                         \
  if (tts_v C != SKC_TTS_INVALID) {                                     \
    SKC_ATOMIC_ADD_LOCAL_RELAXED_SUBGROUP(smem->accum.atomic.ttp + py C, dy C); \
  }

#else
  //
  // CPU/SIMD -- ITERATE OVER VECTOR, NO NEED FOR ATOMICS
  //
  // WITH SIMD, ONCE A TTS_INVALID IS DETECTED WE CAN QUIT
  //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                 \
  if (tts_v C == SKC_TTS_INVALID)               \
    return;                                     \
  smem->accum.aN.ttp[py C] = dy C;
#endif

  SKC_PREFIX_TTS_VECTOR_INT_EXPAND();
}

//
// The implication here is that if our device configuration has a
// rectangular 1:2 tile then we need a block size of at least 2
// subblocks. The subblock size of course needs to match the length of
// the smallest tile side.
//

static
void
skc_accum_flush(__local struct skc_subgroup_smem * const smem,
                __global skc_bp_elem_t           * const bp_elems,
                skc_block_id_t                     const pb_id)
{
  // load the ttp elements
  SKC_PREFIX_TTP_V const ttp_v  = smem->accum.vN.ttp[get_sub_group_local_id()];
  skc_uint         const offset = pb_id * (SKC_DEVICE_SUBBLOCK_WORDS / SKC_TILE_RATIO) + skc_subgroup_lane();
  
#if   ( SKC_TILE_RATIO == 1 )

  bp_elems[offset] = ttp_v;

#elif ( SKC_TILE_RATIO == 2 )

  vstore2(ttp_v,offset,bp_elems);

#else

#error("tile ratio greater than 2 not supported")

#endif
}

//
//
//

static
void
skc_accum_reset(__local struct skc_subgroup_smem * const smem)
{
  for (uint ii=0; ii<SKC_TILE_HEIGHT / SKC_PREFIX_SMEM_ZERO_WIDTH / SKC_PREFIX_SUBGROUP_SIZE; ii++)
    smem->accum.zero.ttp[ii * SKC_PREFIX_SUBGROUP_SIZE + skc_subgroup_lane()] = ( 0 );
}

//
// get next sk key
//

static
skc_ttsk_s_t
skc_ttsk_v_get_next(skc_ttsk_v_t * const sk_v,
                    skc_uint     * const sk_next,
                    skc_int      * const rkpk_rem)
{
  // decrement count
  *rkpk_rem -= 1;

#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // SIMT with subgroup support is easy
  //
  // SIMT without subgroup support can always emulate with smem
  //
#if 0
  //
  // BUG TICKLED BY FILTHY CODE -- Intel compiler doesn't properly
  // broadcast a uint2 cast to a long. It was probably bad to do this
  // anyway without a union wrapping the TTSK scalar type.
  //
  // Consider creating a union { ulong; uint2 } at a later date --
  // probably no need to ever do this unless it makes broadcast faster
  // which is unlikely since it will probably be implemented as 2
  // 32-bit broadcasts.
  //
  // Additionally, the TTRK and TTXK key bitfield sizes are probably
  // cast in stone and we aren't going to change them no matter
  // architecture we're on.
  //
  skc_ttsk_s_t sk_s = sub_group_broadcast(SKC_AS(ulong)(*sk_v),(*sk_next)++);
#else
  skc_ttsk_s_t sk_s;

  sk_s.lo   = sub_group_broadcast(sk_v->lo,*sk_next);
  sk_s.hi   = sub_group_broadcast(sk_v->hi,*sk_next);
  *sk_next += 1;
#endif

#else
  //
  // SIMD will always grab component .s0 and then rotate the vector
  //
  sk_s = ( sk_v->s0 );

  skc_ttsk_v_rotate_down(sk_v);

#endif

  return sk_s;
}

//
//
//

static
skc_raster_yx_s
skc_ttsk_v_first(skc_ttsk_v_t * const sk_v, skc_uint const sk_next)
{
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // SIMT with subgroup support is easy
  //
  // SIMT without subgroup support can always emulate with smem
  //
  skc_raster_yx_s const yx_s = sub_group_broadcast(sk_v->hi,sk_next);

#else
  //
  // SIMD will always grab component .s0 and then rotate the vector
  //
  skc_raster_yx_s const yx_s = ( sk_v->s0.hi );

#endif

  return yx_s;
}

//
// mask off ttsb id
//

static
skc_block_id_s_t
skc_ttsk_s_get_ttsb_id(skc_ttsk_s_t const * const sk_s)
{
  return ( sk_s->lo & SKC_TTXK_LO_MASK_ID );
}

//
// load tts_v as early as possible
//

static
skc_tts_v_t
skc_load_tts(__global skc_bp_elem_t * const bp_elems,
             skc_block_id_s_t         const sb_id)
{
  return ( bp_elems[sb_id * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane()] );
}

//
// massage ttrk keys into ttsk keys
//

static
void
skc_ttrk_to_ttsk(skc_ttsk_v_t * const sk_v)
{
  sk_v->lo = sk_v->lo  & SKC_TTXK_LO_MASK_ID;     // clear high (N/A) bits
  sk_v->hi = sk_v->hi << SKC_TTRK_HI_BITS_COHORT; // shift cohort away -- zeroes low bits
}

//
// replenish ttsk keys
//

static
void
skc_ttsk_v_replenish(skc_ttsk_v_t                * const sk_v,
                     skc_uint                    * const sk_next,
                     skc_uint                    * const rks_next,
                     __global skc_ttrk_e_t const * const rks)
{
  // if there are still keys available then return
  if (*sk_next < SKC_PREFIX_TTXK_V_SIZE)
    return;

  //
  // otherwise, replenish sk_v
  //
  // NOTE NOTE NOTE -- we are assuming rks[] extent size is always
  // divisible by TTXK_V_SIZE and therefore loading some keys from the
  // next raster is OK.
  //
  *sk_next   = 0;
  *rks_next += SKC_PREFIX_SUBGROUP_SIZE;
  *sk_v      = rks[*rks_next];

#if 0
  printf("* %08X ( %3u, %3u )\n",
         sk_v->hi,
         (sk_v->hi >> 12) & 0xFFF,
         (sk_v->hi      ) & 0xFFF);
#endif
  
  skc_ttrk_to_ttsk(sk_v);

#if 0
  printf("! %08X ( %3u, %3u )\n",
         sk_v->hi,
         (sk_v->hi >> 20) & 0xFFF,
         (sk_v->hi >>  8) & 0xFFF);
#endif
}

//
// replenish block ids
//
// note that you can't overrun the block id pool since it's a ring
//

static
void
skc_blocks_replenish(skc_uint                      * const blocks_next,
                     skc_uint                      * const blocks_idx,
                     skc_block_id_v_t              * const blocks,
                     skc_uint                        const bp_mask, // pow2 modulo mask for block pool ring
                     __global skc_block_id_t const * const bp_ids)

{
  *blocks_idx += SKC_PREFIX_BLOCK_ID_V_SIZE;
  *blocks      = bp_ids[*blocks_idx & bp_mask];
  *blocks_next = 0;

#if 0
  printf("replenish blocks: %u\n",*blocks);
#endif
}

//
//
//

static
skc_block_id_t
skc_blocks_get_next(skc_uint                      * const blocks_next,
                    skc_uint                      * const blocks_idx,
                    skc_block_id_v_t              * const blocks,
                    skc_uint                        const bp_mask, // pow2 modulo mask for block pool ring
                    __global skc_block_id_t const * const bp_ids)
{
  // replenish?
  if (*blocks_next == SKC_PREFIX_BLOCK_ID_V_SIZE)
    {
      skc_blocks_replenish(blocks_next,blocks_idx,blocks,bp_mask,bp_ids);
    }

#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
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

#if ( SKC_DEVICE_SUBBLOCK_WORDS_LOG2 < SKC_DEVICE_BLOCK_WORDS_LOG2 )

static
skc_block_id_t
skc_subblocks_get_next_pb_id(skc_block_id_t                * const subblocks,
                             skc_uint                      * const blocks_next,
                             skc_uint                      * const blocks_idx,
                             skc_block_id_v_t              * const blocks,
                             skc_uint                        const bp_mask, // pow2 modulo mask for block pool ring
                             __global skc_block_id_t const * const bp_ids)
{
  if ((*subblocks & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK) == 0)
    {
      *subblocks = skc_blocks_get_next(blocks_next,blocks_idx,blocks,bp_mask,bp_ids);
    }

  skc_block_id_t const pb_id = *subblocks;

  *subblocks += SKC_TILE_RATIO; // note this is one or two subblocks

  return pb_id;
}

#endif

//
// append a ttsk key to the work-in-progress node
//

static
void
skc_node_v_append_sk(skc_ttsk_s_t            const * const sk_s,

                     skc_ttxk_v_t                  * const xk_v,
                     skc_uint                      * const xk_v_next,
                     skc_uint                      * const xk_v_idx,
                     __global skc_bp_elem_t        * const bp_elems,

                     skc_int                         const rkpk_rem,

                     skc_uint                      * const blocks_next,
                     skc_uint                      * const blocks_idx,
                     skc_block_id_v_t              * const blocks,
                     skc_uint                        const bp_mask,
                     __global skc_block_id_t const * const bp_ids)
{
  //
  // Append an sk key to the in-register xk_v vector
  //
  // If the work-in-progress node in gmem will only have room for one
  // more key then:
  //
  //   - if this was the final SK then write out xk_v and exit
  //
  //   - otherwise, acquire a block id, link it, write out xk_v,
  //     prepare new node
  //
  // Note that this does *not* try to squeeze in a final key into the
  // next node slot.  This optimization isn't worth the added
  // down-pipeline complexity.
  //
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // SIMT
  //
  if (get_sub_group_local_id() == (*xk_v_next & SKC_PREFIX_TTXK_V_MASK))
    {
      *xk_v = *sk_s;
    }

  *xk_v_next += 1;

  // are there more keys coming?
  if (rkpk_rem > 0)
    {
      // is the node almost full?
      if (*xk_v_next == SKC_RASTER_NODE_DWORDS - 1)
        {
          skc_block_id_t const id = skc_blocks_get_next(blocks_next,blocks_idx,blocks,bp_mask,bp_ids);

          if (get_sub_group_local_id() == SKC_PREFIX_TTXK_V_SIZE - 1)
            {
              xk_v->lo = id;
              xk_v->hi = SKC_UINT_MAX; // this initialization isn't necessary
            }

          // store xk_v (uint2) to bp (uint)
          bp_elems[*xk_v_idx                         ] = xk_v->lo;
          bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v->hi;
#if 0
          printf("S) %u : %08v2X\n",*xk_v_idx,*xk_v);
#endif
          // reinitialize xk_v
          xk_v->lo = SKC_UINT_MAX;
          xk_v->hi = SKC_UINT_MAX;

          // update node elem idx
          *xk_v_idx = id * SKC_DEVICE_SUBBLOCK_WORDS + get_sub_group_local_id();

          // reset node count
          *xk_v_next = 0;
        }
      // is xk_v full?
      else if ((*xk_v_next & SKC_PREFIX_TTXK_V_MASK) == 0)
        {
          // store xk_v to bp
          bp_elems[*xk_v_idx                         ] = xk_v->lo;
          bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v->hi;
#if 0
          printf("s) %u : %08v2X\n",*xk_v_idx,*xk_v);
#endif
          // reinitialize xk_v
          xk_v->lo = SKC_UINT_MAX;
          xk_v->hi = SKC_UINT_MAX;

          // increment node elem idx
          *xk_v_idx += SKC_PREFIX_SUBGROUP_SIZE * 2;
        }
    }
  else
    {
      bp_elems[*xk_v_idx                         ] = xk_v->lo;
      bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v->hi;
#if 0
      printf("z) %u : %08v2X\n",*xk_v_idx,*xk_v);
#endif
      while ((*xk_v_idx & SKC_DEVICE_BLOCK_WORDS_MASK) < SKC_DEVICE_BLOCK_WORDS - SKC_PREFIX_SUBGROUP_SIZE * 2)
        {
          *xk_v_idx += SKC_PREFIX_SUBGROUP_SIZE * 2;

          bp_elems[*xk_v_idx]                          = SKC_UINT_MAX;
          bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = SKC_UINT_MAX;
        }
    }

#else
  //
  // SIMD
  //

#endif
}

//
//
//

static
skc_ttpk_s_t
skc_ttpk_create(skc_raster_yx_s const yx_prev,
                skc_raster_yx_s const yx_next,
                skc_block_id_t  const pb_id)
{
  // - yx_prev is already incremented by one 
  // - yx_span is already shifted up at hi.x
  skc_uint const yx_span = yx_next - yx_prev;

  skc_ttpk_s_t pk;

  // turn on prefix bit | shift span bits upward
  pk.lo = pb_id | SKC_TTXK_LO_MASK_PREFIX | (yx_span << SKC_TTPK_LO_SHL_YX_SPAN);

  // shift down high span bits | yx of tile
  pk.hi = (yx_span >> SKC_TTPK_HI_SHR_YX_SPAN) | yx_prev;

#if 0
  if (get_sub_group_local_id() == 0)
    printf("* %08v2X : %u\n",pk,yx_span);
#endif

  return pk;
}

//
// append a ttpk key to the work-in-progress node
//

static
void
skc_node_v_append_pk(skc_ttpk_s_t            const * const pk_s,

                     skc_ttxk_v_t                  * const xk_v,
                     skc_uint                      * const xk_v_next,
                     skc_uint                      * const xk_v_idx,
                     __global skc_bp_elem_t        * const bp_elems,

                     skc_uint                      * const blocks_next,
                     skc_uint                      * const blocks_idx,
                     skc_block_id_v_t              * const blocks,
                     skc_uint                        const bp_mask,
                     __global skc_block_id_t const * const bp_ids)
{
  //
  // append a pk key to the in-register xk_v vector
  //
  // if the work-in-progress node in gmem will only have room for one
  // more key then:
  //
  //   - if this was the final SK then write out xk_v and exit
  //
  //   - otherwise, acquire a block id, link it, write out xk_v,
  //     prepare new node
  //
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // SIMT
  //
  if (get_sub_group_local_id() == (*xk_v_next & SKC_PREFIX_TTXK_V_MASK))
    {
      *xk_v = *pk_s;
    }

  *xk_v_next += 1;

  // is the node almost full?
  if (*xk_v_next == SKC_RASTER_NODE_DWORDS - 1)
    {
      skc_block_id_t const id = skc_blocks_get_next(blocks_next,blocks_idx,blocks,bp_mask,bp_ids);

      if (get_sub_group_local_id() == SKC_PREFIX_TTXK_V_SIZE - 1)
        {
          xk_v->lo = id;
          xk_v->hi = SKC_UINT_MAX; // this initialization isn't necessary
        }

      // store xk_v to bp
      bp_elems[*xk_v_idx                         ] = xk_v->lo;
      bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v->hi;
#if 0
      printf("P) %u : %08v2X\n",*xk_v_idx,*xk_v);
#endif
      // reinitialize xk_v
      xk_v->lo = SKC_UINT_MAX;
      xk_v->hi = SKC_UINT_MAX;

      // update node elem idx
      *xk_v_idx  = id * SKC_DEVICE_SUBBLOCK_WORDS + get_sub_group_local_id();

      // reset node count
      *xk_v_next = 0;
    }
  // is xk_v full?
  else if ((*xk_v_next & SKC_PREFIX_TTXK_V_MASK) == 0)
    {
      // store xk_v to bp
      bp_elems[*xk_v_idx                         ] = xk_v->lo;
      bp_elems[*xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v->hi;
#if 0
      printf("p) %u : %08v2X\n",*xk_v_idx,*xk_v);
#endif
      // reinitialize xk_v
      xk_v->lo = SKC_UINT_MAX;
      xk_v->hi = SKC_UINT_MAX;
      
      // increment node elem idx
      *xk_v_idx += SKC_PREFIX_SUBGROUP_SIZE * 2;
    }

#else
  //
  // SIMD
  //
#endif
}

//
// append the first 3 fields of meta info to the raster header
//

static
void
skc_node_v_init_header(skc_ttxk_v_t                           * const xk_v,
                       skc_uint                               * const xk_v_next,
                       union skc_raster_cohort_meta_out const * const meta)
{
#if ( SKC_PREFIX_SUBGROUP_SIZE > 1 )
  //
  // SIMT
  //
  if (get_sub_group_local_id() < 2)
    {
      *xk_v = ((get_sub_group_local_id() & 1) == 0) ? meta->u32v4.lo : meta->u32v4.hi;
    }

#if 0
  if (get_sub_group_local_id() == 0)
    printf("header: %08v4X\n",meta->u32v4);
#endif

  //
  // increment counter: uint4 + uint4 = uint2 x 4
  //
  *xk_v_next = 2 + 2; // +2 for unitialized bounds

#else
  //
  // SIMD
  //

#endif
}

//
//
//

__kernel
SKC_PREFIX_KERNEL_ATTRIBS
void
skc_kernel_prefix(__global skc_uint       const * const bp_atomics,
                  __global skc_block_id_t const * const bp_ids,
                  __global skc_bp_elem_t        * const bp_elems,
                  skc_uint                        const bp_mask, // pow2 modulo mask for block pool ring
                  __global skc_ttrk_e_t   const * const rks,
                  __global skc_block_id_t       * const map,
                  __global skc_uint       const * const metas,
                  skc_uint                        const count)
{
  //
  // declare shared memory block
  //
#if ( SKC_PREFIX_WORKGROUP_SUBGROUPS == 1 )
  __local struct skc_subgroup_smem                  smem[1];
#else
  __local struct skc_subgroup_smem                  smems[SKC_PREFIX_WORKGROUP_SUBGROUPS];
  __local struct skc_subgroup_smem * restrict const smem = smems + get_sub_group_id();
#endif

  //
  // where is this subgroup in the grid?
  //
#if ( SKC_PREFIX_WORKGROUP_SUBGROUPS == 1 )
  skc_uint const sgi = get_group_id(0);
#else
  skc_uint const sgi = get_group_id(0) * SKC_PREFIX_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  skc_uint const sgl = get_sub_group_local_id();

  //
  // return if this subgroup is excess
  //
#if ( SKC_PREFIX_WORKGROUP_SUBGROUPS > 1 )
  if (sgi >= count)
    return;
#endif

  //
  // get meta info for this subgroup's raster
  //
  union skc_raster_cohort_meta_out const meta  = { vload4(sgi,metas) };
  skc_uint                         const reads = metas[SKC_RASTER_COHORT_META_OFFSET_READS + sgi];

#if 0
  if (get_sub_group_local_id() == 0)
    printf("%3u : %5u / %5u / %5u / %5u / %u\n",
           sgi,
           meta.blocks,
           meta.offset,
           meta.nodes,
           meta.keys,
           reads);
#endif

  //
  // preload blocks -- align on subgroup
  //
  skc_uint         blocks_idx  = (reads & ~SKC_PREFIX_SUBGROUP_MASK) + skc_subgroup_lane();
  skc_block_id_v_t blocks      = bp_ids[blocks_idx & bp_mask];
  skc_uint         blocks_next = (reads &  SKC_PREFIX_SUBGROUP_MASK);

  //
  // prime xk_v_idx with a block but note that OpenCL vstore_n() will scale the offset
  //
  skc_uint xk_v_idx = sub_group_broadcast(blocks,blocks_next++) * SKC_DEVICE_SUBBLOCK_WORDS + get_sub_group_local_id();

  //
  // initialize raster header -- assumes block is greater than 8 words (4 doublewords)
  //
  skc_ttxk_v_t xk_v = { SKC_UINT_MAX, SKC_UINT_MAX };
  skc_uint     xk_v_next;

  skc_node_v_init_header(&xk_v,&xk_v_next,&meta);

  //
  // no keys -- this is an empty raster!
  //
  if (meta.keys == 0)
    {
      bp_elems[xk_v_idx                         ] = xk_v.lo;
      bp_elems[xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = xk_v.hi;

      while ((xk_v_idx & SKC_DEVICE_BLOCK_WORDS_MASK) < SKC_DEVICE_BLOCK_WORDS - SKC_PREFIX_SUBGROUP_SIZE * 2)
        {
          xk_v_idx += SKC_PREFIX_SUBGROUP_SIZE * 2;

          bp_elems[xk_v_idx]                          = SKC_UINT_MAX;
          bp_elems[xk_v_idx+SKC_PREFIX_SUBGROUP_SIZE] = SKC_UINT_MAX;
        }

      return;
    }

  //
  // load TTRK keys and in-place convert to TTSK keys
  //
  skc_uint         rks_next    = (meta.offset & ~SKC_PREFIX_SUBGROUP_MASK) + skc_subgroup_lane();
  skc_ttsk_v_t     sk_v        = rks[rks_next];
  skc_uint         sk_next     = (meta.offset & SKC_PREFIX_SUBGROUP_MASK);
  skc_int          rkpk_rem    = meta.keys; // signed count of remaining rk+pk keys

#if 0
  printf("* %08X ( %3u, %3u )\n",
         sk_v.hi,
         (sk_v.hi >> 12) & 0xFFF,
         (sk_v.hi      ) & 0xFFF);
#endif
  
  skc_ttrk_to_ttsk(&sk_v);

#if 0
  printf("! %08X ( %3u, %3u )\n",
         sk_v.hi,
         (sk_v.hi >> 20) & 0xFFF,
         (sk_v.hi >>  8) & 0xFFF);
#endif

  //
  // subblocks
  //
#if ( SKC_DEVICE_SUBBLOCK_WORDS_LOG2 < SKC_DEVICE_BLOCK_WORDS_LOG2 )
  skc_block_id_t subblocks = 0;
#endif

  //
  // begin "scan" of tiles
  //
  skc_raster_yx_s yx_prev = skc_ttsk_v_first(&sk_v,sk_next);

  //
  // zero the accumulator
  //
  skc_accum_reset(smem);

  while (true)
    {
      // get next rk key
      skc_ttsk_s_t     const sk_s  = skc_ttsk_v_get_next(&sk_v,&sk_next,&rkpk_rem);

      // load ttsb id
      skc_block_id_s_t const sb_id = skc_ttsk_s_get_ttsb_id(&sk_s);

      // load tts_v transaction "in flight" as early as possible
      skc_tts_v_t      const tts_v = skc_load_tts(bp_elems,sb_id);

#if 0
      printf("{ %08X }\n",tts_v);
#endif

#if 0
      if (get_sub_group_local_id() == 0)
        printf("[ %d, %X ]\n",rkpk_rem,sb_id);
#endif

#if 0
      if (get_sub_group_local_id() == 0)
        printf("@ %08X ( %3u, %3u )\n",sk_s.hi,(sk_s.hi >> 20),(sk_s.hi >> 8) & 0xFFF);
#endif

      //
      // FIXME -- SOME OF THESE COMPARISONS CAN BE PERFORMED AHEAD OF
      // TIME AND SIMD'IZED
      //

      // if yx's don't match then we're either issuing a ttpk or
      // resetting the accumulator
      if (sk_s.hi != yx_prev)
        {
          // if yx_next.y == yx_last.y then x changed
          if (((sk_s.hi ^ yx_prev) & SKC_TTXK_HI_MASK_Y) == 0)
            {
              //
              // if the tile is not square then it's ratio is 1:2
              //
#if SKC_DEVICE_SUBBLOCK_WORDS_LOG2 < SKC_DEVICE_BLOCK_WORDS_LOG2
              skc_block_id_t const pb_id = skc_subblocks_get_next_pb_id(&subblocks,
                                                                        &blocks_next,
                                                                        &blocks_idx,
                                                                        &blocks,
                                                                        bp_mask,
                                                                        bp_ids);
#else
              skc_block_id_t const pb_id = skc_blocks_get_next(&blocks_next,
                                                               &blocks_idx,
                                                               &blocks,
                                                               bp_mask,
                                                               bp_ids);
#endif

              // flush accumulated ttp vector to block/subblock at ttpb_id
              skc_accum_flush(smem,bp_elems,pb_id);

#if 0
              if (get_sub_group_local_id() == 0)
                {
                  printf("%8u : ( %4u, %4u ) -> ( %4u, %4u )\n",
                         pb_id,
                         (yx_prev >> SKC_TTXK_HI_OFFSET_Y),
                         (yx_prev >> SKC_TTXK_HI_OFFSET_X) & 0xFFF,
                         (sk_s.hi >> SKC_TTXK_HI_OFFSET_Y) & 0xFFF,
                         (sk_s.hi >> SKC_TTXK_HI_OFFSET_X) & 0xFFF);
                }
#endif

              //
              // FIXME -- A SIMD-WIDE BLOCK OF TTPK KEYS CAN BE CREATED IN ONE STEP
              //
              rkpk_rem -= 1;

              // create the pk
              skc_ttpk_s_t const pk_s = skc_ttpk_create(yx_prev+SKC_TTXK_HI_ONE_X,sk_s.hi,pb_id);

              // append pk key to xk buffer
              skc_node_v_append_pk(&pk_s,

                                   &xk_v,
                                   &xk_v_next,
                                   &xk_v_idx,
                                   bp_elems,

                                   &blocks_next,
                                   &blocks_idx,
                                   &blocks,
                                   bp_mask,
                                   bp_ids);
            }
          else if (rkpk_rem > 0) // we're starting a new tile row
            {
              skc_accum_reset(smem);
            }
        }

      //
      // append sk key to node_v
      //
      // if rkpk_rem is zero then return from kernel
      //
      skc_node_v_append_sk(&sk_s,

                           &xk_v,
                           &xk_v_next,
                           &xk_v_idx,
                           bp_elems,

                           rkpk_rem,

                           &blocks_next,
                           &blocks_idx,
                           &blocks,
                           bp_mask,
                           bp_ids);

      // we're done if no more sk keys
      if (rkpk_rem == 0)
        break;

      // move to new tile
      yx_prev = sk_s.hi;

      // scatter tts values into accumulator
      skc_accum_scatter(smem,tts_v);

      // replenish sk keys
      skc_ttsk_v_replenish(&sk_v,&sk_next,&rks_next,rks);
    }
}

//
//
//

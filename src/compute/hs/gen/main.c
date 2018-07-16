/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

//
//
//

#include "networks.h"
#include "common/util.h"
#include "common/macros.h"

//
//
//

#undef  HSG_OP_EXPAND_X
#define HSG_OP_EXPAND_X(t) #t ,

char const * const
hsg_op_type_string[] =
  {
    HSG_OP_EXPAND_ALL()
  };

//
//
//

#define EXIT()                            (struct hsg_op){ HSG_OP_TYPE_EXIT                                     }

#define END()                             (struct hsg_op){ HSG_OP_TYPE_END                                      }
#define BEGIN()                           (struct hsg_op){ HSG_OP_TYPE_BEGIN                                    }
#define ELSE()                            (struct hsg_op){ HSG_OP_TYPE_ELSE                                     }

#define TARGET_BEGIN()                    (struct hsg_op){ HSG_OP_TYPE_TARGET_BEGIN                             }
#define TARGET_END()                      (struct hsg_op){ HSG_OP_TYPE_TARGET_END                               }

#define TRANSPOSE_KERNEL_PROTO()          (struct hsg_op){ HSG_OP_TYPE_TRANSPOSE_KERNEL_PROTO                   }
#define TRANSPOSE_KERNEL_PREAMBLE()       (struct hsg_op){ HSG_OP_TYPE_TRANSPOSE_KERNEL_PREAMBLE                }
#define TRANSPOSE_KERNEL_BODY()           (struct hsg_op){ HSG_OP_TYPE_TRANSPOSE_KERNEL_BODY                    }

#define BS_KERNEL_PROTO(i)                (struct hsg_op){ HSG_OP_TYPE_BS_KERNEL_PROTO,             { i       } }
#define BS_KERNEL_PREAMBLE(i)             (struct hsg_op){ HSG_OP_TYPE_BS_KERNEL_PREAMBLE,          { i       } }

#define BC_KERNEL_PROTO(i)                (struct hsg_op){ HSG_OP_TYPE_BC_KERNEL_PROTO,             { i       } }
#define BC_KERNEL_PREAMBLE(i)             (struct hsg_op){ HSG_OP_TYPE_BC_KERNEL_PREAMBLE,          { i       } }

#define FM_KERNEL_PROTO(s,r)              (struct hsg_op){ HSG_OP_TYPE_FM_KERNEL_PROTO,             { s, r    } }
#define FM_KERNEL_PREAMBLE(h)             (struct hsg_op){ HSG_OP_TYPE_FM_KERNEL_PREAMBLE,          { h       } }

#define HM_KERNEL_PROTO(s)                (struct hsg_op){ HSG_OP_TYPE_HM_KERNEL_PROTO,             { s       } }
#define HM_KERNEL_PREAMBLE(h)             (struct hsg_op){ HSG_OP_TYPE_HM_KERNEL_PREAMBLE,          { h       } }

#define BX_REG_GLOBAL_LOAD(n,v)           (struct hsg_op){ HSG_OP_TYPE_BX_REG_GLOBAL_LOAD,          { n, v    } }
#define BX_REG_GLOBAL_STORE(n)            (struct hsg_op){ HSG_OP_TYPE_BX_REG_GLOBAL_STORE,         { n       } }

#define FM_REG_GLOBAL_LOAD_LEFT(n,i)      (struct hsg_op){ HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_LEFT,     { n, i    } }
#define FM_REG_GLOBAL_STORE_LEFT(n,i)     (struct hsg_op){ HSG_OP_TYPE_FM_REG_GLOBAL_STORE_LEFT,    { n, i    } }
#define FM_REG_GLOBAL_LOAD_RIGHT(n,i)     (struct hsg_op){ HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_RIGHT,    { n, i    } }
#define FM_REG_GLOBAL_STORE_RIGHT(n,i)    (struct hsg_op){ HSG_OP_TYPE_FM_REG_GLOBAL_STORE_RIGHT,   { n, i    } }
#define FM_MERGE_RIGHT_PRED(n,s)          (struct hsg_op){ HSG_OP_TYPE_FM_MERGE_RIGHT_PRED,         { n, s    } }

#define HM_REG_GLOBAL_LOAD(n,i)           (struct hsg_op){ HSG_OP_TYPE_HM_REG_GLOBAL_LOAD,          { n, i    } }
#define HM_REG_GLOBAL_STORE(n,i)          (struct hsg_op){ HSG_OP_TYPE_HM_REG_GLOBAL_STORE,         { n, i    } }

#define SLAB_FLIP(f)                      (struct hsg_op){ HSG_OP_TYPE_SLAB_FLIP,                   { f       } }
#define SLAB_HALF(h)                      (struct hsg_op){ HSG_OP_TYPE_SLAB_HALF,                   { h       } }

#define CMP_FLIP(a,b,c)                   (struct hsg_op){ HSG_OP_TYPE_CMP_FLIP,                    { a, b, c } }
#define CMP_HALF(a,b)                     (struct hsg_op){ HSG_OP_TYPE_CMP_HALF,                    { a, b    } }

#define CMP_XCHG(a,b,p)                   (struct hsg_op){ HSG_OP_TYPE_CMP_XCHG,                    { a, b, p } }

#define BS_REG_SHARED_STORE_V(m,i,r)      (struct hsg_op){ HSG_OP_TYPE_BS_REG_SHARED_STORE_V,       { m, i, r } }
#define BS_REG_SHARED_LOAD_V(m,i,r)       (struct hsg_op){ HSG_OP_TYPE_BS_REG_SHARED_LOAD_V,        { m, i, r } }
#define BC_REG_SHARED_LOAD_V(m,i,r)       (struct hsg_op){ HSG_OP_TYPE_BC_REG_SHARED_LOAD_V,        { m, i, r } }

#define BX_REG_SHARED_STORE_LEFT(r,i,p)   (struct hsg_op){ HSG_OP_TYPE_BX_REG_SHARED_STORE_LEFT,    { r, i, p } }
#define BS_REG_SHARED_STORE_RIGHT(r,i,p)  (struct hsg_op){ HSG_OP_TYPE_BS_REG_SHARED_STORE_RIGHT,   { r, i, p } }

#define BS_REG_SHARED_LOAD_LEFT(r,i,p)    (struct hsg_op){ HSG_OP_TYPE_BS_REG_SHARED_LOAD_LEFT,     { r, i, p } }
#define BS_REG_SHARED_LOAD_RIGHT(r,i,p)   (struct hsg_op){ HSG_OP_TYPE_BS_REG_SHARED_LOAD_RIGHT,    { r, i, p } }

#define BC_REG_GLOBAL_LOAD_LEFT(r,i,p)    (struct hsg_op){ HSG_OP_TYPE_BC_REG_GLOBAL_LOAD_LEFT,     { r, i, p } }

#define REG_F_PREAMBLE(s)                 (struct hsg_op){ HSG_OP_TYPE_REG_F_PREAMBLE,              { s       } }
#define REG_SHARED_STORE_F(r,i,s)         (struct hsg_op){ HSG_OP_TYPE_REG_SHARED_STORE_F,          { r, i, s } }
#define REG_SHARED_LOAD_F(r,i,s)          (struct hsg_op){ HSG_OP_TYPE_REG_SHARED_LOAD_F,           { r, i, s } }
#define REG_GLOBAL_STORE_F(r,i,s)         (struct hsg_op){ HSG_OP_TYPE_REG_GLOBAL_STORE_F,          { r, i, s } }

#define BLOCK_SYNC()                      (struct hsg_op){ HSG_OP_TYPE_BLOCK_SYNC                               }

#define BS_FRAC_PRED(m,w)                 (struct hsg_op){ HSG_OP_TYPE_BS_FRAC_PRED,                { m, w    } }

#define BS_MERGE_H_PREAMBLE(i)            (struct hsg_op){ HSG_OP_TYPE_BS_MERGE_H_PREAMBLE,         { i       } }
#define BC_MERGE_H_PREAMBLE(i)            (struct hsg_op){ HSG_OP_TYPE_BC_MERGE_H_PREAMBLE,         { i       } }

#define BX_MERGE_H_PRED(p)                (struct hsg_op){ HSG_OP_TYPE_BX_MERGE_H_PRED,             { p       } }

#define BS_ACTIVE_PRED(m,l)               (struct hsg_op){ HSG_OP_TYPE_BS_ACTIVE_PRED,              { m, l    } }

//
// DEFAULTS
//

static
struct hsg_config hsg_config =
  {
    .merge  = {
      .flip = {
        .lo         = 1,
        .hi         = 1
      },
      .half =  {
        .lo         = 1,
        .hi         = 1
      },
    },

    .block  = {
      .warps_min    = 1,          // min warps for a block that uses smem barriers
      .warps_max    = UINT32_MAX, // max warps for the entire multiprocessor
      .warps_mod    = 2,          // the number of warps necessary to load balance horizontal merging

      .smem_min     = 0,
      .smem_quantum = 1,

      .smem_bs      = 49152,
      .smem_bc      = UINT32_MAX  // implies field not set
    },

    .warp   = {
      .lanes        = 32,
      .lanes_log2   = 5,
    },

    .thread = {
      .regs         = 24,
      .xtra         = 0
    },

    .type   = {
      .words        = 2
    }
  };

//
// ZERO HSG_MERGE STRUCT
//

static
struct hsg_merge hsg_merge[MERGE_LEVELS_MAX_LOG2] = { 0 };

//
// STATS ON INSTRUCTIONS
//

static hsg_op_type hsg_op_type_counts[HSG_OP_TYPE_COUNT] = { 0 };

//
//
//

static
void
hsg_op_debug()
{
  uint32_t total = 0;

  for (hsg_op_type t=HSG_OP_TYPE_EXIT; t<HSG_OP_TYPE_COUNT; t++)
    {
      uint32_t const count = hsg_op_type_counts[t];

      total += count;

      fprintf(stderr,"%-37s : %u\n",hsg_op_type_string[t],count);
    }

  fprintf(stderr,"%-37s : %u\n\n\n","TOTAL",total);
}

//
//
//

static
void
hsg_config_init_shared()
{
  //
  // The assumption here is that a proper smem_bs value was provided
  // that represents the maximum fraction of the multiprocessor's
  // available shared memory that can be accessed by the initial block
  // sorting kernel.
  //
  // With CUDA devices this is 48KB out of 48KB, 64KB or 96KB.
  //
  // Intel subslices are a little trickier and the minimum allocation
  // is 4KB and the maximum is 64KB on pre-Skylake IGPs.  Sizes are
  // allocated in 1KB increments.  If a maximum of two block sorters
  // can occupy a subslice then each should be assigned 32KB of shared
  // memory.
  //
  // News Flash: apparently GEN9+ IGPs can allocate 1KB of SMEM per
  // workgroup so all the previously written logic to support this
  // issue is being removed.
  //
  uint32_t const bs_keys = hsg_config.block.smem_bs / (hsg_config.type.words * sizeof(uint32_t));

  hsg_config.warp.skpw_bs = bs_keys / hsg_merge[0].warps;
}

static
void
hsg_merge_levels_init_shared(struct hsg_merge * const merge)
{
  {
    //
    // What is the max amount of shared in each possible bs block config?
    //
    // The provided smem_bs size will be allocated for each sorting block.
    //
    uint32_t const bs_threads   = merge->warps << hsg_config.warp.lanes_log2;
    uint32_t const bs_keys      = hsg_config.block.smem_bs / (hsg_config.type.words * sizeof(uint32_t));
    uint32_t const bs_kpt       = bs_keys / bs_threads;
    uint32_t const bs_kpt_mod   = (bs_kpt / hsg_config.block.warps_mod) * hsg_config.block.warps_mod;
    uint32_t const bs_rows_even = bs_kpt_mod & ~1; // must be even because flip merge only works on row pairs

    // this is a showstopper
    if (bs_rows_even < 2)
      {
        fprintf(stderr,"Error: need at least 2 rows of shared memory.\n");
        exit(-1);
      }

    // clamp to number of registers
    merge->rows_bs = MIN_MACRO(bs_rows_even, hsg_config.thread.regs);
  }

  //
  // smem key allocation rule for BC kernels is that a single block
  // can't allocate more than smem_bs and must allocate at least
  // smem_min in smem_quantum steps.
  //
  // Note that BC blocks will always be less than or equal to BS
  // blocks.
  //
  {
    //
    // if merge->warps is not pow2 then we're going to skip creating a bc elsewhere
    //
    uint32_t const bc_warps_min  = MAX_MACRO(merge->warps,hsg_config.block.warps_min);
    uint32_t const bc_threads    = bc_warps_min << hsg_config.warp.lanes_log2;
    uint32_t const bc_block_rd   = (((hsg_config.block.smem_bc * bc_warps_min) / hsg_config.block.warps_max) /
                                    hsg_config.block.smem_quantum) * hsg_config.block.smem_quantum;
    uint32_t const bc_block_max  = MAX_MACRO(bc_block_rd,hsg_config.block.smem_min);
    uint32_t const bc_block_smem = MIN_MACRO(bc_block_max,hsg_config.block.smem_bs);

    // what is the max amount of shared in each possible bc block config?
    uint32_t const bc_keys       = bc_block_smem / (hsg_config.type.words * sizeof(uint32_t));
    uint32_t const bc_kpt        = bc_keys / bc_threads;
    uint32_t const bc_kpt_mod    = (bc_kpt / hsg_config.block.warps_mod) * hsg_config.block.warps_mod;

    merge->rows_bc = MIN_MACRO(bc_kpt_mod, hsg_config.thread.regs);
    merge->skpw_bc = bc_keys / bc_warps_min;
  }
}

//
//
//

static
void
hsg_merge_levels_init_1(struct hsg_merge * const merge, uint32_t const warps, uint32_t const level, uint32_t const offset)
{
  uint32_t const even_odd = warps & 1;

  merge->levels[level].evenodds[even_odd]++;
  merge->levels[level].networks[even_odd] = warps;

  if (warps == 1)
    return;

  merge->levels[level].active.b64 |= BITS_TO_MASK_AT_64(warps,offset);

  uint32_t const count = merge->levels[level].count++;
  uint32_t const index = (1 << level) + count;
  uint32_t const bit   = 1 << count;

  merge->levels[level].evenodd_masks[even_odd] |= bit;

  if (count > 0)
    {
      // offset from network to left of this network
      uint32_t const diff   = offset - merge->offsets[index-1];

      uint32_t const diff_0 = merge->levels[level].diffs[0];
      uint32_t const diff_1 = merge->levels[level].diffs[1];

      uint32_t diff_idx = UINT32_MAX;

      if        ((diff_0 == 0) || (diff_0 == diff)) {
        diff_idx = 0;
      } else if ((diff_1 == 0) || (diff_1 == diff)) {
        diff_idx = 1;
      } else {
        fprintf(stderr, "*** MORE THAN TWO DIFFS ***\n");
        exit(-1);
      }

      merge->levels[level].diffs     [diff_idx]  = diff;
      merge->levels[level].diff_masks[diff_idx] |= 1 << (count-1);
    }

  merge->networks[index] = warps;
  merge->offsets [index] = offset;

  uint32_t const l = (warps+1)/2; // lower/larger  on left
  uint32_t const r = (warps+0)/2; // higher/smaller on right

  hsg_merge_levels_init_1(merge,l,level+1,offset);
  hsg_merge_levels_init_1(merge,r,level+1,offset+l);
}

static
void
hsg_merge_levels_debug(struct hsg_merge * const merge)
{
  for (uint32_t level=0; level<MERGE_LEVELS_MAX_LOG2; level++)
    {
      uint32_t count = merge->levels[level].count;

      if (count == 0)
        break;

      fprintf(stderr,
              "%-4u : %016llX \n",
              count,
              merge->levels[level].active.b64);

      fprintf(stderr,
              "%-4u : %08X (%2u)\n"
              "%-4u : %08X (%2u)\n",
              merge->levels[level].diffs[0],
              merge->levels[level].diff_masks[0],
              __popcnt(merge->levels[level].diff_masks[0]),
              merge->levels[level].diffs[1],
              merge->levels[level].diff_masks[1],
              __popcnt(merge->levels[level].diff_masks[1]));

      fprintf(stderr,
              "EVEN : %08X (%2u)\n"
              "ODD  : %08X (%2u)\n",
              merge->levels[level].evenodd_masks[0],
              __popcnt(merge->levels[level].evenodd_masks[0]),
              merge->levels[level].evenodd_masks[1],
              __popcnt(merge->levels[level].evenodd_masks[1]));

      for (uint32_t ii=0; ii<2; ii++)
        {
          if (merge->levels[level].networks[ii] > 1)
            {
              fprintf(stderr,
                      "%-4s : ( %2u x %2u )\n",
                      (ii == 0) ? "EVEN" : "ODD",
                      merge->levels[level].evenodds[ii],
                      merge->levels[level].networks[ii]);
            }
        }

      uint32_t index = 1 << level;

      while (count-- > 0)
        {
          fprintf(stderr,
                  "[ %2u %2u ] ",
                  merge->offsets [index],
                  merge->networks[index]);

          index += 1;
        }

      fprintf(stderr,"\n\n");
    }
}

static
void
hsg_merge_levels_hint(struct hsg_merge * const merge, bool const autotune)
{
  // clamp against merge levels
  for (uint32_t level=0; level<MERGE_LEVELS_MAX_LOG2; level++)
    {
      // max network
      uint32_t const n_max = MAX_MACRO(merge->levels[level].networks[0],
                                 merge->levels[level].networks[1]);

      if (n_max <= (merge->rows_bs + hsg_config.thread.xtra))
        break;

      if (autotune)
        {
          hsg_config.thread.xtra = n_max - merge->rows_bs;

          uint32_t const r_total = hsg_config.thread.regs + hsg_config.thread.xtra;
          uint32_t const r_limit = (hsg_config.type.words == 1) ? 120 : 58;

          if (r_total <= r_limit)
            {
              fprintf(stderr,"autotune: %u + %u\n",
                      hsg_config.thread.regs,
                      hsg_config.thread.xtra);
              break;
            }
          else
            {
              fprintf(stderr,"skipping autotune: %u + %u > %u\n",
                      hsg_config.thread.regs,
                      hsg_config.thread.xtra,
                      r_limit);
              exit(-1);
            }
        }

      fprintf(stderr,"*** HINT *** Try extra registers: %u\n",
              n_max - merge->rows_bs);

      exit(-1);
    }
}

//
//
//

static
struct hsg_op *
hsg_op(struct hsg_op * ops, struct hsg_op const opcode)
{
  hsg_op_type_counts[opcode.type] += 1;

  *ops = opcode;

  return ops+1;
}

static
struct hsg_op *
hsg_exit(struct hsg_op * ops)
{
  return hsg_op(ops,EXIT());
}

static
struct hsg_op *
hsg_end(struct hsg_op * ops)
{
  return hsg_op(ops,END());
}

static
struct hsg_op *
hsg_begin(struct hsg_op * ops)
{
  return hsg_op(ops,BEGIN());
}

static
struct hsg_op *
hsg_else(struct hsg_op * ops)
{
  return hsg_op(ops,ELSE());
}

static
struct hsg_op *
hsg_network_copy(struct hsg_op            *       ops,
                 struct hsg_network const * const nets,
                 uint32_t                   const idx,
                 uint32_t                   const prefix)
{
  uint32_t              const len = nets[idx].length;
  struct hsg_op const * const cxa = nets[idx].network;

  for (uint32_t ii=0; ii<len; ii++)
    {
      struct hsg_op const * const cx = cxa + ii;

      ops = hsg_op(ops,CMP_XCHG(cx->a,cx->b,prefix));
    }

  return ops;
}

static
struct hsg_op *
hsg_thread_sort(struct hsg_op * ops)
{
  uint32_t const idx = hsg_config.thread.regs / 2 - 1;

  return hsg_network_copy(ops,hsg_networks_sorting,idx,UINT32_MAX);
}

static
struct hsg_op *
hsg_thread_merge_prefix(struct hsg_op * ops, uint32_t const network, uint32_t const prefix)
{
  if (network <= 1)
    return ops;

  return hsg_network_copy(ops,hsg_networks_merging,network-2,prefix);
}

static
struct hsg_op *
hsg_thread_merge(struct hsg_op * ops, uint32_t const network)
{
  return hsg_thread_merge_prefix(ops,network,UINT32_MAX);
}

static
struct hsg_op *
hsg_thread_merge_offset_prefix(struct hsg_op * ops, uint32_t const offset, uint32_t const network, uint32_t const prefix)
{
  if (network <= 1)
    return ops;

  uint32_t                  const idx = network - 2;
  uint32_t                  const len = hsg_networks_merging[idx].length;
  struct hsg_op const * const cxa = hsg_networks_merging[idx].network;

  for (uint32_t ii=0; ii<len; ii++)
    {
      struct hsg_op const * const cx = cxa + ii;

      ops = hsg_op(ops,CMP_XCHG(offset + cx->a,offset + cx->b,prefix));
    }

  return ops;
}

static
struct hsg_op *
hsg_thread_merge_offset(struct hsg_op * ops, uint32_t const offset, uint32_t const network)
{
  return hsg_thread_merge_offset_prefix(ops,offset,network,UINT32_MAX);
}

static
struct hsg_op *
hsg_thread_merge_left_right_prefix(struct hsg_op * ops, uint32_t const left, uint32_t const right, uint32_t const prefix)
{
  for (uint32_t l=left,r=left+1; r<=left+right; l--,r++)
    {
      ops = hsg_op(ops,CMP_XCHG(l,r,prefix));
    }

  return ops;
}

static
struct hsg_op *
hsg_thread_merge_left_right(struct hsg_op * ops, uint32_t const left, uint32_t const right)
{
  return hsg_thread_merge_left_right_prefix(ops,left,right,UINT32_MAX);
}

static
struct hsg_op *
hsg_warp_half_network(struct hsg_op * ops)
{
  uint32_t const n = hsg_config.thread.regs;

  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,CMP_HALF(r-1,r));

  return ops;
}

static
struct hsg_op *
hsg_warp_half_downto(struct hsg_op * ops, uint32_t h)
{
  //
  // *** from h: downto[f/2,1)
  // **** lane_half(h)
  //
  for (; h > 1; h/=2)
    {
      ops = hsg_begin(ops);

      ops = hsg_op(ops,SLAB_HALF(h));
      ops = hsg_warp_half_network(ops);

      ops = hsg_end(ops);
    }

  return ops;
}

static
struct hsg_op *
hsg_warp_flip_network(struct hsg_op * ops)
{
  uint32_t const n = hsg_config.thread.regs;

  for (uint32_t r=1; r<=n/2; r++)
    ops = hsg_op(ops,CMP_FLIP(r-1,r,n+1-r));

  return ops;
}

static
struct hsg_op *
hsg_warp_flip(struct hsg_op * ops, uint32_t f)
{
  ops = hsg_begin(ops);

  ops = hsg_op(ops,SLAB_FLIP(f));
  ops = hsg_warp_flip_network(ops);

  ops = hsg_end(ops);

  return ops;
}

static
struct hsg_op *
hsg_bx_warp_load(struct hsg_op * ops, const int32_t vin_or_vout)
{
  uint32_t const n = hsg_config.thread.regs;

  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,BX_REG_GLOBAL_LOAD(r,vin_or_vout));

  return ops;
}

static
struct hsg_op *
hsg_bx_warp_store(struct hsg_op * ops)
{
  uint32_t const n = hsg_config.thread.regs;

  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,BX_REG_GLOBAL_STORE(r));

  return ops;
}

//
//
//

static
struct hsg_op *
hsg_warp_transpose(struct hsg_op * ops)
{
  // func proto
  ops = hsg_op(ops,TRANSPOSE_KERNEL_PROTO());

  // begin
  ops = hsg_begin(ops);

  // preamble
  ops = hsg_op(ops,TRANSPOSE_KERNEL_PREAMBLE());

  // load
  ops = hsg_bx_warp_load(ops,1); // 1 = load from vout[]

  // emit transpose blend and remap macros ...
  ops = hsg_op(ops,TRANSPOSE_KERNEL_BODY());

  // ... done!
  ops = hsg_end(ops);

  return ops;
}

//
//
//

static
struct hsg_op *
hsg_warp_half(struct hsg_op * ops, uint32_t const h)
{
  //
  // *** from h: downto[f/2,1)
  // **** lane_half(h)
  // *** thread_merge
  //
  ops = hsg_warp_half_downto(ops,h);
  ops = hsg_thread_merge(ops,hsg_config.thread.regs);

  return ops;
}

static
struct hsg_op *
hsg_warp_merge(struct hsg_op * ops)
{
  //
  // * from f: upto[2,warp.lanes]
  // ** lane_flip(f)
  // *** from h: downto[f/2,1)
  // **** lane_half(h)
  // *** thread_merge
  //
  uint32_t const level = hsg_config.warp.lanes;

  for (uint32_t f=2; f<=level; f*=2)
    {
      ops = hsg_warp_flip(ops,f);
      ops = hsg_warp_half(ops,f/2);
    }

  return ops;
}

//
//
//

static
struct hsg_op *
hsg_bc_half_merge_level(struct hsg_op          *       ops,
                        struct hsg_merge const * const merge,
                        uint32_t                 const r_lo,
                        uint32_t                 const s_count)
{
  // guaranteed to be an even network
  uint32_t const net_even = merge->levels[0].networks[0];

  // min of warps in block and remaining horizontal rows
  uint32_t const active = MIN_MACRO(s_count, net_even);

  // conditional on blockIdx.x
  if (active < merge->warps)
    ops = hsg_op(ops,BX_MERGE_H_PRED(active)); // FIXME BX_MERGE

  // body begin
  ops = hsg_begin(ops);

  // scale for min block
  uint32_t const scale = net_even >= hsg_config.block.warps_min ? 1 : hsg_config.block.warps_min / net_even;

  // loop if more smem rows than warps
  for (uint32_t rr=0; rr<s_count; rr+=active)
    {
      // body begin
      ops = hsg_begin(ops);

      // skip down slab
      uint32_t const gmem_base = r_lo - 1 + rr;

      // load registers horizontally -- striding across slabs
      for (uint32_t ll=1; ll<=net_even; ll++)
        ops = hsg_op(ops,BC_REG_GLOBAL_LOAD_LEFT(ll,gmem_base+(ll-1)*hsg_config.thread.regs,0));

      // merge all registers
      ops = hsg_thread_merge_prefix(ops,net_even,0);

      // if we're looping then there is a base
      uint32_t const smem_base = rr * net_even * scale;

      // store all registers
      for (uint32_t ll=1; ll<=net_even; ll++)
        ops = hsg_op(ops,BX_REG_SHARED_STORE_LEFT(ll,smem_base+ll-1,0));

      // body end
      ops = hsg_end(ops);
    }

  // body end
  ops = hsg_end(ops);

  return ops;
}

static
struct hsg_op *
hsg_bc_half_merge(struct hsg_op * ops, struct hsg_merge const * const merge)
{
  //
  // will only be called with merge->warps >= 2
  //
  uint32_t const warps    = MAX_MACRO(merge->warps,hsg_config.block.warps_min);

  // guaranteed to be an even network
  uint32_t const net_even = merge->levels[0].networks[0];

  // set up left SMEM pointer
  ops = hsg_op(ops,BC_MERGE_H_PREAMBLE(merge->index));

  // trim to number of warps in block -- FIXME -- try make this a
  // multiple of local processor count (Intel = 8, NVIDIA = 4)
  uint32_t const s_max = merge->rows_bc;

  // for all the registers
  for (uint32_t r_lo = 1; r_lo <= hsg_config.thread.regs; r_lo += s_max)
    {
      // compute store count
      uint32_t const r_rem   = hsg_config.thread.regs + 1 - r_lo;
      uint32_t const s_count = MIN_MACRO(s_max,r_rem);

      // block sync -- can skip if first
      if (r_lo > 1)
        ops = hsg_op(ops,BLOCK_SYNC());

      // merge loop
      ops = hsg_bc_half_merge_level(ops,merge,r_lo,s_count);

      // block sync
      ops = hsg_op(ops,BLOCK_SYNC());

      // load rows from shared
      for (uint32_t c=0; c<s_count; c++)
        ops = hsg_op(ops,BC_REG_SHARED_LOAD_V(warps,r_lo+c,c));
    }

  return ops;
}

//
//
//

static
struct hsg_op *
hsg_bs_flip_merge_level(struct hsg_op          *       ops,
                        struct hsg_merge const * const merge,
                        uint32_t                 const level,
                        uint32_t                 const s_pairs)
{
  //
  // Note there are a number of ways to flip merge these warps.  There
  // is a magic number in the merge structure that indicates which
  // warp to activate as well as what network size to invoke.
  //
  // This more complex scheme was used in the past.
  //
  // The newest scheme is far dumber/simpler and simply directs a warp
  // to gather up the network associated with a row and merge them.
  //
  // This scheme may use more registers per thread but not all
  // compilers are high quality.
  //
  // If there are more warps than smem row pairs to merge then we
  // disable the spare warps.
  //
  // If there are more row pairs than warps then each warp works on
  // an equal number of rows.
  //
  // Note that it takes two warps to flip merge two smem rows.
  //
  // FIXME -- We may want to apply the warp smem "mod" value here to
  // attempt to balance the load>merge>store operations across the
  // multiprocessor cores.
  //
  // FIXME -- the old scheme attempted to keep all the warps active
  // but the iteration logic was more complex.  See 2016 checkins.
  //

  // where are we in computed merge?
  uint32_t const count  = merge->levels[level].count;
  uint32_t const index  = 1 << level;

  uint32_t       s_rows = s_pairs * 2;
  uint32_t       base   = 0;

  while (s_rows > 0)
    {
      uint32_t active = merge->warps;

      // disable warps if necessary
      if (merge->warps > s_rows) {
        active = s_rows;
        ops    = hsg_op(ops,BX_MERGE_H_PRED(active));
      }

      // body begin
      ops = hsg_begin(ops);

      // how many equal number of rows to merge?
      uint32_t loops = s_rows / active;

      // decrement
      s_rows -= loops * active;

      for (uint32_t ss=0; ss<loops; ss++)
        {
          // load all registers
          for (uint32_t ii=0; ii<count; ii++)
            {
              // body begin
              ops = hsg_begin(ops);

              uint32_t const offset  = merge->offsets [index+ii];
              uint32_t const network = merge->networks[index+ii];
              uint32_t const lo      = (network + 1) / 2;

              for (uint32_t ll=1; ll<=lo; ll++)
                ops = hsg_op(ops,BS_REG_SHARED_LOAD_LEFT(ll,base+offset+ll-1,ii));

              for (uint32_t rr=lo+1; rr<=network; rr++)
                ops = hsg_op(ops,BS_REG_SHARED_LOAD_RIGHT(rr,base+offset+rr-1,ii));

              // compare left and right
              ops = hsg_thread_merge_left_right_prefix(ops,lo,network-lo,ii);

              // right merging network
              ops = hsg_thread_merge_offset_prefix(ops,lo,network-lo,ii);

              // left merging network
              ops = hsg_thread_merge_prefix(ops,lo,ii);

              for (uint32_t ll=1; ll<=lo; ll++)
                ops = hsg_op(ops,BX_REG_SHARED_STORE_LEFT(ll,base+offset+ll-1,ii));

              for (uint32_t rr=lo+1; rr<=network; rr++)
                ops = hsg_op(ops,BS_REG_SHARED_STORE_RIGHT(rr,base+offset+rr-1,ii));

              // body end
              ops = hsg_end(ops);
            }

          base += active * merge->warps;
        }

      // body end
      ops = hsg_end(ops);
    }

  return ops;
}

static
struct hsg_op *
hsg_bs_flip_merge(struct hsg_op * ops, struct hsg_merge const * const merge)
{
  // set up horizontal smem pointer
  ops = hsg_op(ops,BS_MERGE_H_PREAMBLE(merge->index));

  // begin merge
  uint32_t level = MERGE_LEVELS_MAX_LOG2;

  while (level-- > 0)
    {
      uint32_t const count = merge->levels[level].count;

      if (count == 0)
        continue;

      uint32_t const r_mid       = hsg_config.thread.regs/2 + 1;
      uint32_t const s_pairs_max = merge->rows_bs/2; // this is warp mod

      // for all the registers
      for (uint32_t r_lo=1; r_lo<r_mid; r_lo+=s_pairs_max)
        {
          uint32_t r_hi = hsg_config.thread.regs + 1 - r_lo;

          // compute store count
          uint32_t const s_pairs = MIN_MACRO(s_pairs_max,r_mid - r_lo);

          // store rows to shared
          for (uint32_t c=0; c<s_pairs; c++)
            {
              ops = hsg_op(ops,BS_REG_SHARED_STORE_V(merge->index,r_lo+c,c*2+0));
              ops = hsg_op(ops,BS_REG_SHARED_STORE_V(merge->index,r_hi-c,c*2+1));
            }

          // block sync
          ops = hsg_op(ops,BLOCK_SYNC());

          // merge loop
          ops = hsg_bs_flip_merge_level(ops,merge,level,s_pairs);

          // block sync
          ops = hsg_op(ops,BLOCK_SYNC());

          // load rows from shared
          for (uint32_t c=0; c<s_pairs; c++)
            {
              ops = hsg_op(ops,BS_REG_SHARED_LOAD_V(merge->index,r_lo+c,c*2+0));
              ops = hsg_op(ops,BS_REG_SHARED_LOAD_V(merge->index,r_hi-c,c*2+1));
            }
        }

      // conditionally clean -- no-op if equal to number of warps/block
      if (merge->levels[level].active.b64 != BITS_TO_MASK_64(merge->warps))
        ops = hsg_op(ops,BS_ACTIVE_PRED(merge->index,level));

      // clean warp
      ops = hsg_begin(ops);
      ops = hsg_warp_half(ops,hsg_config.warp.lanes);
      ops = hsg_end(ops);
    }

  return ops;
}

/*

//
// DELETE ME WHEN READY
//

static
struct hsg_op *
hsg_bs_flip_merge_all(struct hsg_op * ops, const struct hsg_merge * const merge)
{
  for (uint32_t merge_idx=0; merge_idx<MERGE_LEVELS_MAX_LOG2; merge_idx++)
    {
      const struct hsg_merge* const m = merge + merge_idx;

      if (m->warps < 2)
        break;

      ops = hsg_op(ops,BS_FRAC_PRED(merge_idx,m->warps));
      ops = hsg_begin(ops);
      ops = hsg_bs_flip_merge(ops,m);
      ops = hsg_end(ops);
    }

  return ops;
}
*/

//
// GENERATE SORT KERNEL
//

static
struct hsg_op *
hsg_bs_sort(struct hsg_op * ops, struct hsg_merge const * const merge)
{
  // func proto
  ops = hsg_op(ops,BS_KERNEL_PROTO(merge->index));

  // begin
  ops = hsg_begin(ops);

  // shared declare
  ops = hsg_op(ops,BS_KERNEL_PREAMBLE(merge->index));

  // load
  ops = hsg_bx_warp_load(ops,0); // 0 = load from vin[]

  // thread sorting network
  ops = hsg_thread_sort(ops);

  // warp merging network
  ops = hsg_warp_merge(ops);

  // slab merging network
  if (merge->warps > 1)
    ops = hsg_bs_flip_merge(ops,merge);

  // store
  ops = hsg_bx_warp_store(ops);

  // end
  ops = hsg_end(ops);

  return ops;
}

//
// GENERATE SORT KERNELS
//

static
struct hsg_op *
hsg_bs_sort_all(struct hsg_op * ops)
{
  for (uint32_t merge_idx=0; merge_idx<MERGE_LEVELS_MAX_LOG2; merge_idx++)
    {
      struct hsg_merge const * const m = hsg_merge + merge_idx;

      if (m->warps == 0)
        break;

      ops = hsg_bs_sort(ops,m);
    }

  return ops;
}

//
// GENERATE CLEAN KERNEL FOR A POWER-OF-TWO
//

static
struct hsg_op *
hsg_bc_clean(struct hsg_op * ops, struct hsg_merge const * const merge)
{
  // func proto
  ops = hsg_op(ops,BC_KERNEL_PROTO(merge->index));

  // begin
  ops = hsg_begin(ops);

  // shared declare
  ops = hsg_op(ops,BC_KERNEL_PREAMBLE(merge->index));

  // if warps == 1 then smem isn't used for merging
  if (merge->warps == 1)
    {
      // load slab directly
      ops = hsg_bx_warp_load(ops,1); // load from vout[]
    }
  else
    {
      // block merging network -- strided load of slabs
      ops = hsg_bc_half_merge(ops,merge);
    }

  // clean warp
  ops = hsg_begin(ops);
  ops = hsg_warp_half(ops,hsg_config.warp.lanes);
  ops = hsg_end(ops);

  // store
  ops = hsg_bx_warp_store(ops);

  // end
  ops = hsg_end(ops);

  return ops;
}

//
// GENERATE CLEAN KERNELS
//

static
struct hsg_op *
hsg_bc_clean_all(struct hsg_op * ops)
{
  for (uint32_t merge_idx=0; merge_idx<MERGE_LEVELS_MAX_LOG2; merge_idx++)
    {
      struct hsg_merge const * const m = hsg_merge + merge_idx;

      if (m->warps == 0)
        break;

      // only generate pow2 clean kernels less than or equal to max
      // warps in block with the assumption that we would've generated
      // a wider sort kernel if we could've so a wider clean kernel
      // isn't a feasible size
      if (!is_pow2_u32(m->warps))
        continue;

      ops = hsg_bc_clean(ops,m);
    }

  return ops;
}

//
// GENERATE FLIP MERGE KERNEL
//

static
struct hsg_op *
hsg_fm_thread_load_left(struct hsg_op * ops, uint32_t const n)
{
  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,FM_REG_GLOBAL_LOAD_LEFT(r,r-1));

  return ops;
}

static
struct hsg_op *
hsg_fm_thread_store_left(struct hsg_op * ops, uint32_t const n)
{
  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,FM_REG_GLOBAL_STORE_LEFT(r,r-1));

  return ops;
}

static
struct hsg_op *
hsg_fm_thread_load_right(struct hsg_op * ops, uint32_t const half_span, uint32_t const half_case)
{
  for (uint32_t r=0; r<half_case; r++)
    ops = hsg_op(ops,FM_REG_GLOBAL_LOAD_RIGHT(r,half_span+1+r));

  return ops;
}

static
struct hsg_op *
hsg_fm_thread_store_right(struct hsg_op * ops, uint32_t const half_span, uint32_t const half_case)
{
  for (uint32_t r=0; r<half_case; r++)
    ops = hsg_op(ops,FM_REG_GLOBAL_STORE_RIGHT(r,half_span+1+r));

  return ops;
}

static
struct hsg_op *
hsg_fm_merge(struct hsg_op * ops,
             uint32_t const scale_log2,
             uint32_t const span_left,
             uint32_t const span_right)
{
  // func proto
  ops = hsg_op(ops,FM_KERNEL_PROTO(scale_log2,msb_idx_u32(pow2_ru_u32(span_right))));

  // begin
  ops = hsg_begin(ops);

  // preamble for loading/storing
  ops = hsg_op(ops,FM_KERNEL_PREAMBLE(span_left));

  // load left span
  ops = hsg_fm_thread_load_left(ops,span_left);

  // load right span
  ops = hsg_fm_thread_load_right(ops,span_left,span_right);

  // compare left and right
  ops = hsg_thread_merge_left_right(ops,span_left,span_right);

  // left merging network
  ops = hsg_thread_merge(ops,span_left);

  // right merging network
  ops = hsg_thread_merge_offset(ops,span_left,span_right);

  // store
  ops = hsg_fm_thread_store_left(ops,span_left);

  // store
  ops = hsg_fm_thread_store_right(ops,span_left,span_right);

  // end
  ops = hsg_end(ops);

  return ops;
}

static
struct hsg_op *
hsg_fm_merge_all(struct hsg_op * ops, uint32_t const scale_log2, uint32_t const warps)
{
  uint32_t const span_left = (warps << scale_log2) / 2;

  for (uint32_t span_right=span_left; span_right >= 1; span_right=pow2_ru_u32(span_right)/2)
    ops = hsg_fm_merge(ops,scale_log2,span_left,span_right);

  return ops;
}

//
// GENERATE HALF MERGE KERNELS
//

static
struct hsg_op *
hsg_hm_thread_load(struct hsg_op * ops, uint32_t const n)
{
  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,HM_REG_GLOBAL_LOAD(r,r-1));

  return ops;
}

static
struct hsg_op *
hsg_hm_thread_store(struct hsg_op * ops, uint32_t const n)
{
  for (uint32_t r=1; r<=n; r++)
    ops = hsg_op(ops,HM_REG_GLOBAL_STORE(r,r-1));

  return ops;
}

static
struct hsg_op *
hsg_hm_merge(struct hsg_op * ops, uint32_t const scale_log2, uint32_t const warps_pow2)
{
  uint32_t const span = warps_pow2 << scale_log2;

  // func proto
  ops = hsg_op(ops,HM_KERNEL_PROTO(scale_log2));

  // begin
  ops = hsg_begin(ops);

  // preamble for loading/storing
  ops = hsg_op(ops,HM_KERNEL_PREAMBLE(span/2));

  // load
  ops = hsg_hm_thread_load(ops,span);

  // thread merging network
  ops = hsg_thread_merge(ops,span);

  // store
  ops = hsg_hm_thread_store(ops,span);

  // end
  ops = hsg_end(ops);

  return ops;
}

//
// GENERATE MERGE KERNELS
//

static
struct hsg_op *
hsg_xm_merge_all(struct hsg_op * ops)
{
  uint32_t const warps = hsg_merge[0].warps;
  uint32_t const warps_pow2 = pow2_rd_u32(warps);

  //
  // GENERATE FLIP MERGE KERNELS
  //
  for (uint32_t scale_log2=hsg_config.merge.flip.lo; scale_log2<=hsg_config.merge.flip.hi; scale_log2++)
    ops = hsg_fm_merge_all(ops,scale_log2,warps);

  //
  // GENERATE HALF MERGE KERNELS
  //
  for (uint32_t scale_log2=hsg_config.merge.half.lo; scale_log2<=hsg_config.merge.half.hi; scale_log2++)
    ops = hsg_hm_merge(ops,scale_log2,warps_pow2);

  return ops;
}

//
//
//

static
struct hsg_op const *
hsg_op_translate_depth(hsg_target_pfn                  target_pfn,
                       struct hsg_target       * const target,
                       struct hsg_config const * const config,
                       struct hsg_merge  const * const merge,
                       struct hsg_op     const *       ops,
                       uint32_t                  const depth)
{
  while (ops->type != HSG_OP_TYPE_EXIT)
    {
      switch (ops->type)
        {
        case HSG_OP_TYPE_END:
          target_pfn(target,config,merge,ops,depth-1);
          return ops + 1;

        case HSG_OP_TYPE_BEGIN:
          target_pfn(target,config,merge,ops,depth);
          ops = hsg_op_translate_depth(target_pfn,target,config,merge,ops+1,depth+1);
          break;

        default:
          target_pfn(target,config,merge,ops++,depth);
        }
    }

  return ops;
}

static
void
hsg_op_translate(hsg_target_pfn                  target_pfn,
                 struct hsg_target       * const target,
                 struct hsg_config const * const config,
                 struct hsg_merge  const * const merge,
                 struct hsg_op     const *       ops)
{
  hsg_op_translate_depth(target_pfn,target,config,merge,ops,0);
}

//
//
//

int
main(int argc, char * argv[])
{
  //
  // PROCESS OPTIONS
  //
  int32_t      opt       = 0;
  bool         verbose   = false;
  bool         autotune  = false;
  char const * arch      = "undefined";

  while ((opt = getopt(argc,argv,"hva:g:G:s:S:w:b:B:m:M:k:r:x:t:f:F:c:C:z")) != EOF)
    {
      switch (opt)
        {
        case 'h':
          fprintf(stderr,"Help goes here...\n");
          return EXIT_FAILURE;

        case 'v':
          verbose = true;
          break;

        case 'a':
          arch = optarg;
          break;

        case 'g':
          hsg_config.block.smem_min = atoi(optarg);
          break;

        case 'G':
          hsg_config.block.smem_quantum = atoi(optarg);
          break;

        case 's':
          hsg_config.block.smem_bs = atoi(optarg);

          // set smem_bc if not already set
          if (hsg_config.block.smem_bc == UINT32_MAX)
            hsg_config.block.smem_bc = hsg_config.block.smem_bs;
          break;

        case 'S':
          hsg_config.block.smem_bc = atoi(optarg);
          break;

        case 'w':
          hsg_config.warp.lanes      = atoi(optarg);
          hsg_config.warp.lanes_log2 = msb_idx_u32(hsg_config.warp.lanes);
          break;

        case 'b':
          // maximum warps in a workgroup / cta / thread block
          {
            uint32_t const warps = atoi(optarg);

            // must always be even
            if ((warps & 1) != 0)
              {
                fprintf(stderr,"Error: -b must be even.\n");
                return EXIT_FAILURE;
              }

            hsg_merge[0].index = 0;
            hsg_merge[0].warps = warps;

            // set warps_max if not already set
            if (hsg_config.block.warps_max == UINT32_MAX)
              hsg_config.block.warps_max = pow2_ru_u32(warps);
          }
          break;

        case 'B':
          // maximum warps that can fit in a multiprocessor
          hsg_config.block.warps_max = atoi(optarg);
          break;

        case 'm':
          // blocks using smem barriers must at least this many warps
          hsg_config.block.warps_min = atoi(optarg);
          break;

        case 'M':
          // the number of warps necessary to load balance horizontal merging
          hsg_config.block.warps_mod = atoi(optarg);
          break;

        case 'r':
          {
            uint32_t const regs = atoi(optarg);

            if ((regs & 1) != 0)
              {
                fprintf(stderr,"Error: -r must be even.\n");
                return EXIT_FAILURE;
              }

            hsg_config.thread.regs = regs;
          }
          break;

        case 'x':
          hsg_config.thread.xtra   = atoi(optarg);
          break;

        case 't':
          hsg_config.type.words    = atoi(optarg);
          break;

        case 'f':
          hsg_config.merge.flip.lo = atoi(optarg);
          break;

        case 'F':
          hsg_config.merge.flip.hi = atoi(optarg);
          break;

        case 'c':
          hsg_config.merge.half.lo = atoi(optarg);
          break;

        case 'C':
          hsg_config.merge.half.hi = atoi(optarg);
          break;

        case 'z':
          autotune = true;
          break;
        }
    }

  //
  // INIT MERGE
  //
  uint32_t const warps_ru_pow2 = pow2_ru_u32(hsg_merge[0].warps);

  for (uint32_t ii=1; ii<=MERGE_LEVELS_MAX_LOG2; ii++)
    {
      hsg_merge[ii].index = ii;
      hsg_merge[ii].warps = warps_ru_pow2 >> ii;
    }

  //
  // WHICH ARCH TARGET?
  //
  hsg_target_pfn hsg_target_pfn;

  if      (strcmp(arch,"debug") == 0)
    hsg_target_pfn = hsg_target_debug;
  else if (strcmp(arch,"cuda") == 0)
    hsg_target_pfn = hsg_target_cuda;
  else if (strcmp(arch,"opencl") == 0)
    hsg_target_pfn = hsg_target_opencl;
  else if (strcmp(arch,"glsl") == 0)
    hsg_target_pfn = hsg_target_glsl;
  else {
    fprintf(stderr,"Invalid arch: %s\n",arch);
    exit(EXIT_FAILURE);
  }

  if (verbose)
    fprintf(stderr,"Target: %s\n",arch);

  //
  // INIT SMEM KEY ALLOCATION
  //
  hsg_config_init_shared();

  //
  // INIT MERGE MAGIC
  //
  for (uint32_t ii=0; ii<MERGE_LEVELS_MAX_LOG2; ii++)
    {
      struct hsg_merge * const merge = hsg_merge + ii;

      if (merge->warps == 0)
        break;

      fprintf(stderr,">>> Generating: %1u %5u %5u %3u %3u ...\n",
              hsg_config.type.words,
              hsg_config.block.smem_bs,
              hsg_config.block.smem_bc,
              hsg_config.thread.regs,
              merge->warps);

      hsg_merge_levels_init_shared(merge);

      hsg_merge_levels_init_1(merge,merge->warps,0,0);

      hsg_merge_levels_hint(merge,autotune);

      //
      // THESE ARE FOR DEBUG/INSPECTION
      //
      if (verbose)
        {
          hsg_merge_levels_debug(merge);
        }
    }

  if (verbose)
    fprintf(stderr,"\n\n");

  //
  // GENERATE THE OPCODES
  //
  uint32_t        const op_count  = 1<<17;
  struct hsg_op * const ops_begin = malloc(sizeof(*ops_begin) * op_count);
  struct hsg_op *       ops       = ops_begin;

  //
  // OPEN INITIAL FILES AND APPEND HEADER
  //
  ops = hsg_op(ops,TARGET_BEGIN());

  //
  // GENERATE TRANSPOSE KERNEL
  //
  ops = hsg_warp_transpose(ops);

  //
  // GENERATE SORT KERNEL
  //
  ops = hsg_bs_sort_all(ops);

  //
  // GENERATE CLEAN KERNELS
  //
  ops = hsg_bc_clean_all(ops);

  //
  // GENERATE MERGE KERNELS
  //
  ops = hsg_xm_merge_all(ops);

  //
  // APPEND FOOTER AND CLOSE INITIAL FILES
  //
  ops = hsg_op(ops,TARGET_END());

  //
  // ... WE'RE DONE!
  //
  ops = hsg_exit(ops);

  //
  // APPLY TARGET TRANSLATOR TO ACCUMULATED OPS
  //
  struct hsg_target target;

  hsg_op_translate(hsg_target_pfn,&target,&hsg_config,hsg_merge,ops_begin);

  //
  // DUMP INSTRUCTION COUNTS
  //
  if (verbose)
    hsg_op_debug();

  return EXIT_SUCCESS;
}

//
//
//

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdio.h>

//
//
//

#include "gen.h"
#include "util.h"
#include "macros.h"
#include "transpose.h"

//
//
//

static
char
hsg_transpose_reg_prefix(uint32_t const cols_log2)
{
  return 'a' + (('r' + cols_log2 - 'a') % 26);
}

static
void
hsg_transpose_blend(uint32_t const cols_log2,
                    uint32_t const row_ll, // lower-left
                    uint32_t const row_ur, // upper-right
                    FILE *         file)
{
  // we're starting register names at '1' for now
  fprintf(file,
          "  HS_TRANSPOSE_BLEND( %c, %c, %2u, %3u, %3u ) \\\n",
          hsg_transpose_reg_prefix(cols_log2-1),
          hsg_transpose_reg_prefix(cols_log2),
          cols_log2,row_ll+1,row_ur+1);
}

static
void
hsg_transpose_remap(uint32_t const row_from,
                    uint32_t const row_to,
                    FILE *         file)
{
  // we're starting register names at '1' for now
  fprintf(file,
          "  HS_TRANSPOSE_REMAP( %c, %3u, %3u )        \\\n",
          hsg_transpose_reg_prefix(msb_idx_u32(hsg_config.warp.lanes)),
          row_from+1,row_to+1);
}

//
//
//

void
hsg_target_igp_genx(struct hsg_file        * const files,
                    struct hsg_merge const * const merge,
                    struct hsg_op    const * const ops,
                    uint32_t                 const depth)
{
  switch (ops->type)
    {
    case HSG_OP_TYPE_END:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "}\n");
      break;

    case HSG_OP_TYPE_BEGIN:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "{\n");
      break;

    case HSG_OP_TYPE_ELSE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "else\n");
      break;

    case HSG_OP_TYPE_FILE_HEADER:
      {
        uint32_t const bc_max          = msb_idx_u32(pow2_rd_u32(merge->warps));
        uint32_t const warp_lanes_log2 = msb_idx_u32(hsg_config.warp.lanes);

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "//                                                            \n"
                "// Copyright 2016 Google Inc.                                 \n"
                "//                                                            \n"
                "// Use of this source code is governed by a BSD-style         \n"
                "// license that can be found in the LICENSE file.             \n"
                "//                                                            \n"
                "                                                              \n"
                "#ifndef HS_CL_ONCE                                            \n"
                "#define HS_CL_ONCE                                            \n"
                "                                                              \n"
                "#define HS_LANES_PER_WARP_LOG2  %u                            \n"
                "#define HS_LANES_PER_WARP       (1 << HS_LANES_PER_WARP_LOG2) \n"
                "#define HS_BS_WARPS             %u                            \n"
                "#define HS_BS_WARPS_LOG2_RU     %u                            \n"
                "#define HS_BC_WARPS_LOG2_MAX    %u                            \n"
                "#define HS_FM_BLOCKS_LOG2_MIN   %u                            \n"
                "#define HS_HM_BLOCKS_LOG2_MIN   %u                            \n"
                "#define HS_KEYS_PER_LANE        %u                            \n"
                "#define HS_REG_LAST(c)          c##%u                         \n"
                "#define HS_KEY_WORDS            %u                            \n"
                "#define HS_KEY_TYPE             %s                            \n"
                "#define HS_EMPTY                                              \n"
                "                                                              \n",
                warp_lanes_log2,
                merge->warps,
                msb_idx_u32(pow2_ru_u32(merge->warps)),
                bc_max,
                hsg_config.merge.flip.lo,
                hsg_config.merge.half.lo,
                hsg_config.thread.regs,
                hsg_config.thread.regs,
                hsg_config.type.words,
                (hsg_config.type.words == 2) ? "ulong" : "uint");

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "#define HS_SLAB_ROWS()    \\\n");

        for (uint32_t ii=1; ii<=hsg_config.thread.regs; ii++)
          fprintf(files[HSG_FILE_TYPE_HEADER].file,
                  "  HS_SLAB_ROW( %3u, %3u ) \\\n",ii,ii-1);

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "  HS_EMPTY\n"
                "          \n");

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "#define HS_TRANSPOSE_SLAB()                \\\n");

        for (uint32_t ii=1; ii<=warp_lanes_log2; ii++)
          fprintf(files[HSG_FILE_TYPE_HEADER].file,
                  "  HS_TRANSPOSE_STAGE( %u )                  \\\n",ii);

        hsg_transpose(msb_idx_u32(hsg_config.warp.lanes),
                      hsg_config.thread.regs,
                      files[HSG_FILE_TYPE_HEADER].file,
                      files[HSG_FILE_TYPE_HEADER].file,
                      hsg_transpose_blend,
                      hsg_transpose_remap);

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "  HS_EMPTY\n"
                "          \n");

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "//                                                      \n"
                "// Copyright 2016 Google Inc.                           \n"
                "//                                                      \n"
                "// Use of this source code is governed by a BSD-style   \n"
                "// license that can be found in the LICENSE file.       \n"
                "//                                                      \n"
                "                                                        \n"
                "#include <%s_macros.h>                                  \n"
                "                                                        \n"
                "//                                                      \n"
                "//                                                      \n"
                "//                                                      \n",
                files[HSG_FILE_TYPE_SOURCE].prefix);
      }
      break;

    case HSG_OP_TYPE_FILE_FOOTER:
      fprintf(files[HSG_FILE_TYPE_HEADER].file,
              "                                \n"
              "#endif                          \n"
              "                                \n"
              "//                              \n"
              "//                              \n"
              "//                              \n"
              "                                \n");
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "                                \n"
              "//                              \n"
              "//                              \n"
              "//                              \n"
              "                                \n");
      break;

    case HSG_OP_TYPE_TRANSPOSE_KERNEL_PROTO:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                " \n"
                "__kernel                                                             \n"
                "__attribute__((intel_reqd_sub_group_size(%u)))                       \n"
                "void hs_kernel_transpose(__global HS_KEY_TYPE * const restrict vout) \n",
                hsg_config.warp.lanes);
      }
      break;

    case HSG_OP_TYPE_TRANSPOSE_KERNEL_PREAMBLE:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "uint const global_id = get_global_id(0);                         \n"
                "uint const gmem_idx  = (global_id / %u) * %u + (global_id & %u); \n"
                "                                                                 \n",
                hsg_config.warp.lanes,
                hsg_config.warp.lanes * hsg_config.thread.regs,
                hsg_config.warp.lanes-1);
      }
      break;

    case HSG_OP_TYPE_TRANSPOSE_KERNEL_BODY:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "HS_TRANSPOSE_SLAB()\n");
      }
      break;

    case HSG_OP_TYPE_BS_KERNEL_PROTO:
      {
        struct hsg_merge const * const m = merge + ops->a;

        uint32_t const tpb = m->warps * hsg_config.warp.lanes;
        uint32_t const bs  = pow2_ru_u32(m->warps);
        uint32_t const msb = msb_idx_u32(bs);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                " \n"
                "__kernel                                                               \n"
                "__attribute__((reqd_work_group_size(%u,1,1)))                          \n"
                "__attribute__((intel_reqd_sub_group_size(%u)))                         \n"
                "void hs_kernel_bs_%u(__global HS_KEY_TYPE const * const restrict vin,  \n"
                "                     __global HS_KEY_TYPE       * const restrict vout) \n",
                tpb,
                hsg_config.warp.lanes,
                msb);
      }
      break;

    case HSG_OP_TYPE_BS_KERNEL_PREAMBLE:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "__local union { \n");

        struct hsg_merge const * const m = merge + ops->a;

        if (m->warps > 1)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "  HS_KEY_TYPE m[%u * %u];\n",
                    m->rows_bs,
                    m->warps * hsg_config.warp.lanes);
          }

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "} shared;          \n"
                "                   \n");

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "uint const global_id = get_global_id(0);                         \n"
                "uint const gmem_idx  = (global_id / %u) * %u + (global_id & %u); \n"
                "                                                                 \n",
                hsg_config.warp.lanes,
                hsg_config.warp.lanes * hsg_config.thread.regs,
                hsg_config.warp.lanes-1);
      }
      break;

    case HSG_OP_TYPE_BC_KERNEL_PROTO:
      {
        uint32_t const bc_max = pow2_rd_u32(merge[0].warps);
        uint32_t const tpb    = bc_max * hsg_config.warp.lanes;
        uint32_t const msb    = msb_idx_u32(merge[ops->a].warps);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "                                                                 \n"
                "__kernel                                                         \n"
                "__attribute__((intel_reqd_sub_group_size(%u)))                   \n"
                "void hs_kernel_bc_%u(__global HS_KEY_TYPE * const restrict vout) \n",
                hsg_config.warp.lanes,msb);
      }
      break;

    case HSG_OP_TYPE_BC_KERNEL_PREAMBLE:
      {
        struct hsg_merge const * const m      = merge + ops->a;
        uint32_t                 const bc_max = pow2_rd_u32(merge[0].warps);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "__local union {    \n");

        if (m->warps > 1)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "  HS_KEY_TYPE m[%-3u * %u];\n",
                    m->rows_bc,
                    m->warps * hsg_config.warp.lanes);
          }

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "} shared;          \n"
                "                   \n");

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "uint const global_id = get_global_id(0);                         \n"
                "uint const gmem_idx  = (global_id / %u) * %u + (global_id & %u); \n"
                "                                                                 \n",
                hsg_config.warp.lanes,
                hsg_config.warp.lanes * hsg_config.thread.regs,
                hsg_config.warp.lanes-1);
      }
      break;

    case HSG_OP_TYPE_FM_KERNEL_PROTO:
      fprintf(files[HSG_FILE_TYPE_HEADER].file,
              "#define HS_FM_BLOCKS_LOG2_%-2u   %u \n",
              ops->a,ops->b);

      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              " \n"
              "__kernel                                                         \n"
              "__attribute__((intel_reqd_sub_group_size(%u)))                   \n"
              "void hs_kernel_fm_%u(__global HS_KEY_TYPE * const restrict vout, \n"
              "                     uint const fm_full,                         \n"
              "                     uint const fm_frac)                         \n",
              hsg_config.warp.lanes,ops->a);
      break;

    case HSG_OP_TYPE_FM_KERNEL_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "uint const    global_id     = (uint)get_global_id(0);                                   \n"
              "uint const    warp_idx      = global_id / %u;                                           \n"
              "uint const    warp_lane_idx = global_id & %u;                                           \n"
              "                                                                                        \n"
              "uint const    merge_idx     = warp_idx / %u >> %u;                                      \n"
              "                                                                                        \n"
              "uint const    merge_stride  = %u * %u << %u;                                            \n"
              "uint const    merge_keys    = merge_stride * %u;                                        \n"
              "                                                                                        \n"
              "uint const    merge_base    = merge_idx * merge_keys;                                   \n"
              "                                                                                        \n"
              "uint const    merge_l_off   = (warp_idx - merge_idx * (%u << %u)) * %u + warp_lane_idx; \n"
              "uint const    merge_l_end   = merge_stride * (%u / 2 - 1) + merge_l_off;                \n"
              "                                                                                        \n"
              "int  const    merge_r_off   = merge_keys - merge_l_end - 1;                             \n"
              "                                                                                        \n"
              "__global HS_KEY_TYPE * const restrict merge_l = vout + (merge_base + merge_l_off);      \n"
              "__global HS_KEY_TYPE * const restrict merge_r = vout + (merge_base + merge_r_off);      \n"
              "                                                                                        \n",
              hsg_config.warp.lanes,
              hsg_config.warp.lanes-1,
              hsg_config.thread.regs,ops->b,
              hsg_config.thread.regs,hsg_config.warp.lanes,ops->b,
              ops->a,
              hsg_config.thread.regs,ops->b,hsg_config.warp.lanes,
              ops->a);
      break;

    case HSG_OP_TYPE_HM_KERNEL_PROTO:
      {
        uint32_t const bc_max = msb_idx_u32(pow2_rd_u32(merge[0].warps));

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "#define HS_HM_BLOCKS_LOG2_%-2u   %u \n",
                ops->a,ops->b);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                " \n"
                "__kernel                                                         \n"
                "__attribute__((intel_reqd_sub_group_size(%u)))                   \n"
                "void hs_kernel_hm_%u(__global HS_KEY_TYPE * const restrict vout) \n",
                hsg_config.warp.lanes,ops->a);
      }
      break;

    case HSG_OP_TYPE_HM_KERNEL_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "uint const    global_id     = (uint)get_global_id(0);                          \n"
              "uint const    warp_idx      = global_id / %u;                                  \n"
              "uint const    warp_lane_idx = global_id & %u;                                  \n"
              "                                                                               \n"
              "uint const    merge_idx     = (warp_idx / %u) >> %u;                           \n"
              "                                                                               \n"
              "uint const    merge_stride  = %u * %u << %u;                                   \n"
              "uint const    merge_keys    = merge_stride * %u;                               \n"
              "                                                                               \n"
              "uint const    merge_base    = merge_idx * merge_keys;                          \n"
              "uint const    merge_off     = (warp_idx - merge_idx * (%u << %u)) * %u;        \n"
              "                                                                               \n"
              "__global HS_KEY_TYPE * const restrict merge_ptr = vout + (merge_base + merge_off + warp_lane_idx); \n"
              "                                                                               \n",
              hsg_config.warp.lanes,
              hsg_config.warp.lanes-1,
              hsg_config.thread.regs,ops->b,
              hsg_config.thread.regs,hsg_config.warp.lanes,ops->b,
              ops->a,
              hsg_config.thread.regs,ops->b,hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BX_REG_GLOBAL_LOAD:
      {
        static char const * const vstr[] = { "vin", "vout" };

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "HS_KEY_TYPE r%-3u = (%s + gmem_idx)[%-3u * %u]; \n",
                ops->n,vstr[ops->v],ops->n-1,hsg_config.warp.lanes);
      }
      break;

    case HSG_OP_TYPE_BX_REG_GLOBAL_STORE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "(vout + gmem_idx)[%-3u * %u] = r%u; \n",
              ops->n-1,hsg_config.warp.lanes,ops->n);
      break;

    case HSG_OP_TYPE_HM_REG_GLOBAL_LOAD:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%-3u = merge_ptr[%-3u * merge_stride];\n",
              ops->a,ops->b);
      break;

    case HSG_OP_TYPE_HM_REG_GLOBAL_STORE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_ptr[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%-3u = merge_l[%-3u * merge_stride];\n",
              ops->a,ops->b);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_STORE_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_l[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%-3u = merge_r[%-3u * merge_stride];\n",
              ops->a,ops->b);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_STORE_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_r[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_WARP_FLIP:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "uint const flip_lane_mask = %u;                                        \n"
              "uint const flip_lane_idx  = get_sub_group_local_id() ^ flip_lane_mask; \n"
              "int  const t_lt           = get_sub_group_local_id() < flip_lane_idx;  \n",
              ops->n-1);
      break;

    case HSG_OP_TYPE_WARP_HALF:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "uint const half_lane_mask = %u;                                        \n"
              "uint const half_lane_idx  = get_sub_group_local_id() ^ half_lane_mask; \n"
              "int  const t_lt           = get_sub_group_local_id() < half_lane_idx;  \n",
              ops->n / 2);
      break;

    case HSG_OP_TYPE_CMP_FLIP:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_CMP_FLIP(%-3u,r%-3u,r%-3u)\n",ops->a,ops->b,ops->c);
      break;

    case HSG_OP_TYPE_CMP_HALF:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_CMP_HALF(%-3u,r%-3u)\n",ops->a,ops->b);
      break;

    case HSG_OP_TYPE_CMP_XCHG:
      if (ops->c == UINT32_MAX)
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "HS_CMP_XCHG(r%-3u,r%-3u)\n",
                  ops->a,ops->b);
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "HS_CMP_XCHG(r%u_%u,r%u_%u)\n",
                  ops->c,ops->a,ops->c,ops->b);
        }
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_STORE_V:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "(shared.m + get_local_id(0))[%-3u * %-2u * %-3u] = r%u;\n",
              merge[ops->a].warps,hsg_config.warp.lanes,ops->c,ops->b);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_V:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "r%-3u = (shared.m + get_local_id(0))[%-3u * %-2u * %-3u];\n",
              ops->b,merge[ops->a].warps,hsg_config.warp.lanes,ops->c);
      break;

    case HSG_OP_TYPE_BC_REG_SHARED_LOAD_V:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%-3u = (shared.m + get_local_id(0))[%-3u * %-2u * %-3u];\n",
              ops->b,ops->a,hsg_config.warp.lanes,ops->c);
      break;

    case HSG_OP_TYPE_BX_REG_SHARED_STORE_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "(shared.m + smem_l_idx)[%5u] = r%u_%u;\n",
              ops->b * hsg_config.warp.lanes,
              ops->c,
              ops->a);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_STORE_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "(shared.m + smem_r_idx)[%5u] = r%u_%u;\n",
              ops->b * hsg_config.warp.lanes,
              ops->c,
              ops->a);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%u_%-3u = (shared.m + smem_l_idx)[%u];\n",
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%u_%-3u = (shared.m + smem_r_idx)[%u];\n",
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BC_REG_GLOBAL_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "HS_KEY_TYPE r%u_%-3u = (vout + gmem_l_idx)[%u];\n",
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BLOCK_SYNC:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "barrier(CLK_LOCAL_MEM_FENCE);\n"); // OpenCL 2.0+: work_group_barrier
      break;

    case HSG_OP_TYPE_BS_FRAC_PRED:
      {
        if (ops->m == 0)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (warp_idx < bs_full)\n");
          }
        else
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "else if (bs_frac == %u)\n",
                    ops->w);
          }
      }
      break;

    case HSG_OP_TYPE_BS_MERGE_H_PREAMBLE:
      {
        struct hsg_merge const * const m = merge + ops->a;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "uint const smem_l_idx = get_sub_group_id() * %u + get_sub_group_local_id(); \n"
                "uint const smem_r_idx = (get_sub_group_id() ^ 1) * %u + (get_sub_group_local_id() ^ %u); \n",
                m->warps * hsg_config.warp.lanes,
                m->warps * hsg_config.warp.lanes, hsg_config.warp.lanes-1);
#if 0
        if (ops->b == true)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "uint smem_l_idx =  get_sub_group_id()      * %u +  get_sub_group_local_id();       \n"
                    "uint smem_r_idx = (get_sub_group_id() ^ 1) * %u + (get_sub_group_local_id() ^ %u); \n",
                    m->warps * hsg_config.warp.lanes,
                    m->warps * hsg_config.warp.lanes, hsg_config.warp.lanes-1);
          }
        else // update
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "smem_l_idx =  get_sub_group_id()      * %u +  get_sub_group_local_id();       \n"
                    "smem_r_idx = (get_sub_group_id() ^ 1) * %u + (get_sub_group_local_id() ^ %u); \n",
                    m->warps * hsg_config.warp.lanes,
                    m->warps * hsg_config.warp.lanes, hsg_config.warp.lanes-1);
          }
#endif
      }
      break;

    case HSG_OP_TYPE_BC_MERGE_H_PREAMBLE:
      {
        struct hsg_merge const * const m = merge + ops->a;
        uint32_t                 const b = m->warps * hsg_config.warp.lanes;
        uint32_t                 const k = b * hsg_config.thread.regs;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "uint const gmem_l_idx = (global_id / %u) * %u + (global_id & %u);            \n"
                "uint const smem_l_idx = get_sub_group_id() * %u  + get_sub_group_local_id(); \n",
                b,k,b-1,
                b);

      }
      break;

    case HSG_OP_TYPE_BX_MERGE_H_PRED:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "if (get_sub_group_id() < %u)\n",
              ops->a);
      break;

    case HSG_OP_TYPE_BS_ACTIVE_PRED:
      {
        struct hsg_merge const * const m = merge + ops->a;

        if (m->warps <= 32)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (((1u << get_sub_group_id()) & 0x%08X) != 0)\n",
                    m->levels[ops->b].active.b32a2[0]);
          }
        else
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (((1UL << get_sub_group_id()) & 0x%08X%08XL) != 0L)\n",
                    m->levels[ops->b].active.b32a2[1],
                    m->levels[ops->b].active.b32a2[0]);
          }
      }
      break;

    case HSG_OP_TYPE_FM_MERGE_RIGHT_PRED:
      {
        if (ops->a == ops->b)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (merge_idx < fm_full) \n");
          }
        else if (ops->b > 1)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "else if (fm_frac == %u) \n",
                    ops->b);
          }
        else
          {
	    fprintf(files[HSG_FILE_TYPE_SOURCE].file,
		    "else\n");
          }
      }
      break;

    default:
      hsg_target_debug(files,merge,ops,depth);
      break;
    }
}

//
//
//

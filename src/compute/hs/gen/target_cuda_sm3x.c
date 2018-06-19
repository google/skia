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

//
//
//

void
hsg_target_cuda_sm3x(struct hsg_file        * const files,
                     struct hsg_merge const * const merge,
                     struct hsg_op    const * const ops,
                     uint32_t                 const depth)
{
  const char* const type     = (hsg_config.type.words == 2) ? "uint64_t"   : "uint32_t";
  const char* const type_max = (hsg_config.type.words == 2) ? "UINT64_MAX" : "UINT32_MAX";

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
        uint32_t const bc_min = msb_idx_u32(hsg_config.block.warps_min);
        uint32_t const bc_max = msb_idx_u32(pow2_rd_u32(merge[0].warps));

        fprintf(files[HSG_FILE_TYPE_HEADER].file,
                "//                                                      \n"                
                "// Copyright 2016 Google Inc.                           \n"
                "//                                                      \n"
                "// Use of this source code is governed by a BSD-style   \n"
                "// license that can be found in the LICENSE file.       \n"
                "//                                                      \n"
                "                                                        \n"
                "#pragma once                                            \n"
                "                                                        \n"
                "#include <stdint.h>                                     \n"
                "                                                        \n"
                "#define HS_LANES_PER_WARP     %u                        \n"
                "#define HS_BS_WARPS_PER_BLOCK %u                        \n"
                "#define HS_BC_WARPS_LOG2_MIN  %u                        \n"
                "#define HS_BC_WARPS_LOG2_MAX  %u                        \n"
                "#define HS_KEYS_PER_THREAD    %u                        \n"
                "#define HS_KEY_WORDS          %u                        \n"
                "#define HS_KEY_TYPE           %s                        \n"
                "                                                        \n"
                "#include <%s_args.h>                                    \n"
                "                                                        \n",
                hsg_config.warp.lanes,
                merge->warps,
                bc_min,
                bc_max,
                hsg_config.thread.regs,
                hsg_config.type.words,
                type,
                files[HSG_FILE_TYPE_SOURCE].prefix);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "// -*- compile-command: \"nvcc -arch sm_52 -Xptxas=-v,-abi=no -cubin -I. %s\"; -*-\n",
                files[HSG_FILE_TYPE_SOURCE].name);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "//                                                      \n"                
                "// Copyright 2016 Google Inc.                           \n"
                "//                                                      \n"
                "// Use of this source code is governed by a BSD-style   \n"
                "// license that can be found in the LICENSE file.       \n"
                "//                                                      \n"
                "                                                        \n"
                "#ifdef __cplusplus                                      \n"
                "extern \"C\" {                                          \n"
                "#endif                                                  \n"
                "                                                        \n"
                "#include \"%s_launcher.h\"                              \n"
                "                                                        \n"
                "#ifdef __cplusplus                                      \n"
                "}                                                       \n"
                "#endif                                                  \n"
                "                                                        \n"
                "#include \"%s_launch_bounds.h\"                         \n"
                "#include <%s_finalize.inl>                              \n"
                "                                                        \n"
                "//                                                      \n"
                "//                                                      \n"
                "//                                                      \n",
                files[HSG_FILE_TYPE_HEADER].prefix,
                files[HSG_FILE_TYPE_SOURCE].prefix,
                files[HSG_FILE_TYPE_SOURCE].prefix);
      }
      break;

    case HSG_OP_TYPE_FILE_FOOTER:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "                              \n"
              "//                            \n"
              "//                            \n"
              "//                            \n"
              "                              \n"
              "#include \"%s_launcher.inl\"  \n"
              "                              \n"
              "//                            \n"
              "//                            \n"
              "//                            \n",
              files[HSG_FILE_TYPE_SOURCE].prefix);
      break;

    case HSG_OP_TYPE_BS_KERNEL_PROTO:
      {
        const uint32_t tpb = merge->warps * hsg_config.warp.lanes;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "                                                \n"
                "extern \"C\"                                    \n"
                "__global__                                      \n"
                "__launch_bounds__(%u,%u)                        \n"
                "void                                            \n"
                "hs_bs_kernel(const struct hs_args args) \n",
                tpb,1);
      }
      break;

    case HSG_OP_TYPE_BS_KERNEL_PREAMBLE:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "__shared__ union { \n");

        for (uint32_t ii=0; ii<MERGE_LEVELS_MAX_LOG2; ii++)
          {
            const struct hsg_merge* const m = merge + ii;

            if (m->warps < 2)
              break;

            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "  %s m%u[%u][%u];\n",
                    type,
                    ii,
                    m->rows_bs,
                    m->warps * hsg_config.warp.lanes);
          }

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                " struct {             \n"
                "    %s f[%u][%u];     \n"
                "    %s l[%u];         \n"
                " };                   \n",
                type,
                merge[0].warps,
                hsg_config.warp.skpw_bs - 1,
                type,
                merge[0].warps);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "} shared;          \n"
                "                   \n");

        const uint32_t kpw = hsg_config.warp.lanes * hsg_config.thread.regs;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "const int32_t block_warp_idx = threadIdx.x / %u;                 \n"
                "const int32_t warp_lane_idx  = threadIdx.x & %u;                 \n"
                "const int32_t warp_idx       = blockIdx.x * %u + block_warp_idx; \n"
                "const int32_t warp_gmem_idx  = warp_idx * %u + warp_lane_idx;    \n"
                "                                                                 \n"
                "%s const * const vin_ptr  = args.vin  + warp_gmem_idx;           \n"
                "%s       * const vout_ptr = args.vout + warp_gmem_idx;           \n"
                "                                                                 \n",

                hsg_config.warp.lanes,
                hsg_config.warp.lanes - 1,
                merge[0].warps,
                kpw,
                type,
                type);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "if (warp_idx >= args.bs.full + args.bs.frac) \n"
                "  return;                                    \n"
                "                                             \n");
      }
      break;

    case HSG_OP_TYPE_BC_KERNEL_PROTO:
      {
        uint32_t const bc_warps = merge[ops->a].warps;
        uint32_t const tpb      = bc_warps * hsg_config.warp.lanes;
        uint32_t const bpm      = hsg_config.block.warps_max / bc_warps;
        uint32_t const msb      = msb_idx_u32(bc_warps);

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "                                                   \n"
                "extern \"C\"                                       \n"
                "__global__                                         \n"
                "__launch_bounds__(%u,%u)                           \n"
                "void                                               \n"
                "hs_bc_%u_kernel(const struct hs_args args) \n",
                tpb,bpm,
                msb);
      }
      break;

    case HSG_OP_TYPE_BC_KERNEL_PREAMBLE:
      {
        const struct hsg_merge* const m = merge + ops->a;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "__shared__ union {    \n");

        if (m->warps >= 2)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "  %s m%u[%u][%u];  \n",
                    type,
                    ops->a,
                    m->rows_bc,
                    m->warps * hsg_config.warp.lanes);
          }

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                " struct {             \n"
                "    %s f[%u][%u];     \n"
                "    %s l[%u];         \n"
                " };                   \n"
                "} shared;             \n"
                "                      \n",
                type,m->warps,m->skpw_bc - 1,
                type,m->warps);

        const uint32_t kpw = hsg_config.warp.lanes * hsg_config.thread.regs;

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "const int32_t block_warp_idx = threadIdx.x / %u;                     \n"
                "const int32_t warp_lane_idx  = threadIdx.x & %u;                     \n"
                "const int32_t warp_gmem_base = blockIdx.x * %u * %u + warp_lane_idx; \n"
                "const int32_t warp_gmem_idx  = warp_gmem_base + block_warp_idx * %u; \n"
                "                                                                     \n"
                "%s * const vout_ptr = args.vout + warp_gmem_idx;                     \n"
                "                                                                     \n",
                hsg_config.warp.lanes,
                hsg_config.warp.lanes - 1,
                m->warps,kpw,
                kpw,
                type);

#if 0
        //
        // NO LONGER NEED THIS TEST
        //
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "if (warp_idx >= args.bc.full) \n"
                "  return;                     \n"
                "                              \n");
#endif
      }
      break;

    case HSG_OP_TYPE_FM_KERNEL_PROTO:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "                                                   \n"
              "#define HS_FM_WARPS_LOG2_%u  %u                \n"
              "extern \"C\"                                       \n"
              "__global__                                         \n"
              "HS_FM_LAUNCH_BOUNDS_%u                         \n"
              "void                                               \n"
              "hs_fm_%u_kernel(const struct hs_args args) \n",
              ops->a,
              ops->b,
              ops->a - ops->b,
              ops->a);
      break;

    case HSG_OP_TYPE_FM_KERNEL_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "const int32_t warp_idx      = (blockDim.x * blockIdx.x + threadIdx.x) / %u;              \n"
              "const int32_t warp_lane_idx = threadIdx.x & %u;                                          \n"
              "                                                                                         \n"
              "const int32_t merge_idx     = warp_idx / %u >> %u;                                       \n"
              "                                                                                         \n"
              "const int32_t merge_stride  = %u * %u << %u;                                             \n"
              "const int32_t merge_keys    = merge_stride * %u;                                         \n"
              "                                                                                         \n"
              "const int32_t merge_base    = merge_idx * merge_keys;                                    \n"
              "                                                                                         \n"
              "const int32_t merge_l_off   = (warp_idx - merge_idx * (%u << %u)) * %u + warp_lane_idx;  \n"
              "const int32_t merge_l_end   = merge_l_off + merge_stride * (%u / 2 - 1);                 \n"
              "%s * const merge_l   = args.vout + merge_base + merge_l_off;                             \n"
              "                                                                                         \n"
              "const int32_t merge_r_off   = merge_keys - merge_l_end - 1;                              \n"
              "%s * const merge_r   = args.vout + merge_base + merge_r_off;                             \n"
              "                                                                                         \n",
              hsg_config.warp.lanes,
              hsg_config.warp.lanes-1,
              hsg_config.thread.regs,ops->b,
              hsg_config.thread.regs,hsg_config.warp.lanes,ops->b,
              ops->a,
              hsg_config.thread.regs,ops->b,hsg_config.warp.lanes,
              ops->a,
              type,
              type);
      break;

    case HSG_OP_TYPE_HM_KERNEL_PROTO:
      {
        const uint32_t bc_max = msb_idx_u32(pow2_rd_u32(merge[0].warps));

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "                                                   \n"
                "#define HS_HM_WARPS_LOG2_%u  %u                \n"
                "extern \"C\"                                       \n"
                "__global__                                         \n"
                "HS_HM_LAUNCH_BOUNDS_%u                         \n"
                "void                                               \n"
                "hs_hm_%u_kernel(const struct hs_args args) \n",
                ops->a,
                ops->b,
                ops->a - ops->b - bc_max,
                ops->a);
      }
      break;

    case HSG_OP_TYPE_HM_KERNEL_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "const int32_t warp_idx      = (blockDim.x * blockIdx.x + threadIdx.x) / %u;       \n"
              "const int32_t warp_lane_idx = threadIdx.x & %u;                                   \n"
              "                                                                                  \n"
              "const int32_t merge_idx     = (warp_idx / %u) >> %u;                              \n"
              "                                                                                  \n"
              "const int32_t merge_stride  = %u * %u << %u;                                      \n"
              "const int32_t merge_keys    = merge_stride * %u;                                  \n"
              "                                                                                  \n"
              "const int32_t merge_base    = merge_idx * merge_keys;                             \n"
              "                                                                                  \n"
              "const int32_t merge_off     = (warp_idx - merge_idx * (%u << %u)) * %u;           \n"
              "%s * const merge_ptr     = args.vout + merge_base + merge_off + warp_lane_idx;    \n"
              "                                                                                  \n",
              hsg_config.warp.lanes,
              hsg_config.warp.lanes-1,
              hsg_config.thread.regs,ops->b,
              hsg_config.thread.regs,hsg_config.warp.lanes,ops->b,
              ops->a,
              hsg_config.thread.regs,ops->b,hsg_config.warp.lanes,
              type);
      break;

    case HSG_OP_TYPE_BX_REG_GLOBAL_LOAD:
      {
        static const char* const vstr[] = { "vin_ptr", "vout_ptr" };

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "%s r%-3u = %s[%-3u * %u]; \n",
                type,ops->n,vstr[ops->v],ops->n-1,hsg_config.warp.lanes);
      }
      break;

    case HSG_OP_TYPE_BX_REG_GLOBAL_STORE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "vout_ptr[%-3u * %u] = r%u; \n",
              ops->n-1,hsg_config.warp.lanes,ops->n);
      break;

#if 0
    case HSG_OP_TYPE_BX_WARP_STORE_PRED:
      if (ops->a == 1)
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "if (!args.is_final) \n");
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "if (blockIdx.x * %u + block_warp_idx >= args.bx.ru) \n"
                  "{                                                   \n"
                  "  return;                                           \n"
                  "}                                                   \n"
                  "else if (!args.is_final)                            \n",
                  ops->a);
        }
      break;
#endif

    case HSG_OP_TYPE_HM_REG_GLOBAL_LOAD:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%-3u = merge_ptr[%-3u * merge_stride];\n",
              type,ops->a,ops->b);
      break;

    case HSG_OP_TYPE_HM_REG_GLOBAL_STORE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_ptr[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%-3u = merge_l[%-3u * merge_stride];\n",
              type,ops->a,ops->b);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_STORE_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_l[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_LOAD_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%-3u = merge_r[%-3u * merge_stride];\n",
              type,ops->a,ops->b);
      break;

    case HSG_OP_TYPE_FM_REG_GLOBAL_STORE_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "merge_r[%-3u * merge_stride] = r%u;\n",
              ops->b,ops->a);
      break;

    case HSG_OP_TYPE_WARP_FLIP:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "const int32_t flip_lane_mask = %u;                             \n"
                "const int32_t flip_lane_idx  = warp_lane_idx ^ flip_lane_mask; \n"
                "const bool    t_lt           = warp_lane_idx < flip_lane_idx;  \n",
                ops->n-1);
      }
      break;

    case HSG_OP_TYPE_WARP_HALF:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "const int32_t half_lane_mask = %u;                             \n"
                "const int32_t half_lane_idx  = warp_lane_idx ^ half_lane_mask; \n"
                "const bool    t_lt           = warp_lane_idx < half_lane_idx;  \n",
                ops->n / 2);
      }
      break;

    case HSG_OP_TYPE_CMP_FLIP:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,      
              "HS_CMP_FLIP(r%-3u,r%-3u,r%-3u)\n",ops->a,ops->b,ops->c);
      break;

    case HSG_OP_TYPE_CMP_HALF:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,      
              "HS_CMP_HALF(r%-3u,r%-3u)\n",ops->a,ops->b);
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
              "smem_v[%-3u * %-2u * %-3u] = r%u;\n",
              ops->a,hsg_config.warp.lanes,ops->c,ops->b);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_V:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "r%-3u = smem_v[%-3u * %-2u * %-3u];\n",
              ops->b,ops->a,hsg_config.warp.lanes,ops->c);
      break;

    case HSG_OP_TYPE_BC_REG_SHARED_LOAD_V:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%-3u = smem_v[%-3u * %-2u * %-3u];\n",
              type,ops->b,ops->a,hsg_config.warp.lanes,ops->c);
      break;

    case HSG_OP_TYPE_BX_REG_SHARED_STORE_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "smem_l[%5u] = r%u_%u;\n",
              ops->b * hsg_config.warp.lanes,
              ops->c,
              ops->a);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_STORE_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "smem_r[%5u] = r%u_%u;\n",
              ops->b * hsg_config.warp.lanes,
              ops->c,
              ops->a);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%u_%-3u = smem_l[%u];\n",
              type,
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BS_REG_SHARED_LOAD_RIGHT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%u_%-3u = smem_r[%u];\n",
              type,
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BC_REG_GLOBAL_LOAD_LEFT:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s r%u_%-3u = gmem_l[%u];\n",
              type,
              ops->c,
              ops->a,
              ops->b * hsg_config.warp.lanes);
      break;

#if 0
    case HSG_OP_TYPE_REG_F_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s*       const f_%u_smem_st_ptr = &shared.f[block_warp_idx]",
              type,
              ops->a);

      if (ops->a >= (int32_t)hsg_config.warp.lanes)
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "[warp_lane_idx * %u];\n",
                  (ops->a / hsg_config.warp.lanes) * hsg_config.warp.lanes + 1);
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "[(warp_lane_idx & 0x%X) * %u + (warp_lane_idx & ~0x%X)];\n",
                  ops->a-1,
                  hsg_config.warp.lanes + 1,
                  ops->a-1);
        }

      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "const %s* const f_%u_smem_ld_ptr = &shared.f[block_warp_idx][warp_lane_idx];\n",
              type,
              ops->a);

      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s*       const f_%u_gmem_st_ptr = args.vout + warp_gmem_idx",
              type,
              ops->a);

      if (ops->a >= (int32_t)hsg_config.warp.lanes)
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,";\n");
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  " - warp_lane_idx + (warp_lane_idx & ~0x%X) * %u + (warp_lane_idx & 0x%X);\n",
                  ops->a-1,
                  hsg_config.thread.regs,
                  ops->a-1);
        }
      break;

    case HSG_OP_TYPE_REG_SHARED_STORE_F:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "f_%u_smem_st_ptr[%-3u] = r%u;\n",
              ops->c,
              ops->b,
              ops->a);
      break;

    case HSG_OP_TYPE_REG_SHARED_LOAD_F:
      if (ops->c >= (int32_t)hsg_config.warp.lanes)
        {
          uint32_t const adjacent = ops->c / hsg_config.warp.lanes;
          uint32_t const stride   = adjacent * hsg_config.warp.lanes + 1;

          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "r%-3u = f_%u_smem_ld_ptr[%-3u];\n",
                  ops->a,
                  ops->c,
                  (ops->b / adjacent) * stride + (ops->b % adjacent) * hsg_config.warp.lanes);
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "r%-3u = f_%u_smem_ld_ptr[%-3u];\n",
                  ops->a,
                  ops->c,
                  ops->b * (hsg_config.warp.lanes + 1));
        }
      break;

    case HSG_OP_TYPE_REG_GLOBAL_STORE_F:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "f_%u_gmem_st_ptr[%-3u * %u + %-3u] = r%u;\n",
              ops->c,
              ops->b,
              hsg_config.thread.regs, // hsg_config.warp.lanes,
              (ops->a - 1) & ~(ops->c - 1),
              ops->a);
      break;
#endif

#if 0
    case HSG_OP_TYPE_FINALIZE:
      {
        fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                "HS_FINALIZE(%s,args,shared.f[block_warp_idx],shared.l,\n"
                "                block_warp_idx,warp_lane_idx,warp_gmem_idx,\n"
                "                r%-3u",
                ops->a == 1 ? "true" : "false",
                1);

#define HS_WARP_FINALIZE_PRETTY_PRINT  8

        for (uint32_t r=2; r<=hsg_config.thread.regs; r++)
          {
            if (r % HS_WARP_FINALIZE_PRETTY_PRINT == 1)
              fprintf(files[HSG_FILE_TYPE_SOURCE].file,",\n");
            else
              fprintf(files[HSG_FILE_TYPE_SOURCE].file,",");

            fprintf(files[HSG_FILE_TYPE_SOURCE].file,"r%-3u",r);
          }

        fprintf(files[HSG_FILE_TYPE_SOURCE].file,");\n");
      }
      break;
#endif

    case HSG_OP_TYPE_BLOCK_SYNC:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "__syncthreads();\n");
      break;

    case HSG_OP_TYPE_BS_FRAC_PRED:
      {
        if (ops->m == 0)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (warp_idx < args.bs.full)\n");
          }
        else
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "else if (args.bs.frac == %u)\n",
                    ops->w);
          }
      }
      break;

#if 0 // DELETED
    case HSG_OP_TYPE_BX_MERGE_V_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s * const smem_v = shared.m%u[0] + threadIdx.x; \n",
              type,ops->a);
      break;
#endif

    case HSG_OP_TYPE_BS_MERGE_H_PREAMBLE:
      if (ops->c == 0)
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "%s * smem_l = shared.m%u[block_warp_idx    ] + warp_lane_idx;        \n"
                  "%s * smem_r = shared.m%u[block_warp_idx ^ 1] + (warp_lane_idx ^ %u); \n",
                  type,ops->a,
                  type,ops->a,hsg_config.warp.lanes-1);
        }
      else
        {
          fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                  "smem_l = shared.m%u[block_warp_idx    ] + warp_lane_idx;        \n"
                  "smem_r = shared.m%u[block_warp_idx ^ 1] + (warp_lane_idx ^ %u); \n",
                  ops->a,
                  ops->a,hsg_config.warp.lanes-1);
        }
      break;

    case HSG_OP_TYPE_BC_MERGE_H_PREAMBLE:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "%s const * const gmem_l = args.vout + (warp_gmem_base + block_warp_idx * %u); \n"
              "%s       * const smem_l = shared.m%u[block_warp_idx] + warp_lane_idx;         \n"
              "%s       * const smem_v = shared.m%u[0] + threadIdx.x;                        \n",
              type,hsg_config.warp.lanes,
              type,ops->a,
              type,ops->a);
      break;

    case HSG_OP_TYPE_BX_MERGE_H_PRED:
      fprintf(files[HSG_FILE_TYPE_SOURCE].file,
              "if (threadIdx.x < %u)\n",
              ops->a * hsg_config.warp.lanes);
      break;

    case HSG_OP_TYPE_BS_ACTIVE_PRED:
      {
        const struct hsg_merge* const m = merge + ops->a;

        if (m->warps <= 32)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (((1u << block_warp_idx) & 0x%08X) != 0)\n",
                    m->levels[ops->b].active.b32a2[0]);
          }
        else
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "if (((1UL << block_warp_idx) & 0x%08X%08XL) != 0L)\n",
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
                    "if (merge_idx < args.fm.full) \n");
          }
        else if (ops->b > 1)
          {
            fprintf(files[HSG_FILE_TYPE_SOURCE].file,
                    "else if (args.fm.frac == %u) \n",
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

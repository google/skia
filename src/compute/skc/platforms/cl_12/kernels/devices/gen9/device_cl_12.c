/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/cl/assert_cl.h"

#include "tile.h"
#include "raster.h"
#include "macros.h"

#include "config_cl.h"
#include "runtime_cl_12.h"

#include "kernel_cl_12.h"
#include "device_cl_12.h"

#include "hs/cl/hs_cl_launcher.h"
#include "hs/cl/intel/gen8/u64/hs_target.h"

//
//
//

#define SKC_KERNEL_SPIRV  0
#define SKC_KERNEL_BINARY 1
#define SKC_KERNEL_SRC    0

//
//
//

#if   SKC_KERNEL_SPIRV

#include "inl/block_pool_init.pre.spv.inl"
#include "inl/paths_copy.pre.spv.inl"
#include "inl/fills_expand.pre.spv.inl"
#include "inl/rasterize.pre.spv.inl"
#include "inl/segment_ttrk.pre.spv.inl"
#include "inl/rasters_alloc.pre.spv.inl"
#include "inl/prefix.pre.spv.inl"
#include "inl/place.pre.spv.inl"
#include "inl/segment_ttck.pre.spv.inl"
#include "inl/render.pre.spv.inl"
#include "inl/paths_reclaim.pre.spv.inl"
#include "inl/rasters_reclaim.pre.spv.inl"

#elif SKC_KERNEL_BINARY

#include "inl/block_pool_init.pre.bin.inl"
#include "inl/paths_copy.pre.bin.inl"
#include "inl/fills_expand.pre.bin.inl"
#include "inl/rasterize.pre.bin.inl"
#include "inl/segment_ttrk.pre.bin.inl"
#include "inl/rasters_alloc.pre.bin.inl"
#include "inl/prefix.pre.bin.inl"
#include "inl/place.pre.bin.inl"
#include "inl/segment_ttck.pre.bin.inl"
#include "inl/render.pre.bin.inl"
#include "inl/paths_reclaim.pre.bin.inl"
#include "inl/rasters_reclaim.pre.bin.inl"

#elif SKC_KERNEL_SRC

#include "inl/block_pool_init.pre.src.inl"
#include "inl/paths_copy.pre.src.inl"
#include "inl/fills_expand.pre.src.inl"
#include "inl/rasterize.pre.src.inl"
#include "inl/segment_ttrk.pre.src.inl"
#include "inl/rasters_alloc.pre.src.inl"
#include "inl/prefix.pre.src.inl"
#include "inl/place.pre.src.inl"
#include "inl/segment_ttck.pre.src.inl"
#include "inl/render.pre.src.inl"
#include "inl/paths_reclaim.pre.src.inl"
#include "inl/rasters_reclaim.pre.src.inl"

#endif

//
// FIXME -- THE CONFIG INITIALIZATION IS ONLY HERE TEMPORARILY
//
// FIXME -- move these to log2 values where appropriate
//

static
struct skc_config const config =
  {
    .suballocator = {
      .host = {
        .size       = 1024 * 1024, // words
        .subbufs    = 1024 // must be <= (1 << (8 * sizeof(skc_subbuf_id_t)))
      },
      .device = {
        .size       = 128 * 1024 * 1024,
        .subbufs    = 1024 // must be <= (1 << (8 * sizeof(skc_subbuf_id_t)))
      }
    },

    .scheduler = {
      .size         = 4096 // 128 // FIXME -- this is just for testing -- way too big -- schedulees should bring their own state
    },

    .subblock = {
      .words        = SKC_DEVICE_SUBBLOCK_WORDS,                         // words per subblock -- pow2
      .bytes        = SKC_DEVICE_SUBBLOCK_WORDS * sizeof(skc_uint)       // bytes per subblock -- pow2
    },

    .block = {
      .words        = SKC_DEVICE_BLOCK_WORDS,                            // words per block     -- pow2
      .bytes        = SKC_DEVICE_BLOCK_WORDS * sizeof(skc_uint),         // bytes per block     -- pow2
      .subblocks    = SKC_DEVICE_BLOCK_WORDS / SKC_DEVICE_SUBBLOCK_WORDS // subblocks per block -- block.bytes >= subblock.bytes
    },

    .block_pool = {
      .pool_size    = 524288, // blocks in pool -- 128 MB
      .ring_pow2    = 524288, // blocks in pool rounded up pow2
      .ring_mask    = 524288 - 1
    },

    .cq_pool     = {
#ifndef NDEBUG
       .cq_props    = CL_QUEUE_PROFILING_ENABLE,
#else
       .cq_props    = 0,
#endif
      .size         = 8
    },

    .handle_pool = {
      .size         = 262144,  // large fraction of block pool size (for now, 1:2)
      .width        = SKC_RECLAIM_ARRAY_SIZE,
      .recs         = 256      // too many?  too few?
    },

    .tile = {
      .width        = SKC_TILE_WIDTH,                  // tile width  in pixels
      .height       = SKC_TILE_HEIGHT,                 // tile height in pixels
      .ratio        = SKC_TILE_HEIGHT / SKC_TILE_WIDTH // subblocks per TTPB
    },

    .paths_copy = {

      .buffer = {
        .count      = 16   // # of subbufs in buffer
      },

      .subbuf = {
        .count      = 1024 // # of blocks/commands in subbuf
      },

      .block = {
        .subbuf     = SKC_DEVICE_BLOCK_WORDS * sizeof(skc_uint) * 1024,     // block.bytes * subbuf.blocks -- multiple of CL_DEVICE_MEM_BASE_ADDR_ALIGN
        .buffer     = SKC_DEVICE_BLOCK_WORDS * sizeof(skc_uint) * 1024 * 16 // block.bytes * subbuf.blocks * subbuf.count
      },

      .command = {
        .subbuf     = sizeof(skc_uint) * 1024,     // sizeof(skc_uint) * subbuf.blocks -- multiple of CL_DEVICE_MEM_BASE_ADDR_ALIGN
        .buffer     = sizeof(skc_uint) * 1024 * 16 // sizeof(skc_uint) * subbuf.blocks * subbuf.count
      },

      // skc_uint paths_lowat;
    },

    .raster_cohort = {
      .path_ids = {
        .elem_count = 8192,
        .snap_count = 1024 // FIXME -- THIS SHOULD BE WAYYYY BIGGER
      },

      .transforms = {
        .elem_count = 8192,
        .snap_count = 1024 // FIXME -- THIS SHOULD BE WAYYYY BIGGER
      },

      .clips = {
        .elem_count = 8192,
        .snap_count = 1024 // FIXME -- THIS SHOULD BE WAYYYY BIGGER
      },

      .fill = {
        .elem_count = 8192,
        .snap_count = 1024 // FIXME -- THIS SHOULD BE WAYYYY BIGGER
      },

      .raster_ids = {
        .elem_count = 8192,
        .snap_count = (1<<SKC_TTRK_HI_BITS_COHORT) // 256
      },

      .expand = {
        .cmds       = 1024*128,
      },

      .rasterize = {
        .keys       = 1024*1024
      }
    },

    .composition = {
      .cmds = {
        .elem_count = 1024*16,
        .snap_count = 1024
      },
      .raster_ids = {
        .elem_count = 1024*1024
      },
      .keys = {
        .elem_count = 1024*1024,
      }
    },
  };

//
//
//

static char const cl_build_options_optimized[] =
  "-cl-std=CL1.2 "
  "-cl-single-precision-constant "
  "-cl-denorms-are-zero "
  "-cl-mad-enable "
  "-cl-no-signed-zeros "
  "-cl-fast-relaxed-math "
  "-cl-kernel-arg-info ";

static char const cl_build_options_debug[] =
  "-cl-std=CL1.2 -cl-kernel-arg-info -g"; // -s c:/users/allanmac/home/google/skia_internal/src/compute/skc";

// #define SKC_BUILD_OPTIONS cl_build_options_debug
#define SKC_BUILD_OPTIONS    cl_build_options_optimized

//
//
//

struct skc_program_source
{
  char   const * name;
  char   const * options;
  char   const * src;
  size_t const   srclen;
};

//
// THIS IS A RELATIVELY COMPACT WAY OF DECLARING EACH PROGRAM SOURCE
// AND ITS BUILD OPTIONS
//

union skc_program_sources
{
  struct {
    struct skc_program_source block_pool_init;
    struct skc_program_source paths_copy;
    struct skc_program_source fills_expand;
    struct skc_program_source rasterize;
    struct skc_program_source segment_ttrk;
    struct skc_program_source rasters_alloc;
    struct skc_program_source prefix;
    struct skc_program_source place;
    struct skc_program_source segment_ttck;
    struct skc_program_source render;
    struct skc_program_source paths_reclaim;
    struct skc_program_source rasters_reclaim;
  };
  struct skc_program_source   sources[];
};

typedef size_t * (*skc_grid_shaper)(size_t    const work_size,
                                    cl_uint * const work_dim,
                                    size_t  * const global_work_size,
                                    size_t  * const local_work_size);
struct skc_program_kernel
{
  char const *         name;
  skc_grid_shaper      shaper;
  skc_device_kernel_id id;
};

union skc_program_kernels
{
  struct {
    struct skc_program_kernel block_pool_init[2];
    struct skc_program_kernel paths_copy     [2];
    struct skc_program_kernel fills_expand   [1];
    struct skc_program_kernel rasterize      [6];
    struct skc_program_kernel segment_ttrk   [1];
    struct skc_program_kernel rasters_alloc  [1];
    struct skc_program_kernel prefix         [1];
    struct skc_program_kernel place          [1];
    struct skc_program_kernel segment_ttck   [1];
    struct skc_program_kernel render         [1];
    struct skc_program_kernel paths_reclaim  [1];
    struct skc_program_kernel rasters_reclaim[1];
  };
  struct skc_program_kernel   kernels[];
};

//
//
//

#if     SKC_KERNEL_SPIRV  // PROGRAM IS SPIR-V
#define SKC_KERNEL_SUFFIX(n) n ## _pre_spv
#elif   SKC_KERNEL_BINARY // PROGRAM IS BINARY
#define SKC_KERNEL_SUFFIX(n) n ## _pre_ir
#elif   SKC_KERNEL_SRC    // PROGRAM IS SOURCE CODE
#define SKC_KERNEL_SUFFIX(n) n ## _pre_cl
#else
#error  "SKC_KERNEL_???"
#endif

//
//
//

#define SKC_PROGRAM_SOURCE_EXPAND(k,s,o) .k = { SKC_STRINGIFY(k), o, s, sizeof(s) }
#define SKC_PROGRAM_SOURCE(k,o)          SKC_PROGRAM_SOURCE_EXPAND(k,SKC_KERNEL_SUFFIX(k),o)
#define SKC_PROGRAM_KERNEL(k)            "skc_kernel_" SKC_STRINGIFY(k), SKC_CONCAT(skc_device_shaper_,k)

//
//
//

static
size_t *
skc_device_shaper_block_pool_init_ids(size_t    const work_size,
                                      cl_uint * const work_dim,
                                      size_t  * const work_global,
                                      size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = work_size;

  return NULL; // let runtime figure out local work size
}

static
size_t *
skc_device_shaper_block_pool_init_atomics(size_t    const work_size,
                                          cl_uint * const work_dim,
                                          size_t  * const work_global,
                                          size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = 2;

  return NULL; // let runtime figure out local work size
}

static
size_t *
skc_device_shaper_paths_alloc(size_t    const work_size,
                              cl_uint * const work_dim,
                              size_t  * const work_global,
                              size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = 1;

  return NULL; // let runtime figure out local work size
}


static
size_t *
skc_device_shaper_paths_copy(size_t    const work_size,
                             cl_uint * const work_dim,
                             size_t  * const work_global,
                             size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_PATHS_COPY_SUBGROUP_SIZE * work_size;
#if 0
  work_local [0] = SKC_PATHS_COPY_SUBGROUP_SIZE;

  return work_local;
#else
  return NULL; // let runtime figure out local work size
#endif
}

static
size_t *
skc_device_shaper_fills_expand(size_t    const work_size,
                               cl_uint * const work_dim,
                               size_t  * const work_global,
                               size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_FILLS_EXPAND_SUBGROUP_SIZE * work_size;
  work_local [0] = SKC_FILLS_EXPAND_SUBGROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_rasterize(size_t    const work_size,
                            cl_uint * const work_dim,
                            size_t  * const work_global,
                            size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_RASTERIZE_SUBGROUP_SIZE * work_size;
  work_local [0] = SKC_RASTERIZE_SUBGROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_rasterize_all(size_t    const work_size,
                                cl_uint * const work_dim,
                                size_t  * const work_global,
                                size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasterize_lines(size_t    const work_size,
                                  cl_uint * const work_dim,
                                  size_t  * const work_global,
                                  size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasterize_quads(size_t    const work_size,
                                  cl_uint * const work_dim,
                                  size_t  * const work_global,
                                  size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasterize_cubics(size_t    const work_size,
                                   cl_uint * const work_dim,
                                   size_t  * const work_global,
                                   size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasterize_rat_quads(size_t    const work_size,
                                      cl_uint * const work_dim,
                                      size_t  * const work_global,
                                      size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasterize_rat_cubics(size_t    const work_size,
                                       cl_uint * const work_dim,
                                       size_t  * const work_global,
                                       size_t  * const work_local)
{
  return skc_device_shaper_rasterize(work_size,work_dim,work_global,work_local);
}

static
size_t *
skc_device_shaper_rasters_alloc(size_t    const work_size,
                                cl_uint * const work_dim,
                                size_t  * const work_global,
                                size_t  * const work_local)
{
  // round up to whole groups
  size_t gs = SKC_ROUND_UP(work_size,SKC_RASTERS_ALLOC_GROUP_SIZE);

  work_dim   [0] = 1;
  work_global[0] = gs;
  work_local [0] = SKC_RASTERS_ALLOC_GROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_segment_ttrk(size_t    const work_size,
                               cl_uint * const work_dim,
                               size_t  * const work_global,
                               size_t  * const work_local)
{
  // work_size is number of keys -- round up to a whole slab
  size_t keys_ru = SKC_ROUND_UP(work_size,HS_SLAB_WIDTH*HS_SLAB_HEIGHT);

  work_dim   [0] = 1;
  work_global[0] = keys_ru / HS_SLAB_HEIGHT;
  work_local [0] = HS_SLAB_WIDTH; // or just return NULL

  return work_local;
}

static
size_t *
skc_device_shaper_segment_ttck(size_t    const work_size,
                               cl_uint * const work_dim,
                               size_t  * const work_global,
                               size_t  * const work_local)
{
  // work_size is number of keys -- round up to a whole slab
  size_t keys_ru = SKC_ROUND_UP(work_size,HS_SLAB_WIDTH*HS_SLAB_HEIGHT);

  work_dim   [0] = 1;
  work_global[0] = keys_ru / HS_SLAB_HEIGHT;
  work_local [0] = HS_SLAB_WIDTH; // or just return NULL

  return work_local;
}

static
size_t *
skc_device_shaper_prefix(size_t    const work_size,
                         cl_uint * const work_dim,
                         size_t  * const work_global,
                         size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_PREFIX_SUBGROUP_SIZE * work_size;
  work_local [0] = SKC_PREFIX_SUBGROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_place(size_t    const work_size,
                        cl_uint * const work_dim,
                        size_t  * const work_global,
                        size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_PLACE_SUBGROUP_SIZE * work_size;
  work_local [0] = SKC_PLACE_SUBGROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_render(size_t    const work_size,
                         cl_uint * const work_dim,
                         size_t  * const work_global,
                         size_t  * const work_local)
{
  work_dim   [0] = 1;
  work_global[0] = SKC_RENDER_SUBGROUP_SIZE * work_size;
  work_local [0] = SKC_RENDER_SUBGROUP_SIZE;

  return work_local;
}

static
size_t *
skc_device_shaper_paths_reclaim(size_t    const work_size,
                                cl_uint * const work_dim,
                                size_t  * const work_global,
                                size_t  * const work_local)
{
  assert(work_size == SKC_RECLAIM_ARRAY_SIZE);

  work_dim   [0] = 1;
  work_global[0] = SKC_RECLAIM_ARRAY_SIZE * SKC_PATHS_RECLAIM_SUBGROUP_SIZE;

  return NULL; // let runtime figure out local work size
}

static
size_t *
skc_device_shaper_rasters_reclaim(size_t    const work_size,
                                  cl_uint * const work_dim,
                                  size_t  * const work_global,
                                  size_t  * const work_local)
{
  assert(work_size == SKC_RECLAIM_ARRAY_SIZE);

  work_dim   [0] = 1;
  work_global[0] = SKC_RECLAIM_ARRAY_SIZE * SKC_PATHS_RECLAIM_SUBGROUP_SIZE;

  return NULL; // let runtime figure out local work size
}

//
//
//

static union skc_program_sources const program_sources = {
  SKC_PROGRAM_SOURCE(block_pool_init,SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(paths_copy,     SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(fills_expand,   SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(rasterize,      SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(segment_ttrk,   SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(rasters_alloc,  SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(prefix,         SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(place,          SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(segment_ttck,   SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(render,         SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(paths_reclaim,  SKC_BUILD_OPTIONS),
  SKC_PROGRAM_SOURCE(rasters_reclaim,SKC_BUILD_OPTIONS)
};

static union skc_program_kernels const program_kernels = {

  .block_pool_init = { { SKC_PROGRAM_KERNEL(block_pool_init_ids),     SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_IDS     },
                       { SKC_PROGRAM_KERNEL(block_pool_init_atomics), SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_ATOMICS } },

  .paths_copy      = { { SKC_PROGRAM_KERNEL(paths_alloc),             SKC_DEVICE_KERNEL_ID_PATHS_ALLOC             },
                       { SKC_PROGRAM_KERNEL(paths_copy) ,             SKC_DEVICE_KERNEL_ID_PATHS_COPY              } },

  .fills_expand    = { { SKC_PROGRAM_KERNEL(fills_expand),            SKC_DEVICE_KERNEL_ID_FILLS_EXPAND            } },

  .rasterize       = { { SKC_PROGRAM_KERNEL(rasterize_all),           SKC_DEVICE_KERNEL_ID_RASTERIZE_ALL           },
                       { SKC_PROGRAM_KERNEL(rasterize_lines),         SKC_DEVICE_KERNEL_ID_RASTERIZE_LINES         },
                       { SKC_PROGRAM_KERNEL(rasterize_quads),         SKC_DEVICE_KERNEL_ID_RASTERIZE_QUADS         },
                       { SKC_PROGRAM_KERNEL(rasterize_cubics),        SKC_DEVICE_KERNEL_ID_RASTERIZE_CUBICS        },
                       { SKC_PROGRAM_KERNEL(rasterize_rat_quads),     SKC_DEVICE_KERNEL_ID_RASTERIZE_RAT_QUADS     },
                       { SKC_PROGRAM_KERNEL(rasterize_rat_cubics),    SKC_DEVICE_KERNEL_ID_RASTERIZE_RAT_CUBICS    } },

  .segment_ttrk    = { { SKC_PROGRAM_KERNEL(segment_ttrk),            SKC_DEVICE_KERNEL_ID_SEGMENT_TTRK            } },

  .rasters_alloc   = { { SKC_PROGRAM_KERNEL(rasters_alloc),           SKC_DEVICE_KERNEL_ID_RASTERS_ALLOC           } },

  .prefix          = { { SKC_PROGRAM_KERNEL(prefix),                  SKC_DEVICE_KERNEL_ID_PREFIX                  } },

  .place           = { { SKC_PROGRAM_KERNEL(place),                   SKC_DEVICE_KERNEL_ID_PLACE                   } },

  .segment_ttck    = { { SKC_PROGRAM_KERNEL(segment_ttck) ,           SKC_DEVICE_KERNEL_ID_SEGMENT_TTCK            } },

  .render          = { { SKC_PROGRAM_KERNEL(render),                  SKC_DEVICE_KERNEL_ID_RENDER                  } },

  .paths_reclaim   = { { SKC_PROGRAM_KERNEL(paths_reclaim),           SKC_DEVICE_KERNEL_ID_PATHS_RECLAIM           } },

  .rasters_reclaim = { { SKC_PROGRAM_KERNEL(rasters_reclaim),         SKC_DEVICE_KERNEL_ID_RASTERS_RECLAIM         } }
};

//
//
//

struct skc_device
{
  //
  // FIXME -- an OpenCL 2.1+ device would clone these kernels in a
  // multithreaded system.
  //
  // Not having the ability to clone kernels (yet set their sticky
  // args) was an oversight in previous versions of OpenCL.
  //
  // For now, we can probably get away with just a single kernel
  // instance as long as args are set and the kernel is launched
  // before having its arguments stomped on.
  //
  cl_kernel kernels [SKC_DEVICE_KERNEL_ID_COUNT];
  size_t    reqd_szs[SKC_DEVICE_KERNEL_ID_COUNT][3];
};

//
// CREATE KERNELS
//

static
void
skc_device_create_kernels(struct skc_runtime              * const runtime,
                          struct skc_program_kernel const * const kernels,
                          skc_uint                          const kernel_count,
                          cl_program                              program)
{
  for (skc_uint ii=0; ii<kernel_count; ii++)
    {
      cl_int cl_err;

      char     const * name = kernels[ii].name;
      skc_uint const   id   = kernels[ii].id;

      fprintf(stderr,"\t\"%s\"\n",name);

      // create the kernel
      runtime->device->kernels[id] = clCreateKernel(program,name,&cl_err); cl_ok(cl_err);

      //
      // release program now
      //
      // FIXME -- if/when we multithread then we need to clone kernels
      // (>=2.1) or keep programs around (<=2.0)
      //

      // get workgroup size
      cl(GetKernelWorkGroupInfo(runtime->device->kernels[id],
                                runtime->cl.device_id,
                                CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                sizeof(runtime->device->reqd_szs[0]),
                                runtime->device->reqd_szs[id],
                                NULL));

      //
      // GEN9+ PROBING
      //
#define SKC_TARGET_GEN9
#ifdef  SKC_TARGET_GEN9

#define CL_DEVICE_SUB_GROUP_SIZES_INTEL         0x4108
#define CL_KERNEL_SPILL_MEM_SIZE_INTEL          0x4109
#define CL_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL  0x410A

      cl_ulong spill_mem_size;

      cl(GetKernelWorkGroupInfo(runtime->device->kernels[id],
                                runtime->cl.device_id,
                                CL_KERNEL_SPILL_MEM_SIZE_INTEL,
                                sizeof(spill_mem_size),
                                &spill_mem_size,
                                NULL));

      fprintf(stderr,"\t\tspill mem size: %lu bytes\n",
              (unsigned long)spill_mem_size);

      cl_ulong local_mem_size;

      cl(GetKernelWorkGroupInfo(runtime->device->kernels[id],
                                runtime->cl.device_id,
                                CL_KERNEL_LOCAL_MEM_SIZE,
                                sizeof(local_mem_size),
                                &local_mem_size,
                                NULL));

      fprintf(stderr,"\t\tlocal mem size: %lu bytes\n",
              (unsigned long)local_mem_size);
#endif
    }
}

static
void
skc_device_build_program(struct skc_runtime              * const runtime,
                         struct skc_program_source const * const source,
                         struct skc_program_kernel const * const kernels,
                         skc_uint                          const kernel_count)
{
  cl_program program;

  fprintf(stderr,"%-20s: ",source->name);

  cl_int cl_err;

#if   SKC_KERNEL_SPIRV // PROGRAM IS SPIR-V

  fprintf(stderr,"Creating (SPIR-V) ... ");

  program = clCreateProgramWithIL(runtime->cl.context,
                                  source->src,
                                  source->srclen,
                                  &cl_err);

#elif SKC_KERNEL_BINARY // PROGRAM IS BINARY

  fprintf(stderr,"Creating (Binary) ... ");

  cl_int status;
  program = clCreateProgramWithBinary(runtime->cl.context,
                                      1,
                                      &runtime->cl.device_id,
                                      &source->srclen,
                                      (unsigned char const *[]){ source->src },
                                      &status,
                                      &cl_err);

#elif SKC_KERNEL_SRC // PROGRAM IS SOURCE CODE

  fprintf(stderr,"Creating (Source) ... ");

  program = clCreateProgramWithSource(runtime->cl.context,
                                      1,
                                      (char const *[]){ source->src },
                                      &source->srclen,
                                      &cl_err);
#else

#error "SKC_KERNEL_???"

#endif

  cl_ok(cl_err);

  fprintf(stderr,"Building ... ");

  // build the program
  cl(BuildProgram(program,
                  1,
                  &runtime->cl.device_id,
                  source->options, // build options are ignored by binary
                  NULL,
                  NULL));

  fprintf(stderr,"Done\n");

  // build the kernels
  skc_device_create_kernels(runtime,kernels,kernel_count,program);

  // we're done with program for now
  // can always recover it from a kernel instance
  cl(ReleaseProgram(program));
}

//
// RELEASE KERNELS
//

static
void
skc_device_release_kernels(struct skc_device * const device)
{
  for (skc_int ii=0; ii<SKC_COUNT_OF(device->kernels); ii++)
    cl(ReleaseKernel(device->kernels[ii]));
}



cl_kernel
skc_device_acquire_kernel(struct skc_device  * const device,
                          skc_device_kernel_id const type)
{
  cl_kernel kernel = device->kernels[type];

  cl(RetainKernel(kernel));

  return kernel;
}


void
skc_device_release_kernel(struct skc_device  * const device,
                          cl_kernel                  kernel)
{
  cl(ReleaseKernel(kernel));
}

//
// INITIALIZE KERNEL ARGS
//
// FIXME
//
// pre-assign any kernel arguments that are never going to change --
// for example, the block pool
//

//
//
//

#define SKC_DEVICE_BUILD_PROGRAM(p) \
  skc_device_build_program(runtime,&program_sources.p,program_kernels.p,SKC_COUNT_OF(program_kernels.p))


void
skc_device_create(struct skc_runtime * const runtime)
{
  struct skc_device * const device = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*device));

  // hang device off of runtime
  runtime->device = device;

  // hang config off of runtime
  runtime->config = &config;

  // create kernels
  SKC_DEVICE_BUILD_PROGRAM(block_pool_init);
  SKC_DEVICE_BUILD_PROGRAM(paths_copy);
  SKC_DEVICE_BUILD_PROGRAM(fills_expand);
  SKC_DEVICE_BUILD_PROGRAM(rasterize);
  SKC_DEVICE_BUILD_PROGRAM(segment_ttrk);
  SKC_DEVICE_BUILD_PROGRAM(rasters_alloc);
  SKC_DEVICE_BUILD_PROGRAM(prefix);
  SKC_DEVICE_BUILD_PROGRAM(place);
  SKC_DEVICE_BUILD_PROGRAM(segment_ttck);
  SKC_DEVICE_BUILD_PROGRAM(render);
  SKC_DEVICE_BUILD_PROGRAM(paths_reclaim);
  SKC_DEVICE_BUILD_PROGRAM(rasters_reclaim);

  // create HotSort instance
  runtime->hs = hs_cl_create(&hs_target,
                             runtime->cl.context,
                             runtime->cl.device_id);
}

void
skc_device_dispose(struct skc_runtime * const runtime)
{
  //
  // FIXME -- dispose of programs, kernels, etc.
  //

  skc_runtime_host_perm_free(runtime,runtime->device);

  // dispose of hotsort etc.
}

//
// FIXME -- just pass the device type
//

void
skc_device_enqueue_kernel(struct skc_device  * const device,
                          skc_device_kernel_id const type,
                          cl_command_queue           cq,
                          cl_kernel                  kernel,
                          size_t               const work_size,
                          cl_uint                    num_events_in_wait_list,
                          cl_event const     * const event_wait_list,
                          cl_event           * const event)
{
  if (work_size == 0)
    return;

  cl_uint  work_dim   [1];
  size_t   work_global[3];
  size_t   work_local [3];

  size_t * work_local_ptr = program_kernels.kernels[type].shaper(work_size,
                                                                 work_dim,
                                                                 work_global,
                                                                 work_local);
  cl(EnqueueNDRangeKernel(cq,
                          kernel,// device->kernels[type],
                          work_dim[0],
                          NULL,
                          work_global,
                          work_local_ptr,
                          num_events_in_wait_list,
                          event_wait_list,
                          event));
}

//
//
//

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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

#include "common/cl/assert_cl.h"

#include "context.h"
#include "handle.h"
#include "grid.h"
#include "path.h"
#include "path_builder.h"

#include "config_cl.h"
#include "export_cl_12.h"
#include "runtime_cl_12.h"
#include "path_builder_cl_12.h"

//
// OpenCL 1.2 devices support mapping of buffers into the host address
// space.
//
// Mapped buffers must be aligned on MIN_DATA_TYPE_ALIGN_SIZE bit
// boundary (e.g. 128 bytes).  This complicates coordinating sharing
// of data between the host and the device.
//
// Some OpenCL 2.0 devices support fine-grained shared virtual memory
// pointers with byte-addressing and allow simpler coordination
// strategies at the cost of maintaining cache coherency.
//
// The path builder is focused on moving bulk path data from the host
// into the device-managed "block" memory pool and arranging it into a
// SIMT/SIMD-friendly data structure that can be efficiently read by
// the rasterizer.
//
// Note that one simplifying assumption is that the maximum length of
// a *single* path can't be larger than what fits in the single extent
// (which is split into M subbuffers).  This would be a very long path
// and a legitimate size limitation.
//
// For some systems, it may be appropriate to never pull path data
// into the device-managed block pool and instead present the path
// data to the device in a temporarily available allocated memory
// "zone" of paths that can be discarded all at once.
//
// For other systems, it may be appropriate to simply copy the path
// data from host to device.
//
// But the majority of OpenCL (and VK, MTL, DX12) devices we'll be
// targeting support basic map/unmap functionality similar to OpenCL
// 1.2.  Furthermore, not all OpenCL 2.0 devices support fine-grained
// sharing of memory and still require a map/unmap step... but note
// that they all support byte-aligned mapping and subbuffers.
//
// The general strategy that this particular CL_12 implementation uses
// is to allocate a large mappable bulk-data path buffer and an
// auxilary mappable command buffer.
//
// The buffers are split into a reasonable number of properly aligned
// subbuffers to enable simultaneous host and device access.
//

//
// Blocks:
//   1 extent
//   M mapped subbuffers (configurable) to allow for concurrency
//
// Commands:
//   1 extent
//   M mapped subbuffers (configurable) to allow for concurrency
//
// Spans:
//   M hi/lo structures
//
// { cl_sub, void*, event, base }
//
// - size of sub buffer
// - remaining
//
// - counts
//

//
// For any kernel launch, at most one path will be discontiguous and
// defined across two sub-buffers.
//
// Nodes are updated locally until full and then stored so they will
// never be incomplete.  Headers are stored locally until the path is
// ended so they will never be incomplete.
//
// A line, quad or cubic acquires 4/6/8 segments which may be spread
// across one or more congtiguous blocks.
//
// If a flush() occurs then the remaining columns of multi-segment
// paths are initialized with zero-length line, quad, cubic elements.
//
// Every block's command word has a type and a count acquired from a
// rolling counter.
//
// The kernel is passed two spans of blocks { base, count } to
// process.  The grid is must process (lo.count + hi.count) blocks.
//

struct skc_subbuffer_blocks
{
  cl_mem   device;
  void *   host;
};

struct skc_subbuffer_cmds
{
  cl_mem   device;
  void *   host;
  cl_event map;
};

//
// ringdex is an index with range [0, blocks-per-subbuf * subbufs-per-buffer )
//

typedef skc_uint skc_ringdex_t;

union skc_ringdex_expand
{
  div_t      qr;

  struct {
#ifndef SKC_DIV_REM_BEFORE_QUOT // offsetof(div_t,quot) != 0
    skc_uint subbuf;
    skc_uint block;
#else
    skc_uint block;
    skc_uint subbuf;
#endif
  };
};

//
// this record is executed by the grid
//

struct skc_release_record
{
  struct skc_path_builder_impl * impl; // back pointer to impl

  skc_grid_t                     grid; // pointer to scheduled grid

  skc_uint                       from; // inclusive starting index   : [from,to)
  skc_uint                       to;   // non-inclusive ending index : [from,to)
};

//
//
//

struct skc_path_builder_impl
{
  struct skc_path_builder       * path_builder;

  struct skc_runtime            * runtime;

  cl_command_queue                cq;

  struct {
    cl_kernel                     alloc;
    cl_kernel                     copy;
  } kernels;

  //
  // FIXME -- make this pointer to constant config
  //
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  struct {
    skc_uint                      subbufs;  // how many subbufs in the buffer?

    struct {
      skc_uint                    buffer;   // how many blocks in the buffer?
      skc_uint                    subbuf;   // how many blocks in a   subbuf?
    } blocks_per;
  } ring;
  //
  // ^^^^^^^^^^^ don't duplicate these constants ^^^^^^^^^^^^^^^^^^
  //

  struct {
    cl_mem                        buffer;   // backing buffer for blocks
    struct skc_subbuffer_blocks * subbufs;  // array of structures
  } blocks;

  struct {
    cl_mem                        buffer;   // backing buffer for commands
    struct skc_subbuffer_cmds   * subbufs;  // array of structures
  } cmds;

  struct {
    struct skc_release_record   * records;  // max release records is equal to max subbufs
    skc_path_t                  * paths;    // max paths is less than or equal to max commands
  } release;

  cl_mem                          reads;    // each kernel only requires one word to store the block pool "base"

  struct {
    skc_uint                      rolling;  // rolling counter used by cmds to map to block pool alloc
    skc_ringdex_t                 from;
    skc_ringdex_t                 to;
  } prev;

  struct {
    skc_ringdex_t                 from;
    skc_ringdex_t                 to;
  } curr;

  struct {
    struct skc_path_head        * head;     // pointer to local path header -- not written until path end
    struct skc_path_node        * node;     // pointer to local node -- may alias head until head is full

    struct {
      skc_uint                    rolling;  // rolling counter of wip node -- valid after one node is allocated
      union skc_tagged_block_id * next;     // next slot in node -- may initially point to head.ids
      skc_uint                    rem;      // how many id slots left in node block
    } ids;

    struct {
      skc_uint                    rem;      // how many subblocks left in block?
      skc_uint                    rolling;  // rolling counter of block of subblocks
      float                     * next;     // next subblock in current subblock block
      skc_uint                    idx;      // index of next subblock
    } subblocks;

    struct {
      skc_uint                    one;      // .block = 1
      skc_uint                    next;     // rolling counter used by cmds to map to block pool alloc
    } rolling;

    skc_ringdex_t                 to;       // ringdex of _next_available_ command/block in ring -- FIXME -- should be current
  } wip;
};

//
// FIXME -- move to a pow2 subbuffer size and dispense with division
// and modulo operations
//

static
union skc_ringdex_expand
skc_ringdex_expand(struct skc_path_builder_impl * const impl,
                   skc_ringdex_t                  const ringdex)
{
  return (union skc_ringdex_expand){
    .qr = div(ringdex,impl->ring.blocks_per.subbuf)
  };
}

static
void
skc_ringdex_wip_to_block_inc(struct skc_path_builder_impl * const impl)
{
  //
  // FIXME - which is faster?
  //
#if 1
  impl->wip.to  = (impl->wip.to + 1) % impl->ring.blocks_per.buffer;
#else
  impl->wip.to -= (impl->wip.to < impl->ring.blocks_per.buffer) ? -1 : impl->wip.to;
#endif

  // this path is too long -- for now assert() and die
  assert(impl->wip.to != impl->curr.from);
}

static
skc_ringdex_t
skc_ringdex_span(struct skc_path_builder_impl * const impl,
                 skc_ringdex_t                  const from,
                 skc_ringdex_t                  const to)
{
  return (to - from) % impl->ring.blocks_per.buffer;
}

static
void
skc_ringdex_wip_to_subbuf_inc(struct skc_path_builder_impl * const impl)
{
  union skc_ringdex_expand const to = skc_ringdex_expand(impl,impl->wip.to);

  // nothing to do if this is the first block in the subbuf
  if (to.block == 0)
    return;

  skc_uint const new_subbuf = (to.subbuf + 1) % impl->ring.subbufs;

  // otherwise increment and mod
  impl->wip.to = new_subbuf * impl->ring.blocks_per.subbuf;
}

static
skc_bool
skc_ringdex_curr_is_equal(struct skc_path_builder_impl * const impl)
{
  return impl->curr.from == impl->curr.to;
}

static
skc_bool
skc_ringdex_prev_is_equal(struct skc_path_builder_impl * const impl)
{
  return impl->prev.from == impl->prev.to;
}

static
skc_uint
skc_ringdex_dont_map_last(struct skc_path_builder_impl * const impl,
                          skc_uint                       const to_block)
{
  // no blocks acquired OR this is last block in subbuf
  return !((impl->wip.to == impl->curr.to) || (to_block == 0));
}

//
//
//

static
struct skc_release_record *
skc_release_curr(struct skc_path_builder_impl * const impl)
{
  union skc_ringdex_expand curr_from = skc_ringdex_expand(impl,impl->curr.from);

  return impl->release.records + curr_from.subbuf;
}

//
// FIXME -- get rid of all distant config references -- grab them at all at creation time
//

static
void
skc_path_builder_pfn_begin(struct skc_path_builder_impl * const impl)
{
  // init header counters // { handle, blocks, nodes, prims }
  impl->wip.head->header = (union skc_path_header){
    .handle = 0,
    .blocks = 0,
    .nodes  = 0,
    .prims  = 0
  };

  // FIXME -- BOUNDS SHOULD USE SIMD4 TRICK AND NEGATE ONE OF THE CORNERS
  impl->wip.head->bounds  = (union skc_path_bounds){ +FLT_MIN, +FLT_MIN, -FLT_MIN, -FLT_MIN };

  // point wip ids at local head node
  impl->wip.ids.next      = impl->wip.head->tag_ids; // point to local head node
  impl->wip.ids.rem       = impl->runtime->config->block.words - SKC_PATH_HEAD_WORDS; // FIXME -- save this constant somewhere

  // start with no subblocks
  impl->wip.subblocks.rem = 0;
}

//
//
//

static
void
skc_path_builder_impl_finalize_node(struct skc_path_builder_impl * const impl)
{
#if 1
  //
  // FIXME -- a Duff's device might be optimal here but would have to
  // be customized per device since node's could be 16-128+ words
  //
  while (impl->wip.ids.rem > 0)
    {
      impl->wip.ids.rem      -= 1;
      impl->wip.ids.next->u32 = SKC_TAGGED_BLOCK_ID_INVALID;
      impl->wip.ids.next     += 1;
    }
#else
  memset(&impl->wip.ids.next->u32,
         SKC_TAGGED_BLOCK_ID_INVALID, // 0xFF
         sizeof(impl->wip.ids.next->u32) * impl->wip.ids.rem);

  impl->wip.ids.next += impl->wip.ids.rem;
  impl->wip.ids.rem   = 0;
#endif
}

//
//
//

static
void
skc_zero_float(skc_float * p, skc_uint rem)
{
  memset(p,0,sizeof(*p)*rem);
}

static
void
skc_path_builder_finalize_subblocks(struct skc_path_builder * const path_builder)
{
  //
  // FIXME -- it might be more performant to zero the remaining
  // columns in a subblock -- a subblock at a time -- instead of the
  // same column across all the subblocks
  //
#if 0
  while (path_builder->line.rem > 0)
    {
      --path_builder->line.rem;

      *path_builder->line.coords[0]++ = 0.0f;
      *path_builder->line.coords[1]++ = 0.0f;
      *path_builder->line.coords[2]++ = 0.0f;
      *path_builder->line.coords[3]++ = 0.0f;
    }

  while (path_builder->quad.rem > 0)
    {
      --path_builder->quad.rem;

      *path_builder->line.coords[0]++ = 0.0f;
      *path_builder->line.coords[1]++ = 0.0f;
      *path_builder->line.coords[2]++ = 0.0f;
      *path_builder->line.coords[3]++ = 0.0f;
      *path_builder->line.coords[4]++ = 0.0f;
      *path_builder->line.coords[5]++ = 0.0f;
    }

  while (path_builder->cubic.rem > 0)
    {
      --path_builder->cubic.rem;

      *path_builder->line.coords[0]++ = 0.0f;
      *path_builder->line.coords[1]++ = 0.0f;
      *path_builder->line.coords[2]++ = 0.0f;
      *path_builder->line.coords[3]++ = 0.0f;
      *path_builder->line.coords[4]++ = 0.0f;
      *path_builder->line.coords[5]++ = 0.0f;
      *path_builder->line.coords[6]++ = 0.0f;
      *path_builder->line.coords[7]++ = 0.0f;
    }
#else
  if (path_builder->line.rem > 0)
    {
      skc_zero_float(path_builder->line.coords[0],path_builder->line.rem);
      skc_zero_float(path_builder->line.coords[1],path_builder->line.rem);
      skc_zero_float(path_builder->line.coords[2],path_builder->line.rem);
      skc_zero_float(path_builder->line.coords[3],path_builder->line.rem);

      path_builder->line.rem = 0;
    }

  if (path_builder->quad.rem > 0)
    {
      skc_zero_float(path_builder->quad.coords[0],path_builder->quad.rem);
      skc_zero_float(path_builder->quad.coords[1],path_builder->quad.rem);
      skc_zero_float(path_builder->quad.coords[2],path_builder->quad.rem);
      skc_zero_float(path_builder->quad.coords[3],path_builder->quad.rem);
      skc_zero_float(path_builder->quad.coords[4],path_builder->quad.rem);
      skc_zero_float(path_builder->quad.coords[5],path_builder->quad.rem);

      path_builder->quad.rem = 0;
    }

  if (path_builder->cubic.rem > 0)
    {
      skc_zero_float(path_builder->cubic.coords[0],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[1],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[2],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[3],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[4],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[5],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[6],path_builder->cubic.rem);
      skc_zero_float(path_builder->cubic.coords[7],path_builder->cubic.rem);

      path_builder->cubic.rem = 0;
    }
#endif
}

//
//
//

static
void
skc_path_builder_impl_unmap(struct skc_path_builder_impl * const impl,
                            skc_uint                             from,
                            skc_uint                             to)
{
  // to might be out of range
  to = to % impl->ring.subbufs;

#if 0
  fprintf(stderr,"unmap: [%2u,%2u)\n",from,to);
#endif

  while (from != to) // 'to' might be out of range
    {
      // bring 'from' back in range
      from = from % impl->ring.subbufs;

      struct skc_subbuffer_blocks * const blocks = impl->blocks.subbufs + from;
      struct skc_subbuffer_cmds   * const cmds   = impl->cmds  .subbufs + from;

      cl(EnqueueUnmapMemObject(impl->cq,
                               blocks->device,
                               blocks->host,
                               0,NULL,NULL));

      cl(EnqueueUnmapMemObject(impl->cq,
                               cmds->device,
                               cmds->host,
                               0,NULL,NULL));

      // bring from back in range
      from = (from + 1) % impl->ring.subbufs;
    }
}

//
// FIXME -- reuse this in create()
//

static
void
skc_path_builder_impl_map(struct skc_path_builder_impl * const impl,
                          skc_uint                             from,
                          skc_uint                             to)
{
  // to might be out of range
  to = to % impl->ring.subbufs;

#if 0
  fprintf(stderr,"  map: [%2u,%2u)\n",from,to);
#endif

  while (from != to)
    {
      cl_int cl_err;

      struct skc_subbuffer_blocks * const blocks = impl->blocks.subbufs + from;
      struct skc_subbuffer_cmds   * const cmds   = impl->cmds  .subbufs + from;

      blocks->host = clEnqueueMapBuffer(impl->cq,
                                        blocks->device,
                                        CL_FALSE,
                                        CL_MAP_WRITE_INVALIDATE_REGION,
                                        0,impl->runtime->config->paths_copy.block.subbuf,
                                        0,NULL,NULL,
                                        &cl_err); cl_ok(cl_err);

      cl(ReleaseEvent(cmds->map));

      cmds->host   = clEnqueueMapBuffer(impl->cq,
                                        cmds->device,
                                        CL_FALSE,
                                        CL_MAP_WRITE_INVALIDATE_REGION,
                                        0,impl->runtime->config->paths_copy.command.subbuf,
                                        0,NULL,&cmds->map,
                                        &cl_err); cl_ok(cl_err);

      // bring from back in range
      from = (from + 1) % impl->ring.subbufs;
    }
  //
  // FIXME -- when we switch to out of order queues we'll need a barrier here
  //
}

//
//
//

static
void
skc_path_builder_release_dispose(struct skc_release_record    * const release,
                                 struct skc_path_builder_impl * const impl)
{
  struct skc_runtime * runtime = impl->runtime;

  if (release->from <= release->to) // no wrap
    {
      skc_path_t const * paths = impl->release.paths + release->from;
      skc_uint           count = release->to         - release->from;

      skc_grid_deps_unmap(runtime->deps,paths,count);
      skc_runtime_path_device_release(runtime,paths,count);
    }
  else // from > to implies wrap
    {
      skc_path_t const * paths_lo = impl->release.paths + release->from;
      skc_uint           count_lo = impl->ring.blocks_per.buffer - release->from;

      skc_grid_deps_unmap(runtime->deps,paths_lo,count_lo);
      skc_runtime_path_device_release(runtime,paths_lo,count_lo);

      skc_grid_deps_unmap(runtime->deps,impl->release.paths,release->to);
      skc_runtime_path_device_release(runtime,impl->release.paths,release->to);
    }

  release->to = release->from;
}

static
void
skc_path_builder_grid_pfn_dispose(skc_grid_t const grid)
{
  struct skc_release_record    * const release = skc_grid_get_data(grid);
  struct skc_path_builder_impl * const impl    = release->impl;

  skc_path_builder_release_dispose(release,impl);
}

static
void
// skc_path_builder_complete(struct skc_release_record * const release)
skc_path_builder_complete(skc_grid_t grid)
{
  //
  // notify deps that this grid is complete enough for other grids to
  // proceed
  //
  // the path builder still has some cleanup to do before all its
  // resources can be reused
  //
  skc_grid_complete(grid);
}

static
void
skc_path_builder_paths_copy_cb(cl_event event, cl_int status, skc_grid_t grid)
{
  SKC_CL_CB(status);

  struct skc_release_record * const release = skc_grid_get_data(grid);

  SKC_SCHEDULER_SCHEDULE(release->impl->runtime->scheduler,skc_path_builder_complete,grid);
}

//
//
//

static
void
skc_path_builder_grid_pfn_waiting(skc_grid_t const grid)
{
  struct skc_release_record    * const release = skc_grid_get_data(grid);
  struct skc_path_builder_impl * const impl    = release->impl;

  // 1. flush incomplete subblocks of path elements
  // 2. unmap subbuffer on cq.unmap
  // 3. flush cq.unmap
  // 4. launch kernel on cq.kernel but wait for unmap completion
  // 5. flush cq.kernel
  // 6. remap relevant subbuffers on cq.map but wait for kernel completion
  // 7. flush cq.map

  //
  // FIXME -- can be smarter about flushing if the wip paths are not
  // in the same subbuf as curr.to
  //
  // THIS IS IMPORTANT TO FIX
  //

  // flush incomplete subblocks
  skc_path_builder_finalize_subblocks(impl->path_builder);

  //
  // get range of subbufs that need to be unmapped
  //
  // note that impl->prev subbufs have already been unmapped
  //
  union skc_ringdex_expand       curr_from  = skc_ringdex_expand(impl,impl->curr.from);
  union skc_ringdex_expand       curr_to    = skc_ringdex_expand(impl,impl->curr.to);
  skc_uint                 const is_partial = curr_to.block > 0;
  skc_uint                 const unmap_to   = curr_to.subbuf + is_partial;

  //
  // unmap all subbufs in range [from,to)
  //
  skc_path_builder_impl_unmap(impl,curr_from.subbuf,unmap_to);

  //
  // launch kernels
  //
  skc_uint const pb_prev_span = skc_ringdex_span(impl,impl->prev.from,impl->prev.to);
  skc_uint const pb_curr_span = skc_ringdex_span(impl,impl->curr.from,impl->curr.to);
  skc_uint const pb_cmds      = pb_prev_span + pb_curr_span;

  //
  // 1) allocate blocks from pool
  //

  //
  // FIXME -- pack integers into struct/vector
  //
  cl(SetKernelArg(impl->kernels.alloc,0,SKC_CL_ARG(impl->runtime->block_pool.atomics.drw)));
  cl(SetKernelArg(impl->kernels.alloc,1,SKC_CL_ARG(impl->reads)));
  cl(SetKernelArg(impl->kernels.alloc,2,SKC_CL_ARG(curr_from.subbuf)));
  cl(SetKernelArg(impl->kernels.alloc,3,SKC_CL_ARG(pb_cmds)));

  skc_device_enqueue_kernel(impl->runtime->device,
                            SKC_DEVICE_KERNEL_ID_PATHS_ALLOC,
                            impl->cq,
                            impl->kernels.alloc,
                            1,
                            0,NULL,NULL);

  //
  // 2) copy blocks from unmapped device-accessible memory
  //

  //
  // FIXME -- pack integers into struct/vector and reduce 13 arguments down to 7
  //
  cl(SetKernelArg(impl->kernels.copy, 0,SKC_CL_ARG(impl->runtime->handle_pool.map.drw)));

  cl(SetKernelArg(impl->kernels.copy, 1,SKC_CL_ARG(impl->runtime->block_pool.ids.drw)));
  cl(SetKernelArg(impl->kernels.copy, 2,SKC_CL_ARG(impl->runtime->block_pool.blocks.drw)));
  cl(SetKernelArg(impl->kernels.copy, 3,SKC_CL_ARG(impl->runtime->block_pool.size->ring_mask)));

  cl(SetKernelArg(impl->kernels.copy, 4,SKC_CL_ARG(impl->reads)));
  cl(SetKernelArg(impl->kernels.copy, 5,SKC_CL_ARG(curr_from.subbuf)));

  cl(SetKernelArg(impl->kernels.copy, 6,SKC_CL_ARG(impl->cmds.buffer)));
  cl(SetKernelArg(impl->kernels.copy, 7,SKC_CL_ARG(impl->blocks.buffer)));

  cl(SetKernelArg(impl->kernels.copy, 8,SKC_CL_ARG(impl->ring.blocks_per.buffer)));
  cl(SetKernelArg(impl->kernels.copy, 9,SKC_CL_ARG(impl->prev.rolling)));

  cl(SetKernelArg(impl->kernels.copy,10,SKC_CL_ARG(impl->prev.from)));
  cl(SetKernelArg(impl->kernels.copy,11,SKC_CL_ARG(pb_prev_span)));
  cl(SetKernelArg(impl->kernels.copy,12,SKC_CL_ARG(impl->curr.from)));

  cl_event complete;

  skc_device_enqueue_kernel(impl->runtime->device,
                            SKC_DEVICE_KERNEL_ID_PATHS_COPY,
                            impl->cq,
                            impl->kernels.copy,
                            pb_cmds,
                            0,NULL,&complete);

  // set a callback on completion
  cl(SetEventCallback(complete,CL_COMPLETE,
                      skc_path_builder_paths_copy_cb,
                      grid));

  // immediately release
  cl(ReleaseEvent(complete));

  //
  // remap as many subbuffers as possible after the kernel completes
  //
  // note that remaps are async and enqueued on the same command queue
  // as the kernel launch
  //
  // we can't remap subbuffers that are in the possibly empty range
  //
  // cases:
  //
  //   - curr.to == wip.to which means no blocks have been acquired
  //   - curr.to points to first block in (next) subbuf
  //   - otherwise, wip acquired blocks in the curr.to subbuf
  //
  // check for these first 2 cases!
  //
  union skc_ringdex_expand const prev_from = skc_ringdex_expand(impl,impl->prev.from);
  skc_uint                 const no_wip    = impl->curr.to == impl->wip.to;
  skc_uint                       map_to    = curr_to.subbuf + (is_partial && no_wip);

  // remap all subbufs in range [from,to)
  skc_path_builder_impl_map(impl,prev_from.subbuf,map_to);

  // flush command queue
  cl(Flush(impl->cq));

  // save rolling
  impl->prev.rolling = impl->wip.rolling.next;

  // update prev and curr
  if (no_wip)
    {
      //
      // if there was no wip then round up to the next subbuf
      //
      skc_ringdex_wip_to_subbuf_inc(impl);

      //
      // update prev/curr with with incremented wip
      //
      impl->prev.from = impl->prev.to = impl->wip.to;
      impl->curr.from = impl->curr.to = impl->wip.to;
    }
  else
    {
      //
      // update prev with wip partials
      //
      impl->prev.from    = impl->curr.to;
      impl->prev.to      = impl->wip .to;

      //
      // start curr on a new subbuf boundary
      //
      skc_ringdex_wip_to_subbuf_inc(impl);

      impl->curr.from    = impl->wip.to;
      impl->curr.to      = impl->wip.to;
    }
}

//
//
//

static
void
skc_path_builder_impl_acquire_subbuffer(struct skc_path_builder_impl * const impl,
                                        skc_uint                       const subbuf)
{
  //
  // FIXME -- move to a power-of-two subbuf size and kickstart path
  // copies as early as possible
  //
  // FIXME -- the subbufs "self-clock" (flow control) the kernel
  // launches and accounting.  Combine all the subbuffers and release
  // records into a single indexable struct instead of 3.
  //
  struct skc_subbuffer_cmds * const sc        = impl->cmds.subbufs    + subbuf;
  struct skc_release_record * const release   = impl->release.records + subbuf;
  struct skc_scheduler      * const scheduler = impl->runtime->scheduler;

  // can't proceed until the paths have been released
  SKC_SCHEDULER_WAIT_WHILE(scheduler,release->from != release->to);

  // throw in a scheduler yield ... FIXME -- get rid of
  skc_scheduler_yield(scheduler);

  // can't proceed until the subbuffer is mapped
  cl(WaitForEvents(1,&sc->map));
}

//
//
//

static
union skc_ringdex_expand
skc_path_builder_impl_acquire_block(struct skc_path_builder_impl * const impl)
{
  // break ringdex into components
  union skc_ringdex_expand const to = skc_ringdex_expand(impl,impl->wip.to);

  // does wip ringdex point to a new subbuffer?
  if (to.block == 0)
    {
      // potentially spin/block waiting for subbuffer
      skc_path_builder_impl_acquire_subbuffer(impl,to.subbuf);
    }

  // post increment wip.to
  skc_ringdex_wip_to_block_inc(impl);

  return to;
}

//
//
//

static
skc_uint
skc_rolling_block(skc_uint const rolling, skc_uint const tag)
{
  return rolling | tag;
}

static
skc_uint
skc_rolling_subblock(skc_uint const rolling, skc_uint const subblock, skc_uint const tag)
{
  return rolling | (subblock << SKC_TAGGED_BLOCK_ID_BITS_TAG) | tag;
}

static
void
skc_rolling_inc(struct skc_path_builder_impl * const impl)
{
  impl->wip.rolling.next += impl->wip.rolling.one;
}

//
//
//

static
void *
skc_path_builder_impl_new_command(struct skc_path_builder_impl * const impl,
                                  skc_uint                       const rolling,
                                  skc_cmd_paths_copy_tag         const tag)
{
  // bump blocks count
  impl->wip.head->header.blocks += 1;

  // acquire a block
  union skc_ringdex_expand    const to          = skc_path_builder_impl_acquire_block(impl);

  // make a pointer
  union skc_tagged_block_id * const cmds_subbuf = impl->cmds.subbufs[to.subbuf].host;

  // store command for block
  cmds_subbuf[to.block].u32 = skc_rolling_block(rolling,tag);

#if 0
  // store command for block
  cmds_subbuf[to.block].u32 = skc_rolling_block(impl->wip.rolling.next,tag);

  // increment rolling
  skc_rolling_inc(impl);
#endif

  // return pointer to block
  float * const blocks_subbuf = impl->blocks.subbufs[to.subbuf].host;

  // FIXME -- make it easier to get config constant
  return blocks_subbuf + (to.block * impl->runtime->config->block.words);
}

//
//
//

static
void
skc_path_builder_impl_flush_node(struct skc_path_builder_impl * const impl)
{
  // store command to subbuf and get pointer to blocks subbuf
  void * const block = skc_path_builder_impl_new_command(impl,impl->wip.ids.rolling,
                                                         SKC_CMD_PATHS_COPY_TAG_NODE);

  // copy head to blocks subbuf -- write-only
  memcpy(block,impl->wip.node,impl->runtime->config->block.bytes);
}

static
void
skc_path_builder_impl_flush_head(struct skc_path_builder_impl * const impl)
{
  // store command to subbuf and get pointer to blocks subbuf
  void * const block = skc_path_builder_impl_new_command(impl,impl->wip.rolling.next,
                                                         SKC_CMD_PATHS_COPY_TAG_HEAD);

  // copy head to blocks subbuf -- write-only
  memcpy(block,impl->wip.head,impl->runtime->config->block.bytes);

  // increment rolling
  skc_rolling_inc(impl);

  // the 'to' index is non-inclusive so assign wip.to after flush_head
  impl->curr.to = impl->wip.to;
}

//
//
//

static
void
skc_path_builder_impl_new_node_block(struct skc_path_builder_impl * const impl)
{
  // update final block id in node
  impl->wip.ids.next->u32 = skc_rolling_block(impl->wip.rolling.next,SKC_BLOCK_ID_TAG_PATH_NEXT);

  // if wip.ids is not the header then flush now full wip node
  if (impl->wip.head->header.nodes > 0)
    skc_path_builder_impl_flush_node(impl);

  // bump node count
  impl->wip.head->header.nodes += 1;

  // save current rolling
  impl->wip.ids.rolling = impl->wip.rolling.next;

  // increment rolling
  skc_rolling_inc(impl);

  // update wip.ids.*
  impl->wip.ids.next = impl->wip.node->tag_ids;
  impl->wip.ids.rem  = impl->runtime->config->block.words;
}

static
void
skc_path_builder_impl_new_segs_block(struct skc_path_builder_impl * const impl)
{
  impl->wip.subblocks.rem     = impl->runtime->config->block.subblocks; // FIXME -- move constants closer to structure
  impl->wip.subblocks.rolling = impl->wip.rolling.next;
  impl->wip.subblocks.next    = skc_path_builder_impl_new_command(impl,impl->wip.rolling.next,
                                                                  SKC_CMD_PATHS_COPY_TAG_SEGS);
  impl->wip.subblocks.idx     = 0;

  // increment rolling
  skc_rolling_inc(impl);
}

//
//
//

static
void
skc_path_builder_impl_acquire_subblocks(struct skc_path_builder_impl * const impl,
                                        skc_block_id_tag                     tag,
                                        skc_uint                             vertices,
                                        float * *                            subblocks)
{
  //
  // FIRST TAG RECORDS THE ELEMENT TYPE
  //
  while (true)
    {
      // if only one block id left in node then acquire new node block
      // and append its block id as with a next tag
      if (impl->wip.ids.rem == 1)
        skc_path_builder_impl_new_node_block(impl);

      // if zero subblocks left then acquire a new subblock block and
      // append its block id
      if (impl->wip.subblocks.rem == 0)
        skc_path_builder_impl_new_segs_block(impl);

      // save first command -- tag and subblocks may have been updated
      impl->wip.ids.next->u32 = skc_rolling_subblock(impl->wip.subblocks.rolling,impl->wip.subblocks.idx,tag);

      // increment node block subblock pointer
      impl->wip.ids.next += 1;
      impl->wip.ids.rem  -= 1;

      // how many vertices can we store
      skc_uint rem = min(vertices,impl->wip.subblocks.rem);

      // decrement vertices
      vertices                -= rem;
      impl->wip.subblocks.rem -= rem;
      impl->wip.subblocks.idx += rem;

      // assign subblocks
      do {
        *subblocks++              = impl->wip.subblocks.next;
        impl->wip.subblocks.next += impl->runtime->config->subblock.words;
        // FIXME -- move constants closer to structure
      } while (--rem > 0);

      // anything left to do?
      if (vertices == 0)
        break;

      // any tag after this will be a caboose command
      tag = SKC_BLOCK_ID_TAG_PATH_NEXT;
    }
}

//
//
//

static
void
skc_path_builder_pfn_end(struct skc_path_builder_impl * const impl, skc_path_t * const path)
{
  // finalize incomplete active subblocks -- we don't care about any
  // remaining unused subblocks in block
  skc_path_builder_finalize_subblocks(impl->path_builder);

  // mark remaining wips.ids in the head or node as invalid
  skc_path_builder_impl_finalize_node(impl);

  // flush node if rem > 0 and node is not actually head
  if (impl->wip.head->header.nodes >= 1)
    skc_path_builder_impl_flush_node(impl);

  // acquire path host id
  *path = skc_runtime_handle_device_acquire(impl->runtime); // FIXME -- MAY WANT TO GRAB AN ID ON BEGIN

  // save path host handle
  impl->wip.head->header.handle = *path;

  // flush head -- acquires a block and bumps head->header.blocks
  skc_path_builder_impl_flush_head(impl);

  // get current release
  struct skc_release_record * const release = skc_release_curr(impl);

  // acquire grid if null
  if (release->grid == NULL)
    {
      release->grid =
        SKC_GRID_DEPS_ATTACH(impl->runtime->deps,
                             &release->grid, // NULL on start/force
                             release,        // data payload
                             skc_path_builder_grid_pfn_waiting,
                             NULL,           // no execute pfn
                             skc_path_builder_grid_pfn_dispose);
    }

  // update grid map
  skc_grid_map(release->grid,*path);

  // update path release
  impl->release.paths[release->to] = *path;

  // increment release.to
  release->to = (release->to + 1) % impl->ring.blocks_per.buffer;

  // add guard bit
  *path |= SKC_TYPED_HANDLE_TYPE_IS_PATH;

#if 1
  //
  // eager kernel launch?
  //
  {
    union skc_ringdex_expand const curr_from = skc_ringdex_expand(impl,impl->curr.from);
    union skc_ringdex_expand const curr_to   = skc_ringdex_expand(impl,impl->curr.to);

    if (curr_from.subbuf != curr_to.subbuf)
      {
        skc_grid_start(release->grid);
        // skc_scheduler_yield(impl->runtime->scheduler);
      }
  }
#endif
}

//
// FIXME -- clean up accessing of CONFIG constants in these 3 routines
//

static
void
skc_path_builder_pfn_new_line(struct skc_path_builder_impl * const impl)
{
  // acquire subblock pointers
  skc_path_builder_impl_acquire_subblocks(impl,SKC_BLOCK_ID_TAG_PATH_LINE,4,
                                          impl->path_builder->line.coords);

  // increment line count
  impl->wip.head->header.prims += 1;

  // update rem_count_xxx count
  impl->path_builder->line.rem = impl->runtime->config->subblock.words;
}

static
void
skc_path_builder_pfn_new_quad(struct skc_path_builder_impl * const impl)
{
  // acquire subblock pointers
  skc_path_builder_impl_acquire_subblocks(impl,SKC_BLOCK_ID_TAG_PATH_QUAD,6,
                                          impl->path_builder->quad.coords);

  // increment line count
  impl->wip.head->header.prims += 1;

  // update rem_count_xxx count
  impl->path_builder->quad.rem = impl->runtime->config->subblock.words;
}

static
void
skc_path_builder_pfn_new_cubic(struct skc_path_builder_impl * const impl)
{
  // acquire subblock pointers
  skc_path_builder_impl_acquire_subblocks(impl,SKC_BLOCK_ID_TAG_PATH_CUBIC,8,
                                          impl->path_builder->cubic.coords);

  // increment line count
  impl->wip.head->header.prims += 1;

  // update rem_count_xxx count
  impl->path_builder->cubic.rem = impl->runtime->config->subblock.words;
}

//
//
//

static
void
skc_path_builder_pfn_release(struct skc_path_builder_impl * const impl)
{
  // decrement reference count
  if (--impl->path_builder->refcount != 0)
    return;

  //
  // otherwise, dispose of everything
  //
  struct skc_runtime * const runtime = impl->runtime;

  // free path builder
  skc_runtime_host_perm_free(impl->runtime,impl->path_builder);

  // release cq
  skc_runtime_release_cq_in_order(runtime,impl->cq);

  // release kernels
  cl(ReleaseKernel(impl->kernels.alloc));
  cl(ReleaseKernel(impl->kernels.copy));

  // free blocks extents
  cl(ReleaseMemObject(impl->blocks.buffer));
  skc_runtime_host_perm_free(runtime,impl->blocks.subbufs);

  cl(ReleaseMemObject(impl->cmds.buffer));
  skc_runtime_host_perm_free(runtime,impl->cmds.subbufs);

  // free records
  skc_runtime_host_perm_free(runtime,impl->release.records);
  skc_runtime_host_perm_free(runtime,impl->release.paths);

  // release staging head and node
  skc_runtime_host_perm_free(runtime,impl->wip.head);
  skc_runtime_host_perm_free(runtime,impl->wip.node);

  // release reads scratch array
  cl(ReleaseMemObject(impl->reads));

  // for all subbuffers
  //   unmap   subbuffer
  //   release subbuffer
  // printf("%s not releasing subbuffers\n",__func__);

  skc_runtime_host_perm_free(impl->runtime,impl);
}

//
//
//

skc_err
skc_path_builder_cl_12_create(struct skc_context        * const context,
                              struct skc_path_builder * * const path_builder)
{
  //
  // retain the context
  // skc_context_retain(context);
  //
  struct skc_runtime * const runtime = context->runtime;

  // allocate path builder
  (*path_builder)             = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(**path_builder));

  // init state
  SKC_ASSERT_STATE_INIT((*path_builder),SKC_PATH_BUILDER_STATE_READY);

  (*path_builder)->context    = context;

  // save opaque impl-specific pointers
  (*path_builder)->begin      = skc_path_builder_pfn_begin;
  (*path_builder)->end        = skc_path_builder_pfn_end;
  (*path_builder)->new_line   = skc_path_builder_pfn_new_line;
  (*path_builder)->new_quad   = skc_path_builder_pfn_new_quad;
  (*path_builder)->new_cubic  = skc_path_builder_pfn_new_cubic;
  (*path_builder)->release    = skc_path_builder_pfn_release;

  // initialize path builder counts
  (*path_builder)->line.rem   = 0;
  (*path_builder)->quad.rem   = 0;
  (*path_builder)->cubic.rem  = 0;

  (*path_builder)->refcount   = 1;

  struct skc_path_builder_impl * const impl = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*impl));

  (*path_builder)->impl       = impl;

  //
  // init impl
  //
  impl->path_builder  = *path_builder;
  impl->runtime       = runtime;

  impl->cq            = skc_runtime_acquire_cq_in_order(runtime);

  impl->kernels.alloc = skc_device_acquire_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_PATHS_ALLOC);
  impl->kernels.copy  = skc_device_acquire_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_PATHS_COPY);

  //
  // FIXME -- let these config constants remain constant and in place
  //
  struct skc_config const * const config = runtime->config;

  impl->ring.subbufs           = config->paths_copy.buffer.count;
  impl->ring.blocks_per.buffer = config->paths_copy.subbuf.count * config->paths_copy.buffer.count;
  impl->ring.blocks_per.subbuf = config->paths_copy.subbuf.count;
  //
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //

  cl_int cl_err;

  // allocate large device-side extent for path data
  impl->blocks.buffer   = clCreateBuffer(runtime->cl.context,
                                         CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                         config->paths_copy.block.buffer, // FIXME -- either use config or local constants everywhere
                                         NULL,&cl_err); cl_ok(cl_err);

  // allocate small host-side array of pointers to mapped subbufs
  impl->blocks.subbufs  = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,
                                                      impl->ring.subbufs *
                                                      sizeof(*impl->blocks.subbufs));

  // allocate large device-side extent for path copy commands
  impl->cmds.buffer     = clCreateBuffer(runtime->cl.context,
                                         CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                         config->paths_copy.command.buffer,
                                         NULL,&cl_err); cl_ok(cl_err);

  // allocate small host-side array of pointers to mapped subbufs
  impl->cmds.subbufs    = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,
                                                      impl->ring.subbufs *
                                                      sizeof(*impl->cmds.subbufs));

  // allocate small host-side array of intervals of path handles
  impl->release.records = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,
                                                      impl->ring.subbufs *
                                                      sizeof(*impl->release.records));

  // allocate large host-side array that is max # of path handles in flight
  impl->release.paths   = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,
                                                      impl->ring.blocks_per.buffer *
                                                      sizeof(*impl->release.paths));

  // small scratch used by kernels
  impl->reads           = clCreateBuffer(runtime->cl.context,
                                         CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                                         sizeof(skc_uint) * impl->ring.subbufs,
                                         NULL,&cl_err); cl_ok(cl_err);

  // initialize release record with impl backpointer
  for (skc_uint ii=0; ii<impl->ring.subbufs; ii++)
    {
      struct skc_release_record * record = impl->release.records + ii;

      record->impl = impl;
      record->grid = NULL;
      record->from = record->to = ii * impl->ring.blocks_per.subbuf;
    }

  //
  // allocate and map subbuffers -- we always check the command
  // subbuffer's map/unmap events before touching it or its associated
  // block subbuffer.
  //
  struct skc_subbuffer_blocks * sb = impl->blocks.subbufs;
  struct skc_subbuffer_cmds   * sc = impl->cmds  .subbufs;

  cl_buffer_region              rb = { 0, config->paths_copy.block.subbuf   };
  cl_buffer_region              rc = { 0, config->paths_copy.command.subbuf };

  // for each subbuffer
  for (skc_uint ii=0; ii<config->paths_copy.buffer.count; ii++)
    {
      sb->device = clCreateSubBuffer(impl->blocks.buffer,
                                     CL_MEM_HOST_WRITE_ONLY,
                                     CL_BUFFER_CREATE_TYPE_REGION,
                                     &rb,
                                     &cl_err); cl_ok(cl_err);

      sb->host   = clEnqueueMapBuffer(impl->cq,
                                      sb->device,
                                      CL_FALSE,
                                      CL_MAP_WRITE_INVALIDATE_REGION,
                                      0,rb.size,
                                      0,NULL,NULL,
                                      &cl_err); cl_ok(cl_err);

      sc->device = clCreateSubBuffer(impl->cmds.buffer,
                                     CL_MEM_HOST_WRITE_ONLY,
                                     CL_BUFFER_CREATE_TYPE_REGION,
                                     &rc,
                                     &cl_err); cl_ok(cl_err);

      sc->host   = clEnqueueMapBuffer(impl->cq,
                                      sc->device,
                                      CL_FALSE,
                                      CL_MAP_WRITE_INVALIDATE_REGION,
                                      0,rc.size,
                                      0,NULL,&sc->map,
                                      &cl_err); cl_ok(cl_err);
      sb        += 1;
      sc        += 1;

      rb.origin += rb.size;
      rc.origin += rc.size;
    }

  //
  // initialize remaining members
  //
  impl->prev.from        = 0;
  impl->prev.to          = 0;
  impl->prev.rolling     = 0;

  impl->curr.from        = 0;
  impl->curr.to          = 0;

  impl->wip.to           = 0;

  impl->wip.head         = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,config->block.bytes);
  impl->wip.node         = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,config->block.bytes);

  impl->wip.rolling.one  = SKC_BLOCK_ID_TAG_COUNT * config->block.subblocks;
  impl->wip.rolling.next = 0;

  // for now, completely initialize builder before returning
  cl(Finish(impl->cq));

  return SKC_ERR_SUCCESS;
}

//
//
//

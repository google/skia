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

#include <stdlib.h>
#include <memory.h>
#include <float.h>

#include "raster_builder.h"

#include "context.h"
#include "weakref.h"

#include "scheduler.h"
#include "handle.h"

#include "common.h"

//
//
//

#ifndef NDEBUG

#include <stdio.h>

#define SKC_CONTEXT_WAIT_DEBUG(p)               \
  fprintf(stderr,"WAITING ON: " p "\n")

#else

#define SKC_CONTEXT_WAIT_DEBUG(p)

#endif

//
//
//

#define SKC_CONTEXT_WAIT_WHILE(c,p)             \
  while (p) {                                   \
    SKC_CONTEXT_WAIT_DEBUG(#p);                 \
    skc_context_wait(c);                        \
  }

//
//
//

#if 0

//
// IDENTITY TRANSFORM
//

static
float const skc_transform_identity[8] =
  {
    1.0f, 0.0f, 0.0f,  // sx  shx tx
    0.0f, 1.0f, 0.0f,  // shy sy  ty
    0.0f, 0.0f         // w0  w1  1  <-- always 1
  };

// float const * const skc_transform_identity_ptr = skc_transform_identity;

//
// DEFAULT RASTER CLIP
//

static
float const skc_raster_clip_default[4] =
  {
    -FLT_MAX, -FLT_MAX, // lower left  corner of bounding box
    +FLT_MAX, +FLT_MAX  // upper right corner of bounding box
  };

// float const * const skc_raster_clip_default_ptr = skc_raster_clip_default;

#endif

//
//
//

skc_err
skc_raster_builder_retain(skc_raster_builder_t raster_builder)
{
  raster_builder->refcount += 1;

  return SKC_ERR_SUCCESS;
}

//xbli
//
//

skc_err
skc_raster_builder_release(skc_raster_builder_t raster_builder)
{
  SKC_ASSERT_STATE_ASSERT(SKC_RASTER_BUILDER_STATE_READY,raster_builder);

  raster_builder->release(raster_builder->impl);

  return SKC_ERR_SUCCESS;
}

//
//
//

static
skc_bool
skc_raster_builder_path_ids_append(struct skc_raster_builder * const raster_builder,
                                   union skc_cmd_fill        * const cmd,
                                   skc_path_t                  const path)
{
  SKC_CONTEXT_WAIT_WHILE(raster_builder->context,skc_extent_ring_is_full(&raster_builder->path_ids.ring));

  cmd->path = path;

  raster_builder->path_ids.extent[skc_extent_ring_wip_index_inc(&raster_builder->path_ids.ring)] = path;

  return skc_extent_ring_wip_is_full(&raster_builder->path_ids.ring);
}

static
skc_bool
skc_raster_builder_transforms_append(struct skc_raster_builder * const raster_builder,
                                     union skc_cmd_fill        * const cmd,
                                     skc_transform_weakref_t   * const transform_weakref,
                                     skc_float           const * const transform)
{
  //
  // FIXME -- check weakref
  //
  SKC_CONTEXT_WAIT_WHILE(raster_builder->context,skc_extent_ring_is_full(&raster_builder->transforms.ring));

  cmd->transform = skc_extent_ring_wip_count(&raster_builder->transforms.ring);

  skc_uint const base = skc_extent_ring_wip_index_inc(&raster_builder->transforms.ring);

  memcpy(raster_builder->transforms.extent[base].f32a8,transform,sizeof(skc_float8));

  return skc_extent_ring_wip_is_full(&raster_builder->transforms.ring);
}

static
skc_bool
skc_raster_builder_clips_append(struct skc_raster_builder * const raster_builder,
                                union skc_cmd_fill        * const cmd,
                                skc_raster_clip_weakref_t * const raster_clip_weakref,
                                skc_float           const * const raster_clip)
{
  //
  // FIXME -- check weakref
  //
  SKC_CONTEXT_WAIT_WHILE(raster_builder->context,skc_extent_ring_is_full(&raster_builder->clips.ring));

  cmd->clip = skc_extent_ring_wip_count(&raster_builder->clips.ring);

  skc_uint const base = skc_extent_ring_wip_index_inc(&raster_builder->clips.ring);

  memcpy(raster_builder->clips.extent[base].f32a4,raster_clip,sizeof(skc_float4));

  return skc_extent_ring_wip_is_full(&raster_builder->clips.ring);
}

static
skc_bool
skc_raster_builder_cmds_append(struct skc_raster_builder * const raster_builder,
                               union skc_cmd_fill        * const cmd)
{
  SKC_CONTEXT_WAIT_WHILE(raster_builder->context,skc_extent_ring_is_full(&raster_builder->fill_cmds.ring));

  cmd->cohort = skc_extent_ring_wip_count(&raster_builder->raster_ids.ring);

  skc_uint const base = skc_extent_ring_wip_index_inc(&raster_builder->fill_cmds.ring);

  raster_builder->fill_cmds.extent[base] = *cmd;

#if 0
  fprintf(stderr,"[ %4u, %4u, %4u, %4u ]\n",
          cmd->path,
          cmd->transform,
          cmd->clip,
          cmd->cohort);
#endif

  return skc_extent_ring_wip_is_full(&raster_builder->fill_cmds.ring);
}

//
//
//

static
skc_bool
skc_raster_builder_raster_ids_append(struct skc_raster_builder * const raster_builder,
                                     skc_raster_t                const raster)
{
  SKC_CONTEXT_WAIT_WHILE(raster_builder->context,skc_extent_ring_is_full(&raster_builder->raster_ids.ring));

  raster_builder->raster_ids.extent[skc_extent_ring_wip_index_inc(&raster_builder->raster_ids.ring)] = raster;

  return skc_extent_ring_wip_is_full(&raster_builder->raster_ids.ring);
}

//
//
//

static
void
skc_raster_builder_checkpoint(struct skc_raster_builder * const raster_builder)
{
  skc_extent_ring_checkpoint(&raster_builder->path_ids  .ring);
  skc_extent_ring_checkpoint(&raster_builder->transforms.ring);
  skc_extent_ring_checkpoint(&raster_builder->clips     .ring);
  skc_extent_ring_checkpoint(&raster_builder->fill_cmds .ring);
  skc_extent_ring_checkpoint(&raster_builder->raster_ids.ring);
}

//
// RASTER OPS
//

skc_err
skc_raster_begin(skc_raster_builder_t raster_builder)
{
  SKC_ASSERT_STATE_TRANSITION(SKC_RASTER_BUILDER_STATE_READY,
                              SKC_RASTER_BUILDER_STATE_BUILDING,
                              raster_builder);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_raster_end(skc_raster_builder_t raster_builder, skc_raster_t * raster)
{
  SKC_ASSERT_STATE_TRANSITION(SKC_RASTER_BUILDER_STATE_BUILDING,
                              SKC_RASTER_BUILDER_STATE_READY,
                              raster_builder);
  // get a raster id
  raster_builder->end(raster_builder->impl,raster);

  // if cohort is full then launch
  skc_bool const snap = skc_raster_builder_raster_ids_append(raster_builder,*raster);

  // checkpoint the current ring range
  skc_raster_builder_checkpoint(raster_builder);

  // snapshot and force start because the cohort is full -- no need to wait
  if (snap)
    raster_builder->force(raster_builder->impl);

  // add guard bit
  *raster |= SKC_TYPED_HANDLE_TYPE_IS_RASTER; // FIXME -- the guard bit can be buried

  return SKC_ERR_SUCCESS;
}

//
// PATH-TO-RASTER OPS
//

skc_err
skc_raster_add_filled(skc_raster_builder_t        raster_builder,
                      skc_path_t                  path,
                      skc_transform_weakref_t   * transform_weakref,
                      float               const * transform,
                      skc_raster_clip_weakref_t * raster_clip_weakref,
                      float               const * raster_clip)
{
  SKC_ASSERT_STATE_ASSERT(SKC_RASTER_BUILDER_STATE_BUILDING,raster_builder);

  //
  // validate and retain the path handle before proceeding
  //
  skc_err err = raster_builder->add(raster_builder->impl,&path,1);

  if (err)
    return err;

  // mask off the guard bits
  path = SKC_TYPED_HANDLE_TO_HANDLE(path);

  //
  // build the command...
  //
  union skc_cmd_fill cmd;

  // append path to ring
  skc_bool snap = skc_raster_builder_path_ids_append(raster_builder,&cmd,path);

  // append transform
  snap = skc_raster_builder_transforms_append(raster_builder,&cmd,transform_weakref,transform) || snap;

  // append raster clip
  snap = skc_raster_builder_clips_append(raster_builder,&cmd,raster_clip_weakref,raster_clip) || snap;

  // append fill command
  snap = skc_raster_builder_cmds_append(raster_builder,&cmd) || snap;

  // snapshot and lazily start
  if (snap)
    raster_builder->start(raster_builder->impl);

  return SKC_ERR_SUCCESS;
}

//
//
//

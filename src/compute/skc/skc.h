/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SKC
#define SKC_ONCE_SKC

//
//
//

#include "skc_err.h"
#include "skc_types.h"
#include "skc_styling.h"

//
// CONTEXT
//

skc_err
skc_context_retain(skc_context_t context);

skc_err
skc_context_release(skc_context_t context);

skc_err
skc_context_reset(skc_context_t context);

//
// PATH BUILDER
//

skc_err
skc_path_builder_create(skc_context_t context, skc_path_builder_t * path_builder);

skc_err
skc_path_builder_retain(skc_path_builder_t path_builder);

skc_err
skc_path_builder_release(skc_path_builder_t path_builder);

//
// PATH OPS
//

skc_err
skc_path_begin(skc_path_builder_t path_builder);

skc_err
skc_path_end(skc_path_builder_t path_builder, skc_path_t * path);

skc_err
skc_path_retain(skc_context_t context, skc_path_t const * paths, uint32_t count);

skc_err
skc_path_release(skc_context_t context, skc_path_t const * paths, uint32_t count);

skc_err
skc_path_flush(skc_context_t context, skc_path_t const * paths, uint32_t count);

//
// PATH SEGMENT OPS
//

//
// FIXME -- we need a bulk/vectorized path segment operation
//

skc_err
skc_path_move_to(skc_path_builder_t path_builder,
                 float x0, float y0);

skc_err
skc_path_close(skc_path_builder_t path_builder);

skc_err
skc_path_line_to(skc_path_builder_t path_builder,
                 float x1, float y1);

skc_err
skc_path_cubic_to(skc_path_builder_t path_builder,
                  float x1, float y1,
                  float x2, float y2,
                  float x3, float y3);

skc_err
skc_path_cubic_smooth_to(skc_path_builder_t path_builder,
                         float x2, float y2,
                         float x3, float y3);

skc_err
skc_path_quad_to(skc_path_builder_t path_builder,
                 float x1, float y1,
                 float x2, float y2);

skc_err
skc_path_quad_smooth_to(skc_path_builder_t path_builder,
                        float x2, float y2);

skc_err
skc_path_ellipse(skc_path_builder_t path_builder,
                 float cx, float cy,
                 float rx, float ry);

//
// RASTER BUILDER
//

skc_err
skc_raster_builder_create(skc_context_t context, skc_raster_builder_t * raster_builder);

skc_err
skc_raster_builder_retain(skc_raster_builder_t raster_builder);

skc_err
skc_raster_builder_release(skc_raster_builder_t raster_builder);

//
// RASTER OPS
//

skc_err
skc_raster_begin(skc_raster_builder_t raster_builder);

skc_err
skc_raster_end(skc_raster_builder_t raster_builder, skc_raster_t * raster);

skc_err
skc_raster_retain(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

skc_err
skc_raster_release(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

skc_err
skc_raster_flush(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

//
// PATH-TO-RASTER OPS
//

//
// FIXME -- do we need a bulk/vectorized "add filled" function?
//

skc_err
skc_raster_add_filled(skc_raster_builder_t        raster_builder,
                      skc_path_t                  path,
                      skc_transform_weakref_t   * transform_weakref,
                      float               const * transform,
                      skc_raster_clip_weakref_t * raster_clip_weakref,
                      float               const * raster_clip);

//
// COMPOSITION STATE
//

skc_err
skc_composition_create(skc_context_t context, skc_composition_t * composition);

skc_err
skc_composition_retain(skc_composition_t composition);

skc_err
skc_composition_release(skc_composition_t composition);

skc_err
skc_composition_place(skc_composition_t    composition,
                      skc_raster_t const * rasters,
                      skc_layer_id const * layer_ids,
                      float        const * txs,
                      float        const * tys,
                      uint32_t             count); // NOTE: A PER-PLACE CLIP IS POSSIBLE

skc_err
skc_composition_seal(skc_composition_t composition);

skc_err
skc_composition_unseal(skc_composition_t composition, bool reset);

skc_err
skc_composition_get_bounds(skc_composition_t composition, int32_t bounds[4]);

#if 0
// let's switch to a per place bounds using weakrefs -- clip 0 will be largest clip
skc_err
skc_composition_set_clip(skc_composition_t composition, int32_t const clip[4]);
#endif

//
// TODO: COMPOSITION "SET ALGEBRA" OPERATIONS
//
// Produce a new composition from the union or intersection of two
// existing compositions
//

//
// TODO: COMPOSITION "HIT DETECTION"
//
// Report which layers and tiles are intersected by one or more
// device-space (x,y) points
//

//
// STYLING STATE
//

skc_err
skc_styling_create(skc_context_t   context,
                   skc_styling_t * styling,
                   uint32_t        layers_count,
                   uint32_t        groups_count,
                   uint32_t        extras_count);

skc_err
skc_styling_retain(skc_styling_t styling);

skc_err
skc_styling_release(skc_styling_t styling);

skc_err
skc_styling_seal(skc_styling_t styling);

skc_err
skc_styling_unseal(skc_styling_t styling); // FIXME

skc_err
skc_styling_reset(skc_styling_t styling); // FIXME -- make unseal reset

//
// STYLING GROUPS AND LAYERS
//

skc_err
skc_styling_group_alloc(skc_styling_t  styling,
                        skc_group_id * group_id);

skc_err
skc_styling_group_enter(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);

skc_err
skc_styling_group_leave(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);

skc_err
skc_styling_group_parents(skc_styling_t        styling,
                          skc_group_id         group_id,
                          uint32_t             depth,
                          skc_group_id const * parents);

skc_err
skc_styling_group_range_lo(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_lo);

skc_err
skc_styling_group_range_hi(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_hi);

skc_err
skc_styling_group_layer(skc_styling_t             styling,
                        skc_group_id              group_id,
                        skc_layer_id              layer_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);

//
// STYLING ENCODERS -- FIXME -- WILL EVENTUALLY BE OPAQUE
//

void
skc_styling_layer_fill_rgba_encoder(skc_styling_cmd_t * cmds, float const rgba[4]);

void
skc_styling_background_over_encoder(skc_styling_cmd_t * cmds, float const rgba[4]);

void
skc_styling_layer_fill_gradient_encoder(skc_styling_cmd_t         * cmds,
                                        float                       x0,
                                        float                       y0,
                                        float                       x1,
                                        float                       y1,
                                        skc_styling_gradient_type_e type,
                                        uint32_t                    n,
                                        float                 const stops[],
                                        float                 const colors[]);

//
// SURFACE
//

skc_err
skc_surface_create(skc_context_t context, skc_surface_t * surface);

skc_err
skc_surface_retain(skc_surface_t surface);

skc_err
skc_surface_release(skc_surface_t surface);

//
// SURFACE RENDER
//

typedef void (*skc_surface_render_notify)(skc_surface_t     surface,
                                          skc_styling_t     styling,
                                          skc_composition_t composition,
                                          skc_framebuffer_t fb,
                                          void            * data);

skc_err
skc_surface_render(skc_surface_t             surface,
                   skc_styling_t             styling,
                   skc_composition_t         composition,
                   skc_framebuffer_t         fb,
                   uint32_t            const clip[4],
                   int32_t             const txty[2],
                   skc_surface_render_notify notify,
                   void                    * data);

//
// COORDINATED EXTERNAL OPERATIONS
//
//  Examples include:
//
//  - Transforming an intermediate layer with a blur, sharpen, rotation or scaling kernel.
//  - Subpixel antialiasing using neighboring pixel color and coverage data.
//  - Performing a blit from one region to another region on a surface.
//  - Blitting from one surface to another.
//  - Loading and processing from one region and storing to another region.
//  - Rendezvousing with an external pipeline.
//

// FORTHCOMING...

//
// SCHEDULER
//

bool
skc_context_yield(skc_context_t context);

void
skc_context_wait(skc_context_t context);

//
//
//

#endif

//
//
//

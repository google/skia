/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"
#include "types.h"

//
// FIXME -- relax the const correctness
//

struct skc_context
{
  struct skc_runtime   * runtime;

  //
  //
  //

  bool                (* yield          )(struct skc_runtime * const runtime);

  void                (* wait           )(struct skc_runtime * const runtime);

  //
  //
  //

  skc_err             (* path_builder   )(struct skc_context * const context,
                                          skc_path_builder_t * const path_builder);

  skc_err             (* path_retain    )(struct skc_runtime * const runtime,
                                          skc_path_t   const *       paths,
                                          uint32_t                   count);

  skc_err             (* path_release   )(struct skc_runtime * const runtime,
                                          skc_path_t   const *       paths,
                                          uint32_t                   count);

  skc_err             (* path_flush     )(struct skc_runtime * const runtime,
                                          skc_path_t   const *       paths,
                                          uint32_t                   count);

  //
  //
  //

  skc_err             (* raster_builder )(struct skc_context   * const context,
                                          skc_raster_builder_t * const raster_builder);

  skc_err             (* raster_retain  )(struct skc_runtime * const runtime,
                                          skc_raster_t const *       rasters,
                                          uint32_t                   count);

  skc_err             (* raster_release )(struct skc_runtime * const runtime,
                                          skc_raster_t const *       rasters,
                                          uint32_t                   count);

  skc_err             (* raster_flush   )(struct skc_runtime * const runtime,
                                          skc_raster_t const *       rasters,
                                          uint32_t                   count);
  //
  //
  //

  skc_err             (* composition    )(struct skc_context * const context,
                                          skc_composition_t  * const composition);

  //
  //
  //

  skc_err             (* styling        )(struct skc_context * const context,
                                          skc_styling_t      * const styling,
                                          uint32_t             const layers_count,
                                          uint32_t             const groups_count,
                                          uint32_t             const extras_count);

  //
  //
  //

  skc_err             (* surface        )(struct skc_context * const context,
                                          skc_surface_t      * const surface);
  //
  //
  //

  skc_int                refcount;
};

//
//
//

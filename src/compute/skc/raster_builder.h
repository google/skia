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
#include "assert_state.h"
#include "extent_ring.h" // note that these structs are *not* opaque

//
//
//

typedef enum skc_raster_builder_state_e {

  SKC_RASTER_BUILDER_STATE_READY,
  SKC_RASTER_BUILDER_STATE_BUILDING

} skc_raster_builder_state_e;

//
// Construct and dispose of a raster builder and its opaque
// implementation.
//

struct skc_raster_builder
{
  struct skc_context              * context;

  struct skc_raster_builder_impl  * impl;

  skc_err                        (* add    )(struct skc_raster_builder_impl * const impl, skc_path_t const * paths, skc_uint count);
  void                           (* end    )(struct skc_raster_builder_impl * const impl, skc_raster_t * const raster);
  void                           (* start  )(struct skc_raster_builder_impl * const impl);
  void                           (* force  )(struct skc_raster_builder_impl * const impl);
  void                           (* release)(struct skc_raster_builder_impl * const impl);

  struct {
    skc_path_t                    * extent;
    struct skc_extent_ring          ring;
  } path_ids;

  struct {
    union skc_transform           * extent;
    struct skc_extent_ring          ring;
  } transforms;

  struct {
    union skc_path_clip           * extent;
    struct skc_extent_ring          ring;
  } clips;

  struct {
    union skc_cmd_fill            * extent;
    struct skc_extent_ring          ring;
  } fill_cmds;

  struct {
    skc_raster_t                  * extent;
    struct skc_extent_ring          ring;
  } raster_ids;

  skc_uint                          refcount; // FIXME -- split this into host and impl refcounts

  SKC_ASSERT_STATE_DECLARE(skc_raster_builder_state_e);
};

//
//
//

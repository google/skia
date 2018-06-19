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
//
//

struct skc_composition
{
  struct skc_context           * context;
  struct skc_composition_impl  * impl;

  //
  // FIXME -- this collection of pfn's isn't complete
  //
  skc_err                     (* place  )(struct skc_composition_impl * const impl,
                                          skc_raster_t          const *       rasters,
                                          skc_layer_id          const *       layer_ids,
                                          skc_float             const *       txs,
                                          skc_float             const *       tys,
                                          skc_uint                            count);

  void                        (* unseal )(struct skc_composition_impl * const impl, skc_bool const reset);
  void                        (* seal   )(struct skc_composition_impl * const impl);
  void                        (* bounds )(struct skc_composition_impl * const impl, skc_int bounds[4]);
  void                        (* release)(struct skc_composition_impl * const impl);

  skc_int                        ref_count;
};

//
//
//

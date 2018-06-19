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

//
// SURFACE
//

struct skc_surface
{
  struct skc_context      * context;
  struct skc_surface_impl * impl;

  skc_int                   ref_count;

  //
  // FIXME -- this list of pfn's isn't complete
  //
  void                   (* release)(struct skc_surface_impl * const impl);
  void                   (* render )(struct skc_surface_impl * const impl,
                                     uint32_t                  const clip[4],
                                     skc_styling_t                   styling,
                                     skc_composition_t               composition,
                                     skc_surface_render_pfn_notify   notify,
                                     void                          * data,
                                     void                          * fb);
  //
  // FIXME -- these will probably be removed
  //
  void                   (* clear  )(struct skc_surface_impl * const impl,
                                     float                     const rgba[4], 
                                     skc_uint                  const rect[4],
                                     void                    *       fb);

  void                   (* blit   )(struct skc_surface_impl * const impl,
                                     skc_uint                  const rect[4], 
                                     skc_int                   const txty[2]);

};

//
//
//

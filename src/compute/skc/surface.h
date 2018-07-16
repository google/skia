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

  void (* release)(struct skc_surface_impl * const impl);

  void (* render )(struct skc_surface_impl * const impl,
                   skc_styling_t                   styling,
                   skc_composition_t               composition,
                   skc_framebuffer_t               fb,
                   uint32_t                  const clip[4],
                   int32_t                   const txty[2],
                   skc_surface_render_notify       notify,
                   void                          * data);
};

//
//
//

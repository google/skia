/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "../../../hs_spirv_target.h"

//
//
//

#include "hs_glsl.h"

//
//
//

#ifndef HS_TARGET_NAME
#define HS_TARGET_NAME      hs_target
#endif

#define HS_TARGET_HELPER(a) a

//
//
//

static struct hs_spirv_target const HS_TARGET_NAME =
{
  .config = {
    .slab = {
      .threads_log2 = HS_SLAB_THREADS_LOG2,
      .width_log2   = HS_SLAB_WIDTH_LOG2,
      .height       = HS_SLAB_HEIGHT
    },

    .words = {
      .key          = HS_KEY_WORDS,
      .val          = HS_VAL_WORDS
    },

    .block = {
      .slabs        = HS_BS_SLABS
    },

    .merge = {
      .fm = {
        .scale_min  = HS_FM_SCALE_MIN,
        .scale_max  = HS_FM_SCALE_MAX
      },
      .hm = {
        .scale_min  = HS_HM_SCALE_MIN,
        .scale_max  = HS_HM_SCALE_MAX,
      }
    }
  },

  .modules.bytes = {
#include "hs_kernels.h"
  }
};

//
//
//

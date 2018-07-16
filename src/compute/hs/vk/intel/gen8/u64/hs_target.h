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
    },

    .pad = { 0 }
  },

  .modules.bytes = {

#include "hs_kernels.h"

#ifdef HS_DUMP
    0,0,0,0
#endif
  }
};

//
//
//

#ifdef HS_DUMP

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char const * argv[])
{
  FILE * fp = fopen("hs_target.bin","wb");

  fwrite(&HS_TARGET_NAME.config,1,sizeof(HS_TARGET_NAME.config),fp);

  uint8_t const * modules = HS_TARGET_NAME.modules.bytes;
  size_t          modsize = (modules[0]<<24) | (modules[1]<<16) | (modules[2]<<8) | modules[3];

  while (modsize > 0) {
    // fprintf(stderr,"%zu\n",modsize);
    modsize += sizeof(uint32_t);
    fwrite(modules,1,modsize,fp);
    modules += modsize;
    modsize  = (modules[0]<<24) | (modules[1]<<16) | (modules[2]<<8) | modules[3];
  }

  fclose(fp);

  return EXIT_SUCCESS;
}

#endif

//
//
//

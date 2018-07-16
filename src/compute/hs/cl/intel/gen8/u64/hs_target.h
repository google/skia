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

#include "../../../hs_cl_target.h"

//
//
//

#include "hs_cl.h"

//
//
//

#ifndef HS_TARGET_NAME
#define HS_TARGET_NAME       hs_target
#endif

#define HS_TARGET_HELPER(a)  a

//
//
//

static struct hs_cl_target const HS_TARGET_NAME =
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

  .program = {
#ifndef HS_DUMP_SOURCE
    0, // KERNELS ARE BINARIES
#include "hs_cl.bin.len.xxd"
    ,
#include "hs_cl.bin.xxd"
#else
    1, // KERNELS ARE SOURCE
#include "hs_cl.src.len.xxd"
    ,
#include "hs_cl.src.xxd"
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

  size_t progsize =
    (HS_TARGET_NAME.program[1]<<24) | (HS_TARGET_NAME.program[2]<<16) |
    (HS_TARGET_NAME.program[3]<< 8) |  HS_TARGET_NAME.program[4];

  // fprintf(stderr,"%zu\n",progsize);

  progsize += 1 + sizeof(uint32_t);

  fwrite(HS_TARGET_NAME.program,1,progsize,fp);

  fclose(fp);

  return EXIT_SUCCESS;
}

#endif

//
//
//

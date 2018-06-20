/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef PATH_BUILDER_CL_12_ONCE
#define PATH_BUILDER_CL_12_ONCE

//
//
//

#include "block.h"

//
// A tag type that fits into the block id tag bitfield
//

typedef enum skc_cmd_paths_copy_tag {

  SKC_CMD_PATHS_COPY_TAG_SEGS,
  SKC_CMD_PATHS_COPY_TAG_NODE,
  SKC_CMD_PATHS_COPY_TAG_HEAD,

  SKC_CMD_PATHS_COPY_TAG_COUNT

} skc_cmd_paths_copy_tag;


SKC_STATIC_ASSERT(SKC_CMD_PATHS_COPY_TAG_COUNT <= SKC_BLOCK_ID_TAG_COUNT);

//
//
//

#endif

//
//
//


/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "suballocator.h"

//
//
//

typedef enum skc_mem_flags_e {

  SKC_MEM_FLAGS_READ_WRITE,
  SKC_MEM_FLAGS_WRITE_ONLY,
  SKC_MEM_FLAGS_READ_ONLY

} skc_mem_flags_e;

//
//
//

struct skc_allocator_host
{
#if 0
  struct {
    // in case we want to instrument perm allocs
  } perm;
#endif

  struct {
    struct skc_suballocator suballocator;
    skc_uchar             * extent;
  } temp;
};

//
//
//

void
skc_allocator_host_create(struct skc_runtime * const runtime);

void
skc_allocator_host_dispose(struct skc_runtime * const runtime);

//
//
//

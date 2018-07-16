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

#include <CL/opencl.h>

//
//
//

#include "suballocator.h"

//
//
//

struct skc_allocator_device
{
#if 0
  struct {

  } perm;
#endif

  struct {
    struct skc_suballocator suballocator;
    cl_mem                  extent;
  } temp;
};

//
//
//

void
skc_allocator_device_create(struct skc_runtime * const runtime);

void
skc_allocator_device_dispose(struct skc_runtime * const runtime);

//
//
//

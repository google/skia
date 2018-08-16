/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include <driver_types.h>
#include <stdbool.h>

//
//
//

cudaError_t
assert_cuda(cudaError_t  const code,
            char const * const file,
            int          const line,
            bool         const abort);

//
//
//

#define cuda(...) assert_cuda((cuda##__VA_ARGS__), __FILE__, __LINE__, true);

//
//
//

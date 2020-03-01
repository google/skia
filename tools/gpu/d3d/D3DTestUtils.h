/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef D3DTestUtils_DEFINED
#define D3DTestUtils_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_DIRECT3D

#include "include/gpu/d3d/GrD3DBackendContext.h"
#include "include/gpu/d3d/GrD3DTypes.h"

struct GrD3DBackendContext;

namespace sk_gpu_test {
    bool CreateD3DBackendContext(GrD3DBackendContext* ctx,
                                 bool isProtected = false);
}

#endif
#endif


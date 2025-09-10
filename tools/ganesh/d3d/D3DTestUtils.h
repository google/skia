/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef D3DTestUtils_DEFINED
#define D3DTestUtils_DEFINED

#ifdef SK_DIRECT3D

struct GrD3DBackendContext;

namespace sk_gpu_test {
    bool CreateD3DBackendContext(GrD3DBackendContext* ctx,
                                 bool isProtected = false);
}

#endif
#endif

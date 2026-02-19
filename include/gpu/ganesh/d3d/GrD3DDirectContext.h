/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DDirectContext_DEFINED
#define GrD3DDirectContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class GrDirectContext;
struct GrContextOptions;
struct GrD3DBackendContext;

namespace GrDirectContexts {
/**
 * Makes a GrDirectContext which uses Direct3D as the backend. The Direct3D context
 * must be kept alive until the returned GrDirectContext is first destroyed or abandoned.
 */
SK_API sk_sp<GrDirectContext> MakeD3D(const GrD3DBackendContext& backendContext,
                                      const GrContextOptions& options);
SK_API sk_sp<GrDirectContext> MakeD3D(const GrD3DBackendContext& backendContext);
}  // namespace GrDirectContexts

#endif

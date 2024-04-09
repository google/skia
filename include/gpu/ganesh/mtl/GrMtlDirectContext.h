/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlDirectContext_DEFINED
#define GrMtlDirectContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class GrDirectContext;
struct GrContextOptions;
struct GrMtlBackendContext;

namespace GrDirectContexts {
/**
 * Makes a GrDirectContext which uses Metal as the backend. The GrMtlBackendContext contains a
 * MTLDevice and MTLCommandQueue which should be used by the backend. These objects must
 * have their own ref which will be released when the GrMtlBackendContext is destroyed.
 * Ganesh will take its own ref on the objects which will be released when the GrDirectContext
 * is destroyed.
 */
SK_API sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext&, const GrContextOptions&);
SK_API sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext&);
}  // namespace GrDirectContexts

#endif

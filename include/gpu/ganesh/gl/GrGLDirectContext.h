/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLDirectContext_DEFINED
#define GrGLDirectContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <memory>

class GrDirectContext;
class SkContext;
struct GrContextOptions;
struct GrGLInterface;
struct SkContextOptions;

namespace GrDirectContexts {
/**
 * Creates a GrDirectContext for a backend context. GrGLInterface must be non-null.
 */
SK_API sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
SK_API sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>);
#if !defined(SK_DISABLE_LEGACY_GL_MAKE_NATIVE_INTERFACE)
SK_API sk_sp<GrDirectContext> MakeGL(const GrContextOptions&);
SK_API sk_sp<GrDirectContext> MakeGL();
#endif
}

namespace SkContexts {

// Creates a context wrapping a Ganesh GPU backend with OpenGL
std::unique_ptr<SkContext> MakeGanesh(sk_sp<const GrGLInterface>, const SkContextOptions& options);

}  // namespace SkContexts

#endif

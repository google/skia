// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/GrContextHolder.h"

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/gl/GrGLInterface.h"

#ifdef SK_GL
GrContextHolder SkMakeGLContext() {
    return GrContextHolder(GrContext::MakeGL(nullptr, GrContextOptions()).release());
}
#endif

void GrContextRelease::operator()(GrContext* ptr) { SkSafeUnref(ptr); }

#else

void GrContextRelease::operator()(GrContext*) { SkASSERT(false); }

#endif

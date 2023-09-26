// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/GrContextHolder.h"

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

#ifdef SK_GL
GrContextHolder SkMakeGLContext() {
    return GrContextHolder(GrDirectContexts::MakeGL(nullptr, GrContextOptions()).release());
}
#endif

void GrContextRelease::operator()(GrDirectContext* ptr) { SkSafeUnref(ptr); }

#else

void GrContextRelease::operator()(GrDirectContext*) { SkDEBUGFAIL(""); }

#endif

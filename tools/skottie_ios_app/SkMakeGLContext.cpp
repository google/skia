// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkMakeGLContext.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/gl/GrGLInterface.h"

void GrGLInterfaceRelease::operator()(const GrGLInterface* ptr) { SkSafeUnref(ptr); }

GrGLInterfaceHolder SkMakeGLInterface() {
    return GrGLInterfaceHolder(GrGLMakeNativeInterface().release());
}

GrContextHolder SkMakeGLContext(const GrGLInterface* iface) {
    GrContextOptions grContextOptions;
    return GrContextHolder(GrContext::MakeGL(sk_ref_sp(iface), grContextOptions).release());
}

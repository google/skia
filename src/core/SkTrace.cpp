/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTrace.h"

#ifdef SK_BUILD_FOR_ANDROID

decltype(SkTrace::fATraceBeginSection) SkTrace::fATraceBeginSection = nullptr;
decltype(SkTrace::fATraceEndSection) SkTrace::fATraceEndSection     = nullptr;
decltype(SkTrace::fATraceIsEnabled) SkTrace::fATraceIsEnabled       = []{ return false; };

void SkTrace::Init() {
    if (void* lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL)) {
        fATraceBeginSection = (decltype(fATraceBeginSection))dlsym(lib, "ATrace_beginSection");
        fATraceEndSection = (decltype(fATraceEndSection))dlsym(lib, "ATrace_endSection");
        fATraceIsEnabled = (decltype(fATraceIsEnabled))dlsym(lib, "ATrace_isEnabled");
    }
}

#endif


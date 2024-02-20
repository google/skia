/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLMakeEGLInterface_DEFINED
#define GrGLMakeEGLInterface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

struct GrGLInterface;

namespace GrGLInterfaces {
SK_API sk_sp<const GrGLInterface> MakeEGL();
}

#if !defined(SK_DISABLE_LEGACY_EGLINTERFACE_FACTORY)
sk_sp<const GrGLInterface> GrGLMakeEGLInterface();
#endif

#endif  // GrGLMakeEGLInterface_DEFINED

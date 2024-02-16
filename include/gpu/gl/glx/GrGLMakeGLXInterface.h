/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLMakeGLXInterface_DEFINED
#define GrGLMakeGLXInterface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

struct GrGLInterface;

namespace GrGLInterfaces {
SK_API sk_sp<const GrGLInterface> MakeGLX();
}

#if !defined(SK_DISABLE_LEGACY_GLXINTERFACE_FACTORY)
sk_sp<const GrGLInterface> GrGLMakeGLXInterface();
#endif

#endif  // GrGLMakeGLXInterface_DEFINED

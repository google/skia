/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrTypes.h"

#include "include/core/SkRefCnt.h"

struct GrGLInterface;

namespace GrGLInterfaces {
sk_sp<const GrGLInterface> MakeGLX();
}

#if !defined(SK_DISABLE_LEGACY_GLXINTERFACE_FACTORY)
sk_sp<const GrGLInterface> GrGLMakeGLXInterface();
#endif

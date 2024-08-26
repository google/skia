/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#if !defined(SK_DISABLE_LEGACY_GL_MAKE_NATIVE_INTERFACE)
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() { return GrGLInterfaces::MakeGLX(); }
#endif

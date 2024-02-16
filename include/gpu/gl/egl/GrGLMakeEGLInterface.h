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

sk_sp<const GrGLInterface> GrGLMakeEGLInterface();

#endif  // GrGLMakeEGLInterface_DEFINED

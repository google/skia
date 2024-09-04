/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLMakeWinInterface_DEFINED
#define GrGLMakeWinInterface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

struct GrGLInterface;

namespace GrGLInterfaces {
SK_API sk_sp<const GrGLInterface> MakeWin();
}

#endif  // GrGLMakeWinInterface_DEFINED

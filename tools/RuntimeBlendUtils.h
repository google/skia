/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RuntimeBlendUtils_DEFINED
#define RuntimeBlendUtils_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class SkBlender;

/** Returns a Runtime Effect-based blender which is equivalent to the passed-in SkBlendMode. */
sk_sp<SkBlender> GetRuntimeBlendForBlendMode(SkBlendMode mode);

#endif

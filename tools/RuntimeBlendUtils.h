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

/**
 * Returns a Runtime Effect-based blender which is equivalent to the passed-in SkBlendMode.
 * This should generate the same output as the equivalent SkBlendMode operation, but always uses
 * SkSL to perform the blend operation instead of relying on specialized/fixed-function code.
 * This is useful for verifying that Runtime Blends are working as expected throughout the pipeline.
 */
sk_sp<SkBlender> GetRuntimeBlendForBlendMode(SkBlendMode mode);

#endif

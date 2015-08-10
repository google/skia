/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_opts_arm_neon_DEFINED
#define SkBlitMask_opts_arm_neon_DEFINED

#include "SkColor.h"
#include "SkBlitMask.h"

extern void SkBlitLCD16OpaqueRow_neon(SkPMColor dst[], const uint16_t src[],
                                      SkColor color, int width,
                                      SkPMColor opaqueDst);

extern void SkBlitLCD16Row_neon(SkPMColor dst[], const uint16_t src[],
                                SkColor color, int width, SkPMColor);

#endif // #ifndef SkBlitMask_opts_arm_neon_DEFINED

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBlitRow_opts_arm_neon_DEFINED
#define SkBlitRow_opts_arm_neon_DEFINED

#include "SkBlitRow.h"

extern const SkBlitRow::Proc sk_blitrow_platform_565_procs_arm_neon[];
extern const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm_neon[];

extern void Color32_arm_neon(SkPMColor* dst, const SkPMColor* src, int count,
                             SkPMColor color);

#endif

/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkUtilsArm.h"

#include "SkBlitRow_opts_arm_neon.h"

extern const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm[] = {
    nullptr, nullptr, nullptr, nullptr,
};

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return SK_ARM_NEON_WRAP(sk_blitrow_platform_32_procs_arm)[flags];
}

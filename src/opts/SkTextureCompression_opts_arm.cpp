/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompression_opts.h"
#include "SkTextureCompression_opts_neon.h"
#include "SkUtilsArm.h"

SkTextureCompressor::CompressionProc
SkTextureCompressorGetPlatformProc(SkColorType colorType, SkTextureCompressor::Format fmt) {
#if SK_ARM_NEON_IS_NONE
    return NULL;
#else
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return NULL;
    }
#endif
    switch (colorType) {
        case kAlpha_8_SkColorType:
        {
            switch (fmt) {
                case SkTextureCompressor::kR11_EAC_Format:
                    return CompressA8toR11EAC_NEON;
                default:
                    return NULL;
            }
        }
        break;

        default:
            return NULL;
    }
#endif
}

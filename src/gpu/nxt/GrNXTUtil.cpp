/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTUtil.h"

GrPixelConfig GrNXTFormatToPixelConfig(nxtTextureFormat format) {
    switch (format) {
        case NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM:
            return kRGBA_8888_GrPixelConfig;
        case NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UINT:
        case NXT_TEXTURE_FORMAT_D32_FLOAT_S8_UINT:
        case NXT_TEXTURE_FORMAT_FORCE32:
            return kRGBA_8888_GrPixelConfig;
        default:
            SkASSERT(false);
            return kRGBA_8888_GrPixelConfig;
    }
}

nxtTextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
        default:
            SkASSERT(false);
            return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
    }
}

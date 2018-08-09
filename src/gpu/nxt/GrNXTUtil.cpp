/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTUtil.h"

GrPixelConfig GrNXTFormatToPixelConfig(dawn::TextureFormat format) {
    switch (format) {
        case dawn::TextureFormat::R8G8B8A8Unorm:
            return kRGBA_8888_GrPixelConfig;
        case dawn::TextureFormat::B8G8R8A8Unorm:
            return kBGRA_8888_GrPixelConfig;
        case dawn::TextureFormat::R8Unorm:
            return kAlpha_8_GrPixelConfig;
        case dawn::TextureFormat::D32FloatS8Uint:
        default:
            SkASSERT(false);
            return kRGBA_8888_GrPixelConfig;
    }
}

dawn::TextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            return dawn::TextureFormat::R8G8B8A8Unorm;
        case kBGRA_8888_GrPixelConfig:
            return dawn::TextureFormat::B8G8R8A8Unorm;
        case kAlpha_8_GrPixelConfig:
            return dawn::TextureFormat::R8Unorm;
        default:
            SkASSERT(false);
            return dawn::TextureFormat::R8G8B8A8Unorm;
    }
}

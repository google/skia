/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTUtil.h"

GrPixelConfig GrNXTFormatToPixelConfig(nxt::TextureFormat format) {
    switch (format) {
        case nxt::TextureFormat::R8G8B8A8Unorm:
            return kRGBA_8888_GrPixelConfig;
        case nxt::TextureFormat::B8G8R8A8Unorm:
            return kBGRA_8888_GrPixelConfig;
        case nxt::TextureFormat::R8Unorm:
            return kAlpha_8_GrPixelConfig;
        case nxt::TextureFormat::D32FloatS8Uint:
        default:
            SkASSERT(false);
            return kRGBA_8888_GrPixelConfig;
    }
}

nxt::TextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            return nxt::TextureFormat::R8G8B8A8Unorm;
        case kBGRA_8888_GrPixelConfig:
            return nxt::TextureFormat::B8G8R8A8Unorm;
        case kAlpha_8_GrPixelConfig:
            return nxt::TextureFormat::R8Unorm;
        default:
            SkASSERT(false);
            return nxt::TextureFormat::R8G8B8A8Unorm;
    }
}

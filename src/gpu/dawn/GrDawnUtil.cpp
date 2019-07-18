/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnUtil.h"

GrPixelConfig GrDawnFormatToPixelConfig(dawn::TextureFormat format) {
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

bool GrPixelConfigToDawnFormat(GrPixelConfig config, dawn::TextureFormat* format) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            *format = dawn::TextureFormat::R8G8B8A8Unorm;
            return true;
        case kBGRA_8888_GrPixelConfig:
            *format = dawn::TextureFormat::B8G8R8A8Unorm;
            return true;
        case kAlpha_8_GrPixelConfig:
            *format = dawn::TextureFormat::R8Unorm;
            return true;
        default:
            return false;
    }
}

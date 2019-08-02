/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnUtil.h"

GrPixelConfig GrDawnFormatToPixelConfig(dawn::TextureFormat format) {
    switch (format) {
        case dawn::TextureFormat::RGBA8Unorm:
            return kRGBA_8888_GrPixelConfig;
        case dawn::TextureFormat::BGRA8Unorm:
            return kBGRA_8888_GrPixelConfig;
        case dawn::TextureFormat::R8Unorm:
            return kAlpha_8_GrPixelConfig;
        case dawn::TextureFormat::Depth24PlusStencil8:
        default:
            SkASSERT(false);
            return kRGBA_8888_GrPixelConfig;
    }
}

bool GrDawnFormatIsRenderable(dawn::TextureFormat format) {
    // For now, all the formats above are renderable. If a non-renderable format is added
    // (see dawn/src/dawn_native/Format.cpp), an exception should be added here.
    return true;
}

bool GrPixelConfigToDawnFormat(GrPixelConfig config, dawn::TextureFormat* format) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            *format = dawn::TextureFormat::RGBA8Unorm;
            return true;
        case kBGRA_8888_GrPixelConfig:
            *format = dawn::TextureFormat::BGRA8Unorm;
            return true;
        case kAlpha_8_GrPixelConfig:
            *format = dawn::TextureFormat::R8Unorm;
            return true;
        default:
            return false;
    }
}

#if GR_TEST_UTILS
const char* GrDawnFormatToStr(dawn::TextureFormat format) {
    switch (format) {
        case dawn::TextureFormat::RGBA8Unorm:
            return "RGBA8Unorm";
        case dawn::TextureFormat::BGRA8Unorm:
            return "BGRA8Unorm";
        case dawn::TextureFormat::R8Unorm:
            return "R8Unorm";
        case dawn::TextureFormat::Depth24PlusStencil8:
            return "Depth24PlusStencil8";
        default:
            SkASSERT(false);
            return "Unknown";
    }
}
#endif

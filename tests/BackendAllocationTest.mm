/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

#import <metal/metal.h>

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    struct {
        SkColorType      fColorType;
        GrMTLPixelFormat fFormat;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm },
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm_sRGB },

        { kRGB_888x_SkColorType,     MTLPixelFormatRGBA8Unorm },

        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm },
        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm_sRGB },

        { kRGBA_1010102_SkColorType, MTLPixelFormatRGB10A2Unorm },
#ifdef SK_BUILD_FOR_IOS
        { kRGB_565_SkColorType,      MTLPixelFormatB5G6R5Unorm },
        { kARGB_4444_SkColorType,    MTLPixelFormatABGR4Unorm },
#endif

        { kAlpha_8_SkColorType,      MTLPixelFormatA8Unorm },
        { kAlpha_8_SkColorType,      MTLPixelFormatR8Unorm },

        { kGray_8_SkColorType,       MTLPixelFormatR8Unorm },

        { kRGBA_F32_SkColorType,     MTLPixelFormatRGBA32Float },

        { kRGBA_F16Norm_SkColorType, MTLPixelFormatRGBA16Float },
        { kRGBA_F16_SkColorType,     MTLPixelFormatRGBA16Float },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,     MTLPixelFormatRG32Float },
        { kUnknown_SkColorType,     MTLPixelFormatRG8Unorm },
        { kUnknown_SkColorType,     MTLPixelFormatR16Float },
#ifdef SK_BUILD_FOR_IOS
        { kUnknown_SkColorType,     MTLPixelFormatETC2_RGB8 }
#endif
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeMtl(combo.fFormat);

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            if (GrRenderable::kYes == renderable) {
                if (kRGB_888x_SkColorType == combo.fColorType) {
                    // Ganesh can't perform the blends correctly when rendering this format
                    continue;
                }
            }

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return context->createBackendTexture(32, 32, format,
                                                     GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}


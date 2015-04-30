/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This is a straightforward test of floating point textures, which are
 * supported on some platforms.  As of right now, this test only supports
 * 32 bit floating point textures, and indeed floating point test values
 * have been selected to require 32 bits of precision and full IEEE conformance
 */

#include <float.h>
#include "Test.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrContextFactory.h"

#include "SkGpuDevice.h"
#include "SkHalf.h"

static const int DEV_W = 100, DEV_H = 100;
static const int FP_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4/*RGBA*/;
static const float kMaxIntegerRepresentableInSPFloatingPoint = 16777216;  // 2 ^ 24

static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);

DEF_GPUTEST(FloatingPointTextureTest, reporter, factory) {
    SkTDArray<float> controlPixelData, readBuffer;
    controlPixelData.setCount(FP_CONTROL_ARRAY_SIZE);
    readBuffer.setCount(FP_CONTROL_ARRAY_SIZE);

    for (int i = 0; i < FP_CONTROL_ARRAY_SIZE; i += 4) {
        controlPixelData[i + 0] = FLT_MIN;
        controlPixelData[i + 1] = FLT_MAX;
        controlPixelData[i + 2] = FLT_EPSILON;
        controlPixelData[i + 3] = kMaxIntegerRepresentableInSPFloatingPoint;
    }

    for (int origin = 0; origin < 2; ++origin) {
        for (int glCtxType = 0; glCtxType < GrContextFactory::kGLContextTypeCnt; ++glCtxType) {
            GrSurfaceDesc desc;
            desc.fFlags  = kRenderTarget_GrSurfaceFlag;
            desc.fWidth  = DEV_W;
            desc.fHeight = DEV_H;
            desc.fConfig = kRGBA_float_GrPixelConfig;
            desc.fOrigin = 0 == origin ?
                kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;

            GrContextFactory::GLContextType type =
                static_cast<GrContextFactory::GLContextType>(glCtxType);
            if (!GrContextFactory::IsRenderingGLContext(type)) {
                continue;
            }
            GrContext* context = factory->get(type);
            if (NULL == context){
                continue;
            }

            SkAutoTUnref<GrTexture> fpTexture(context->textureProvider()->createTexture(
                desc, false, controlPixelData.begin(), 0));
            // Floating point textures are NOT supported everywhere
            if (NULL == fpTexture) {
                continue;
            }
            fpTexture->readPixels(0, 0, DEV_W, DEV_H, desc.fConfig, readBuffer.begin(), 0);
            REPORTER_ASSERT(reporter,
                    0 == memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.bytes()));
        }
    }
}

static const int HALF_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 1 /*alpha-only*/;

DEF_GPUTEST(HalfFloatTextureTest, reporter, factory) {
    SkTDArray<SkHalf> controlPixelData, readBuffer;
    controlPixelData.setCount(HALF_CONTROL_ARRAY_SIZE);
    readBuffer.setCount(HALF_CONTROL_ARRAY_SIZE);

    for (int i = 0; i < HALF_CONTROL_ARRAY_SIZE; i += 4) {
        controlPixelData[i + 0] = SK_HalfMin;
        controlPixelData[i + 1] = SK_HalfMax;
        controlPixelData[i + 2] = SK_HalfEpsilon;
        controlPixelData[i + 3] = 0x6800;   // 2^11
    }

    for (int origin = 0; origin < 2; ++origin) {
        for (int glCtxType = 0; glCtxType < GrContextFactory::kGLContextTypeCnt; ++glCtxType) {
            GrSurfaceDesc desc;
            desc.fFlags  = kRenderTarget_GrSurfaceFlag;
            desc.fWidth  = DEV_W;
            desc.fHeight = DEV_H;
            desc.fConfig = kAlpha_half_GrPixelConfig;
            desc.fOrigin = 0 == origin ?
                kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;

            GrContextFactory::GLContextType type =
                static_cast<GrContextFactory::GLContextType>(glCtxType);
            if (!GrContextFactory::IsRenderingGLContext(type)) {
                continue;
            }
            GrContext* context = factory->get(type);
            if (NULL == context){
                continue;
            }

            SkAutoTUnref<GrTexture> fpTexture(context->textureProvider()->createTexture(
                desc, false, controlPixelData.begin(), 0));
            // 16-bit floating point textures are NOT supported everywhere
            if (NULL == fpTexture) {
                continue;
            }
            fpTexture->readPixels(0, 0, DEV_W, DEV_H, desc.fConfig, readBuffer.begin(), 0);
            REPORTER_ASSERT(reporter,
                    0 == memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.bytes()));
        }
    }
}

#endif


/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkGpuDevice.h"

static const int X_SIZE = 12;
static const int Y_SIZE = 12;

void ReadWriteAlphaTest(skiatest::Reporter* reporter, GrContext* context) {

#if SK_SCALAR_IS_FIXED
    // GPU device known not to work in the fixed pt build.
    return;
#endif

    unsigned char textureData[X_SIZE][Y_SIZE];

    memset(textureData, 0, X_SIZE * Y_SIZE);

    GrTextureDesc desc;

    // let Skia know we will be using this texture as a render target
    desc.fFlags     = kRenderTarget_GrTextureFlagBit;
    // it is a single channel texture
    desc.fConfig    = kAlpha_8_GrPixelConfig;
    desc.fWidth     = X_SIZE;
    desc.fHeight    = Y_SIZE;
    desc.fSampleCnt = 0;

    // We are initializing the texture with zeros here
    GrTexture* texture = context->createUncachedTexture(desc, textureData, 0);
    if (!texture) {
        return;
    }

    GrAutoUnref au(texture);

    // create a distinctive texture
    for (int y = 0; y < Y_SIZE; ++y) {
        for (int x = 0; x < X_SIZE; ++x) {
            textureData[x][y] = x*Y_SIZE+y;
        }
    }

    // upload the texture
    texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                         textureData, 0);

    unsigned char readback[X_SIZE][Y_SIZE];

    // clear readback to something non-zero so we can detect readback failures
    memset(readback, 0x1, X_SIZE * Y_SIZE);

    // read the texture back
    texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig, 
                        readback, 0);

    // make sure the original & read back versions match
    bool match = true;

    for (int y = 0; y < Y_SIZE; ++y) {
        for (int x = 0; x < X_SIZE; ++x) {
            if (textureData[x][y] != readback[x][y]) {
                match = false;
            }
        }
    }

    REPORTER_ASSERT(reporter, match);

    // Now try writing on the single channel texture
    SkCanvas canvas;

    canvas.setDevice(new SkGpuDevice(context, texture->asRenderTarget()))->unref();

    SkPaint paint;

    const SkRect rect = SkRect::MakeLTRB(-10, -10, X_SIZE + 10, Y_SIZE + 10);

    paint.setColor(SK_ColorWHITE);

    canvas.drawRect(rect, paint);

    texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig, 
                        readback, 0);

    match = true;
    
    for (int y = 0; y < Y_SIZE; ++y) {
        for (int x = 0; x < X_SIZE; ++x) {
            if (0xFF != readback[x][y]) {
                match = false;
            }
        }
    }

    REPORTER_ASSERT(reporter, match);
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("ReadWriteAlpha", ReadWriteAlphaTestClass, ReadWriteAlphaTest)


/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSmallAllocator.h"
#include "SkSpriteBlitter.h"

SkSpriteBlitter::SkSpriteBlitter(const SkBitmap& source)
        : fSource(&source) {
    fSource->lockPixels();
}

SkSpriteBlitter::~SkSpriteBlitter() {
    fSource->unlockPixels();
}

void SkSpriteBlitter::setup(const SkBitmap& device, int left, int top,
                            const SkPaint& paint) {
    fDevice = &device;
    fLeft = left;
    fTop = top;
    fPaint = &paint;
}

#ifdef SK_DEBUG
void SkSpriteBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                const int16_t runs[]) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitMask(const SkMask&, const SkIRect& clip) {
    SkDEBUGFAIL("how did we get here?");
}
#endif

///////////////////////////////////////////////////////////////////////////////

// returning null means the caller will call SkBlitter::Choose() and
// have wrapped the source bitmap inside a shader
SkBlitter* SkBlitter::ChooseSprite(const SkBitmap& device, const SkPaint& paint,
        const SkBitmap& source, int left, int top, SkTBlitterAllocator* allocator) {
    /*  We currently ignore antialiasing and filtertype, meaning we will take our
        special blitters regardless of these settings. Ignoring filtertype seems fine
        since by definition there is no scale in the matrix. Ignoring antialiasing is
        a bit of a hack, since we "could" pass in the fractional left/top for the bitmap,
        and respect that by blending the edges of the bitmap against the device. To support
        this we could either add more special blitters here, or detect antialiasing in the
        paint and return null if it is set, forcing the client to take the slow shader case
        (which does respect soft edges).
    */
    SkASSERT(allocator != NULL);

    SkSpriteBlitter* blitter;

    switch (device.colorType()) {
        case kRGB_565_SkColorType:
            blitter = SkSpriteBlitter::ChooseD16(source, paint, allocator);
            break;
        case kPMColor_SkColorType:
            blitter = SkSpriteBlitter::ChooseD32(source, paint, allocator);
            break;
        default:
            blitter = NULL;
            break;
    }

    if (blitter) {
        blitter->setup(device, left, top, paint);
    }
    return blitter;
}

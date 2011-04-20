/* libs/graphics/sgl/SkBlitter_Sprite.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

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
    SkASSERT(!"how did we get here?");
}

void SkSpriteBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                const int16_t runs[]) {
    SkASSERT(!"how did we get here?");
}

void SkSpriteBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(!"how did we get here?");
}

void SkSpriteBlitter::blitMask(const SkMask&, const SkIRect& clip) {
    SkASSERT(!"how did we get here?");
}
#endif

///////////////////////////////////////////////////////////////////////////////

// returning null means the caller will call SkBlitter::Choose() and
// have wrapped the source bitmap inside a shader
SkBlitter* SkBlitter::ChooseSprite( const SkBitmap& device,
                                    const SkPaint& paint,
                                    const SkBitmap& source,
                                    int left, int top,
                                    void* storage, size_t storageSize) {
    /*  We currently ignore antialiasing and filtertype, meaning we will take our
        special blitters regardless of these settings. Ignoring filtertype seems fine
        since by definition there is no scale in the matrix. Ignoring antialiasing is
        a bit of a hack, since we "could" pass in the fractional left/top for the bitmap,
        and respect that by blending the edges of the bitmap against the device. To support
        this we could either add more special blitters here, or detect antialiasing in the
        paint and return null if it is set, forcing the client to take the slow shader case
        (which does respect soft edges).
    */

    SkSpriteBlitter* blitter;

    switch (device.getConfig()) {
        case SkBitmap::kRGB_565_Config:
            blitter = SkSpriteBlitter::ChooseD16(source, paint, storage,
                                                 storageSize);
            break;
        case SkBitmap::kARGB_8888_Config:
            blitter = SkSpriteBlitter::ChooseD32(source, paint, storage,
                                                 storageSize);
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


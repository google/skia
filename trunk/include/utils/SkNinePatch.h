
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkNinePatch_DEFINED
#define SkNinePatch_DEFINED

#include "SkRect.h"
#include "SkRegion.h"

class SkBitmap;
class SkCanvas;
class SkPaint;

class SkNinePatch {
public:
    static void DrawNine(SkCanvas* canvas, const SkRect& dst,
                     const SkBitmap& bitmap, const SkIRect& margins,
                     const SkPaint* paint = NULL);

    static void DrawMesh(SkCanvas* canvas, const SkRect& dst,
                         const SkBitmap& bitmap,
                         const int32_t xDivs[], int numXDivs,
                         const int32_t yDivs[], int numYDivs,
                         const SkPaint* paint = NULL);
};

#endif
